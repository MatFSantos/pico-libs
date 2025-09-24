#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"     // Biblioteca da Raspberry Pi Pico para funções padrão (GPIO, temporização, etc.)
#include "pico/cyw43_arch.h" // Biblioteca para arquitetura Wi-Fi da Pico com CYW43
#include "pico/unique_id.h"  // Biblioteca com recursos para trabalhar com os pinos GPIO do Raspberry Pi Pico

#include "lwip/apps/mqtt.h"      // Biblioteca LWIP MQTT -  fornece funções e recursos para conexão MQTT
#include "lwip/apps/mqtt_priv.h" // Biblioteca que fornece funções e recursos para Geração de Conexões
#include "lwip/dns.h"            // Biblioteca que fornece funções e recursos suporte DNS:
#include "lwip/altcp_tls.h"      // Biblioteca que fornece funções e recursos para conexões seguras usando TLS:

#include "mqtt_pico.h"

bool mqtt_start(
    mqtt_config_t * mqtt,
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
) {
    mqtt->client_info.client_id = mqtt_generate_client_id(device_name);

    if (!mqtt->client_info.client_id) {
        return false; // erro ao criar o client_id unico
    }

    mqtt->client_info.client_user = username;
    mqtt->client_info.client_pass = password;
    mqtt->client_info.keep_alive = keep_alive;
    mqtt->client_info.will_msg = will_msg;
    mqtt->client_info.will_qos = will_qos;
    mqtt->client_info.will_retain = true;
    mqtt->sub_qos = sub_qos;
    mqtt->pub_qos = pub_qos;
    mqtt->unique_topic = unique_topic;

    char *_topic_buf = malloc(MQTT_TOPIC_LEN);
    if (!_topic_buf) {
        return false;
    }
    strncpy(_topic_buf, mqtt_full_topic(mqtt, will_topic), MQTT_TOPIC_LEN);
    mqtt->client_info.will_topic = _topic_buf;

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

    return true;
}

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
}

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

char *mqtt_full_topic(mqtt_config_t *mqtt, char *name) {
    if (mqtt->unique_topic) {
        static char full_topic[MQTT_TOPIC_LEN];
        snprintf(full_topic, MQTT_TOPIC_LEN, "/%s%s", mqtt->client_info.client_id, name);
        return full_topic;
    } else {
        return name;
    }
}

void mqtt_manage_topics(mqtt_config_t *mqtt, char **topics, size_t num_topics, mqtt_action_t action, mqtt_request_cb_t cb) {
    for (int i = 0; i < num_topics; i++) {
        mqtt_sub_unsub(mqtt->client, mqtt_full_topic(mqtt, topics[i]), mqtt->sub_qos, cb, mqtt, action);
    }
}

/** ---------------------------------------------------------------------------- */
/**-------------------- funções genericas para ser usadas como callback -------- */

void mqtt_dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;
    if (ipaddr) {
        mqtt->server_ip = *ipaddr;
    } else {
        printf("[MQTT ERROR] DNS REQUEST FAILED");
    }
}

void mqtt_g_pub_cb(__unused void *arg, err_t err) {
    if (err != 0) {
        printf("[MQTT ERROR] Error during publish data: %d", err);
    }
}

void mqtt_g_sub_cb(void *arg, err_t err) {
    mqtt_config_t *mqtt = (mqtt_config_t *) arg;
    if (err != 0) {
        return;
    }
    mqtt->sub_count++;
}

void mqtt_g_unsub_cb(void *arg, err_t err){
    mqtt_config_t *mqtt = (mqtt_config_t *) arg;
    if (err != 0) {
        return;
    }
    mqtt->sub_count--;

    if (mqtt->sub_count <= 0 && mqtt->stop_client) {
        mqtt_disconnect(mqtt->client);
    }
}

void mqtt_g_in_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;

    strncpy(mqtt->data.data, (const char *)data, len);
    mqtt->data.len = (uint32_t) len;
    mqtt->data.data[len] = '\0';
}

void mqtt_g_in_pub_cb(void *arg, const char *topic, u32_t tot_len) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;
    
    size_t len = strlen(topic);
    if (len >= sizeof(mqtt->data.topic)) { // verifica para nao passar do tamanho do buffer topic
        len = sizeof(mqtt->data.topic) - 1;
    }

    memcpy(mqtt->data.topic, topic, len);
    mqtt->data.topic[len] = '\0';
}

void mqtt_g_connection_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    mqtt_config_t *mqtt = (mqtt_config_t *) arg;

    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt->connect_done = true;
        char *topics[] = {"/test"};
        size_t num_topics = sizeof(topics)/sizeof(topics[0]);
        mqtt_manage_topics(mqtt, topics, num_topics, MQTT_SUBSCRIBE, mqtt_g_sub_cb);

        if (mqtt->client_info.will_topic) {
            mqtt_publish(
                mqtt->client,
                mqtt->client_info.will_topic,
                "1",
                1,
                mqtt->client_info.will_qos,
                mqtt->client_info.will_retain,
                mqtt_g_pub_cb,
                mqtt
            );
        }
    } else if (status == MQTT_CONNECT_DISCONNECTED) {
        if (!mqtt->connect_done) {
            printf("[MQTT ERROR] FAILED TO CONNECT TO MQTT SERVER");
        }
    } else {
        printf("[MQTT ERROR] UNEXPECTED CONNECT STATUS");
    }
}