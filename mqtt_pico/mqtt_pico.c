#include "mqtt_pico.h"

/**
 * @brief Inicializa as configurações TLS de um cliente, se disponíveis
 *
 * @param[out] mqtt        Ponteiro para a estrutura de configuração MQTT
 *
 * @see mqtt_start_client, mqtt_manage_topics
 */
void mqtt_tls(mqtt_config_t * mqtt) {
    #if LWIP_ALTCP && LWIP_ALTCP_TLS // TLS enable
        #ifdef MQTT_CERT_INC
            const uint8_t ca_cert[] = TLS_ROOT_CERT;
            const uint8_t client_key[] = TLS_CLIENT_KEY;
            const uint8_t client_cert[] = TLS_CLIENT_CERT;
            mqtt->client_info.tls_config = altcp_tls_create_config_client_2wayauth(
                                            ca_cert,
                                            sizeof(ca_cert),
                                            client_key,
                                            sizeof(client_key),
                                            NULL,
                                            0,
                                            client_cert,
                                            sizeof(client_cert));
            #if ALTCP_MBEDTLS_AUTHMODE != MBEDTLS_SSL_VERIFY_REQUIRED 
            //    tls without verification is insecure
            #endif
        #else
            mqtt->client_info.tls_config = altcp_tls_create_config_client(NULL, 0);
        #endif
    #endif
}

/**
 * @brief Inicializa o cliente MQTT, conecta ao broker e define callbacks para eventos.
 *
 * Esta função configura o cliente MQTT com os parâmetros fornecidos, tenta
 * estabelecer a conexão com o broker e registra funções de callback para lidar
 * com eventos de conexão e mensagens recebidas.
 *
 * @param[in] mqtt            Ponteiro para a estrutura de configuração do cliente MQTT. Não deve ser NULL.
 * @param[in] server          Endereço ou hostname do broker MQTT.
 * @param[in] conn_cb         Callback chamado quando a conexão com o broker é estabelecida ou perdida.
 * @param[in] pub_cb Callback chamado quando uma nova publicação chega em um tópico inscrito.
 * @param[in] data_cb Callback chamado quando os dados de uma publicação estão disponíveis.
 * @param[in] dns_cb Callback chamado no pedido de DNS ao servidor. Esse callback só é chamado em casos onde o status
 *      retornado `dns_gethostbyname` é `ERR_INPROGRESS`.
 *
 * @return bool em caso de sucesso na configuração e conexão do cliente MQTT. Retorna false caso contrário.
 *
 * @note Esta função deve ser chamada antes de tentar publicar ou assinar tópicos. É necessário que a arquitetura
 *      CYW43 esteja em operação, e a mesma esteja conectada a uma rede WiFi, em modo Station.
 * @warning Certifique-se de que os callbacks fornecidos permanecem válidos
 *          durante todo o ciclo de vida do cliente MQTT.
 *
 * @see mqtt_sub_unsub, mqtt_manage_topics
 */
bool mqtt_start_client(
    mqtt_config_t *mqtt,
    char *server,
    mqtt_connection_cb_t conn_cb,
    mqtt_incoming_publish_cb_t pub_cb,
    mqtt_incoming_data_cb_t data_cb,
    dns_found_callback dns_cb
) {
    // Faz um pedido de DNS para o endereço IP do servidor MQTT
    cyw43_arch_lwip_begin();
    int err1 = dns_gethostbyname(server, &mqtt->server_ip, dns_cb, mqtt);
    cyw43_arch_lwip_end();

    if (err1 != ERR_OK && err1 != ERR_INPROGRESS) {
        return false;
    }

    #if LWIP_ALTCP && LWIP_ALTCP_TLS
        uint port = MQTT_TLS_PORT;
    #else
        uint port = MQTT_PORT;
    #endif

    mqtt->client = mqtt_client_new();
    if (!mqtt->client) {
        return false;
    }

    cyw43_arch_lwip_begin();
    int err2 = mqtt_client_connect(mqtt->client, &mqtt->server_ip, port, conn_cb, mqtt, &mqtt->client_info);
    if (err2 != ERR_OK) {
        return false;
    }

    #if LWIP_ALTCP && LWIP_ALTCP_TLS
        mbedtls_ssl_set_hostname(altcp_tls_context(mqtt->client->conn), server);
    #endif
    mqtt_set_inpub_callback(mqtt->client, pub_cb, data_cb, mqtt);
    cyw43_arch_lwip_end();

    return true;
}

