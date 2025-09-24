#ifndef MQTT_PICO_H
#define MQTT_PICO_H

#include "pico/stdlib.h"     // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "pico/unique_id.h" // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "lwip/apps/mqtt.h"      // Biblioteca LWIP MQTT -  fornece funções e recursos para conexão MQTT
#include "lwip/apps/mqtt_priv.h" // Biblioteca que fornece funções e recursos para Geração de Conexões
#include "lwip/dns.h"            // Biblioteca que fornece funções e recursos suporte DNS
#include "lwip/altcp_tls.h"      // Biblioteca que fornece funções e recursos para conexões seguras usando TLS

// Este arquivo inclui seu certificado de cliente para autenticação do servidor cliente
#ifdef MQTT_CERT_INC
#include MQTT_CERT_INC
#endif

/**
 * Tamanho máximo do tópico MQTT.
 */
#ifndef MQTT_TOPIC_LEN
#define MQTT_TOPIC_LEN MQTT_OUTPUT_RINGBUF_SIZE
#endif

/**
 * @brief Alias para informações do cliente MQTT.
 */
typedef struct mqtt_connect_client_info_t mqtt_client_info_t;

/**
 * @brief Estrutura que armazena dados de mensagens MQTT.
 */
typedef struct mqtt_data_t {
    char data[MQTT_OUTPUT_RINGBUF_SIZE]; /**< Buffer de dados para envio/recebimento MQTT */
    char topic[MQTT_TOPIC_LEN];          /**< Tópico MQTT para publicação/assinatura */
    uint32_t len;                        /**< Tamanho da string de dado recebida */
} mqtt_data_t;

/**
 * @brief Estrutura de configuração do cliente MQTT.
 *
 * Contém todas as informações necessárias para gerenciar a conexão,
 * tópicos, QoS, mensagens e controle do cliente MQTT.
 */
typedef struct mqtt_config_t {
    mqtt_client_t *client;          /**< Ponteiro para a estrutura do cliente MQTT */
    mqtt_client_info_t client_info; /**< Informações do cliente MQTT (ID, usuário, senha, etc.) */
    ip_addr_t server_ip;            /**< Endereço IP do broker MQTT */
    mqtt_data_t data;               /**< Dados recebidos ou a serem enviados nos tópicos */
    bool connect_done;              /**< Indica se a conexão com o broker foi concluída */
    int sub_count;                  /**< Contagem de tópicos inscritos */
    bool stop_client;               /**< Flag para indicar parada do cliente MQTT */
    uint8_t sub_qos;                /**< Qualidade de serviço para assinaturas (QoS) */
    uint8_t pub_qos;                /**< Qualidade de serviço para publicações (QoS) */
    bool retain;                    /**< Define se as mensagens publicadas devem ser retidas no broker */
    bool unique_topic;              /**< Se true, adiciona o nome do cliente aos tópicos, permitindo múltiplos dispositivos no mesmo broker */
} mqtt_config_t;

/**
 * @brief Ações possíveis para gerenciar tópicos MQTT.
 */
typedef enum {
    MQTT_UNSUBSCRIBE = 0, /**< Remover inscrição de um tópico */
    MQTT_SUBSCRIBE   = 1  /**< Inscrever em um tópico */
} mqtt_action_t;

/**
 * @brief Cria e inicializa a estrutura de configuração padrão do cliente MQTT.
 *
 * Esta função instancia uma estrutura `mqtt_config_t` preenchida com os
 * atributos fornecidos, definindo os parâmetros necessários para conectar
 * ao broker MQTT e configurar tópicos, QoS e mensagens Last Will.
 *
 * @param[out] mqtt        Ponteiro para a estrutura de configuração MQTT
 * @param[in] device_name  Nome arbitrário do dispositivo. Não deve ser NULL.
 * @param[in] keep_alive   Intervalo de keep-alive em segundos.
 * @param[in] username     Nome de usuário para autenticação no broker MQTT.
 * @param[in] password     Senha para autenticação no broker MQTT.
 * @param[in] will_topic   Nome do tópico Last Will que indica se o cliente está vivo.
 * @param[in] will_msg     Mensagem que será enviada no tópico Last Will.
 * @param[in] will_qos     Nível de QoS (qualidade de serviço) para a mensagem Last Will.
 * @param[in] sub_qos      Nível de QoS para assinaturas de tópicos.
 * @param[in] pub_qos      Nível de QoS para publicações de tópicos.
 * @param[in] retain       Indica se as mensagens publicadas devem ser retidas no broker.
 * @param[in] unique_topic Se true, adiciona o nome do cliente aos tópicos, permitindo múltiplos dispositivos usando o mesmo broker.
 *
 * @return bool em caso de sucesso da configuração inicial, retorna true. Retorna false caso contrário
 *
 * @note Os ponteiros para strings fornecidos (`device_name`, `username`, `password`,
 *       `will_topic`, `will_msg`) devem permanecer válidos enquanto a
 *       estrutura for usada.
 *
 * @see mqtt_start_client, mqtt_manage_topics
 */
