#ifndef MQTT_PICO_H
#define MQTT_PICO_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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

void mqtt_tls(mqtt_config_t *mqtt);

bool mqtt_start_client(
    mqtt_config_t *mqtt,
    char *server,
    mqtt_connection_cb_t conn_cb,
    mqtt_incoming_publish_cb_t pub_cb,
    mqtt_incoming_data_cb_t data_cb,
    dns_found_callback dns_cb
);

char *mqtt_full_topic(mqtt_config_t *mqtt, char *name);

char *mqtt_generate_client_id(char *device_name);

void mqtt_manage_topics(mqtt_config_t *mqtt, char **topics, size_t num_topics, mqtt_action_t action, mqtt_request_cb_t cb);

#endif // MQTT_PICO_H