/**
 * @brief Gera um ID de cliente MQTT único baseado no nome do dispositivo e no ID único do hardware.
 *
 * O ID do cliente é construído no formato: `<device_name>-<unique_id>`.
 * Exemplo: `smartmeter-a1b2c`. Este ID pode ser usado para identificar
 * o dispositivo de forma única no broker MQTT.
 *
 * @param[in] device_name Nome arbitrário do dispositivo. Não deve ser NULL.
 * @return char* Ponteiro para uma string alocada dinamicamente contendo
 *              o Client ID. O chamador é responsável por liberar a memória
 *              usando `free()`.
 *
 * @note O tamanho do Client ID depende do comprimento de `device_name`
 *       e do tamanho do ID único do hardware.
 * @warning Certifique-se de chamar `free()` após usar a string para evitar vazamento de memória.
 */
char *mqtt_generate_client_id(char *device_name) {
    const int UNIQUE_ID_LEN = 5;
    const int DEVICE_NAME_LEN = strlen(device_name);

    // Gera um ID de cliente único baseado no ID único do dispositivo
    char unique_id[UNIQUE_ID_LEN];
    pico_get_unique_board_id_string(unique_id, UNIQUE_ID_LEN);
    for (int i = 0; i < UNIQUE_ID_LEN - 1; i++) {
        unique_id[i] = tolower(unique_id[i]);
    }

    int client_id_len = DEVICE_NAME_LEN + UNIQUE_ID_LEN + 2; // [device_name]-[unique_id]\0
    char *client_id = malloc(client_id_len);
    if(!client_id) {
        return NULL;
    }

    snprintf(client_id, client_id_len, "%s-%s", device_name, unique_id);

    return client_id;
}

/**
 * @brief Gera o tópico MQTT completo, acrescentando o nome do cliente se necessário.
 *
 * Esta função combina o nome do cliente MQTT com o nome do tópico base fornecido,
 * criando uma string de tópico completa que pode ser usada para publicar ou
 * assinar mensagens.
 * Exemplo: se o cliente tiver ID `smartmeter` e `name` for `/led`, o resultado
 * pode ser `/smartmeter/led`.
 *
 * @param[in] mqtt Ponteiro para a estrutura de configuração do cliente MQTT. Não deve ser NULL.
 * @param[in] name Nome do tópico base. Não deve ser NULL.
 *
 * @return char* Ponteiro para uma string alocada dinamicamente contendo o tópico completo.
 *               O conteúdo **não deve ser liberado** pelo chamador e
 *               **pode ser sobrescrito em chamadas subsequentes**.
 *
 * @note O formato exato do tópico depende de como o cliente MQTT foi configurado.
 */
char *mqtt_full_topic(mqtt_config_t *mqtt, char *name) {
    if (mqtt->unique_topic) {
        static char full_topic[MQTT_TOPIC_LEN];
        snprintf(full_topic, MQTT_TOPIC_LEN, "%s/%s", mqtt->client_info.client_id, name);
        return full_topic;
    } else {
        return name;
    }
}

/**
 * @brief Gerencia inscrições ou remoções em múltiplos tópicos MQTT.
 *
 * Esta função percorre uma lista de tópicos e realiza a ação especificada
 * (inscrição ou remoção) em cada um deles, chamando o callback fornecido
 * após cada operação, se aplicável.
 *
 * @param[in] mqtt       Ponteiro para a estrutura de configuração do cliente MQTT. Não deve ser NULL.
 * @param[in] topics     Array de strings contendo os nomes dos tópicos.
 * @param[in] num_topics Número de tópicos presentes no array `topics`.
 * @param[in] action     Ação a ser realizada em cada tópico: MQTT_SUBSCRIBE ou MQTT_UNSUBSCRIBE.
 * @param[in] cb         Callback opcional para notificação do resultado de cada subscribe/unsubscribe.
 *                       Pode ser NULL se não houver necessidade de callback.
 *
 * @note O array `topics` **não deve ser NULL** e deve conter pelo menos `num_topics` elementos válidos.
 * @note Esta função **não copia** os tópicos, apenas referencia os ponteiros fornecidos.
 *
 * @see mqtt_sub_unsub, mqtt_config_t, mqtt_action_t
 *
 * @code
 * // Exemplo de uso:
 * char *topics[] = { "/led", "/beep", "/ping", "/exit" };
 * size_t num_topics = sizeof(topics) / sizeof(topics[0]);
 *
 * // Inscreve nos tópicos com callback
 * mqtt_manage_topics(&mqtt, topics, num_topics, MQTT_SUBSCRIBE, my_callback);
 *
 * // Remove inscrições nos tópicos sem callback
 * mqtt_manage_topics(&mqtt, topics, num_topics, MQTT_UNSUBSCRIBE, NULL);
 * @endcode
 */
void mqtt_manage_topics(mqtt_config_t *mqtt, char **topics, size_t num_topics, mqtt_action_t action, mqtt_request_cb_t cb) {
    for (int i = 0; i < num_topics; i++) {
        mqtt_sub_unsub(mqtt->client, topics[i], mqtt->sub_qos, cb, mqtt, action);
    }
}