bool mqtt_start(
    mqtt_config_t *mqtt,
    char *device_name,
    uint keep_alive,
    char *username,
    char *password,
    char *will_topic,
    char *will_msg,
    uint8_t will_qos,
    uint8_t sub_qos,
    uint8_t pub_qos,
    bool retain,
    bool unique_topic
);

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
 * @param[in] dns_cb Callback chamado no pedido de DNS ao servidor
 * 
 * @return bool em caso de sucesso na configuração e conexão do cliente MQTT. Retorna false caso contrário.
 * 
 * @note Esta função deve ser chamada antes de tentar publicar ou assinar tópicos.
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
);

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
char *mqtt_full_topic(mqtt_config_t *mqtt, char *name);

/**
 * @brief Callback chamado quando um hostname é resolvido via DNS.
 *
 * Esta função é chamada automaticamente pelo sistema de DNS quando
 * a resolução de um hostname termina. É usada para obter o endereço
 * IP de um host e prosseguir com a conexão MQTT.
 *
 * @param[in] hostname O nome do host que foi resolvido (string).
 * @param[in] ipaddr   Ponteiro para a estrutura contendo o endereço IP
 *                     resolvido. Pode ser NULL se a resolução falhou.
 * @param[in] arg      Ponteiro genérico passado pelo usuário na
 *                     chamada de registro do callback, geralmente
 *                     usado para passar contexto ou estado da conexão.
 *
 * @note Esta função não deve bloquear. Operações longas devem ser
 *       delegadas a outra thread ou agendadas de forma assíncrona.
 *
 * @see dns_gethostbyname, mqtt_connect
 */
void mqtt_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg);

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
char *mqtt_generate_client_id(char *device_name);

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
void mqtt_manage_topics(mqtt_config_t *mqtt, char **topics, size_t num_topics, mqtt_action_t action, mqtt_request_cb_t cb);

/*************************************************************************************/
/** ------------- Funções de callback genéricas que podem ser usadas --------------  */
/*************************************************************************************/

void mqtt_g_pub_cb(__unused void *arg, err_t err);

/**
 * @brief Callback genérico chamado após uma operação de subscribe.
 *
 * Esta função pode ser usada como callback padrão após tentar inscrever
 * o cliente MQTT em um tópico.
 *
 * @param[in] arg Ponteiro genérico passado pelo usuário, geralmente o contexto do cliente MQTT.
 * @param[in] err Código de erro da operação (ERR_OK se bem-sucedida).
 */
void mqtt_g_sub_cb(void *arg, err_t err);

/**
 * @brief Callback genérico chamado após uma operação de unsubscribe.
 *
 * Esta função pode ser usada como callback padrão após tentar remover
 * a inscrição de um tópico MQTT.
 *
 * @param[in] arg Ponteiro genérico passado pelo usuário, geralmente o contexto do cliente MQTT.
 * @param[in] err Código de erro da operação (ERR_OK se bem-sucedida).
 */
void mqtt_g_unsub_cb(void *arg, err_t err);

/**
 * @brief Callback genérico chamado quando dados chegam em um tópico inscrito.
 *
 * Esta função pode ser usada para processar os dados recebidos de forma genérica.
 *
 * @param[in] arg  Ponteiro genérico passado pelo usuário, geralmente o contexto do cliente MQTT.
 * @param[in] data Ponteiro para o buffer contendo os dados recebidos.
 * @param[in] len  Tamanho dos dados recebidos em bytes.
 * @param[in] flags Flags adicionais do pacote, conforme especificação do MQTT.
 */
void mqtt_g_in_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags);

/**
 * @brief Callback genérico chamado após uma operação de publish.
 *
 * Esta função pode ser usada como callback padrão para tratar a confirmação
 * de publicação de mensagens.
 *
 * @param[in] arg     Ponteiro genérico passado pelo usuário, geralmente o contexto do cliente MQTT.
 * @param[in] topic   Tópico em que a mensagem foi publicada.
 * @param[in] tot_len Tamanho total da mensagem publicada em bytes.
 */
void mqtt_g_in_pub_cb(void *arg, const char *topic, u32_t tot_len);

/**
 * @brief Callback genérico de conexão MQTT.
 *
 * Esta função é chamada automaticamente pelo cliente MQTT
 * sempre que ocorre uma tentativa de conexão com o broker.
 *
 * @param[in] client Ponteiro para a instância do cliente MQTT (`mqtt_client_t`).
 * @param[in] arg    Ponteiro para argumento do usuário (definido na configuração do cliente).
 * @param[in] status Status da conexão, do tipo `mqtt_connection_status_t`.
 *               - MQTT_CONNECT_ACCEPTED (0): Conexão estabelecida com sucesso
 *               - Outros valores indicam erro (ex.: timeout, recusado, etc.)
 *
 * @note Esta função deve ser registrada com `mqtt_client_connect()`.
 * @warning Não deve executar operações bloqueantes dentro do callback,
 *          pois ele é chamado no contexto da pilha lwIP.
 */
void mqtt_g_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status);

#endif // MQTT_PICO_H