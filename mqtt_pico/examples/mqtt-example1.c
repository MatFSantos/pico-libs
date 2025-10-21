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

// WIFI credentials
#include "credentials.h"

#define DEVICE_NAME     "pico"
#define KEEP_ALIVE      60
#define WILL_TOPIC      "alive"

/**
 * [0] - No máximo uma vez
 * [1] - Ao menos uma vez
 * [2] - Exatamente uma vez
 */
#define WILL_QOS        1
#define PUB_QOS         1
#define SUB_QOS         1
#define RETAIN_MSG      false
#define UNIQUE_TOPIC    false
#define LED_PIN CYW43_WL_GPIO_LED_PIN /* LED of the rp pico w board */

void dns_found(const char *hostname, const ip_addr_t *ipaddr, void *arg) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;
    if (ipaddr) {
        mqtt->server_ip = *ipaddr;
    }
}

void pub_cb(__unused void *arg, err_t err) {
    if (err != 0) {
        return;
    }
}

void sub_cb(void *arg, err_t err) {
    mqtt_config_t *mqtt = (mqtt_config_t *) arg;
    if (err != 0) {
        return;
    }
    mqtt->sub_count++;
}

void in_data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;

    strncpy(mqtt->data.data, (const char *)data, len);
    mqtt->data.len = (uint32_t) len;
    mqtt->data.data[len] = '\0';

    printf("Topico: %s\n", mqtt->data.topic);
    printf("Mensagem: %s\n", mqtt->data.data);
}

void in_pub_cb(void *arg, const char *topic, u32_t tot_len) {
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;
    
    size_t len = strlen(topic);
    if (len >= sizeof(mqtt->data.topic)) { // verifica para nao passar do tamanho do buffer topic
        len = sizeof(mqtt->data.topic) - 1;
    }

    memcpy(mqtt->data.topic, topic, len);
    mqtt->data.topic[len] = '\0';
}

void conn_cb(mqtt_client_t *client, void *arg, mqtt_connection_status_t status) {
    mqtt_config_t *mqtt = (mqtt_config_t *) arg;

    if (status == MQTT_CONNECT_ACCEPTED) {
        mqtt->connect_done = true;
        char *topics[] = {
            mqtt_full_topic(mqtt, "test-sub")
        };
        size_t num_topics = sizeof(topics)/sizeof(topics[0]);
        mqtt_manage_topics(mqtt, topics, num_topics, MQTT_SUBSCRIBE, sub_cb);

        printf("[MQTT CONN] MQTT CONNECTED\n");
    } else if (status == MQTT_CONNECT_DISCONNECTED) {
        if (!mqtt->connect_done) {
            printf("[MQTT CONN] MQTT DISCONNECTED\n");
        }
    } else {
        printf("[MQTT CONN] BAD STATUS: %d\n", status);
    }
}

int main() {
    stdio_init_all();
    sleep_ms(5000); // espera para que seja possível debugar via monitor serial

    //inicializa arquitetura do modulo WiFi
    printf("Inicializando CYW43 ARCH");
    while (cyw43_arch_init()) {
        printf(".");
        sleep_ms(500);
    }
    printf("\nCYW43 ARCH inicializado com sucesso!\n");

    // GPIO do CI CYW43 em nível alto
    cyw43_arch_gpio_put(LED_PIN, 1);

    // Ativa o Wi-Fi no modo Station, de modo a que possam ser feitas ligações a outros pontos de acesso Wi-Fi.
    cyw43_arch_enable_sta_mode();

    int attemps = 0;
    printf("Conectando WiFi");
    while(cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK, 5000)){
        if (attemps > 2) {
            panic("[WIFI ERROR] ERROR TO CONNECT WIFI");
        }
        printf("..");
        sleep_ms(100);
        attemps++;
    }
    printf("\nWiFi conectado com sucesso!\n");

    // struct para MQTT
    mqtt_config_t mqtt;
    mqtt.client_info.client_id = mqtt_generate_client_id(DEVICE_NAME);
    if (!mqtt.client_info.client_id) {
        // se ocorrer erro ao criar o client_id único, utiliza o DEVICE_NAME
        mqtt.client_info.client_id = DEVICE_NAME;
    }

    mqtt.client_info.client_user = MQTT_USERNAME;
    mqtt.client_info.client_pass = MQTT_PASSWORD;
    mqtt.client_info.keep_alive = KEEP_ALIVE;

    mqtt.client_info.will_topic = WILL_TOPIC;
    mqtt.client_info.will_msg = mqtt.client_info.client_id;
    mqtt.client_info.will_qos = WILL_QOS;
    mqtt.client_info.will_retain = true;

    mqtt.sub_qos = SUB_QOS;
    mqtt.pub_qos = PUB_QOS;
    mqtt.retain = RETAIN_MSG;
    mqtt.unique_topic = UNIQUE_TOPIC;

    mqtt_tls(&mqtt); // configura o certificado TLS, se definido.

    mqtt_start_client(&mqtt, MQTT_SERVER, conn_cb, in_pub_cb, in_data_cb, dns_found);

    while(true){
        if (mqtt.connect_done) {
            char msg[] = "im alive";
            mqtt_publish(
                mqtt.client,
                mqtt_full_topic(&mqtt, "test-pub"),
                msg,
                strlen(msg),
                mqtt.pub_qos,
                mqtt.retain,
                pub_cb,
                &mqtt
            );
            printf("Mensagem publicada\n");
        }
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
        sleep_ms(2000);
    }

    // Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}