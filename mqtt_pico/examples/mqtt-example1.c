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
#define WILL_TOPIC      "/alive"
#define WILL_MSG        "1"

/**
 * [0] - No máximo uma vez
 * [1] - Ao menos uma vez
 * [2] - Exatamente uma vez
 */
#define WILL_QOS        1
#define PUB_QOS         1
#define SUB_QOS         1
#define RETAIN_MSG      true
#define UNIQUE_TOPIC    false
#define LED_PIN CYW43_WL_GPIO_LED_PIN /* LED of the rp pico w board */

void data_cb(void *arg, const u8_t *data, u16_t len, u8_t flags) {
    mqtt_g_in_data_cb(arg, data, len, flags);
    mqtt_config_t *mqtt = (mqtt_config_t *)arg;

    printf("Topico: %s\n", mqtt->data.topic);
    printf("Mensagem: %s\n", mqtt->data.data);
}

int main() {
    stdio_init_all();
    sleep_ms(5000); // espera para que seja possível debugar via monitor serial

    // struct para MQTT
    mqtt_config_t mqtt;

    //inicializa arquitetura do modulo WiFi
    printf("Inicializando CYW43 ARCH");
    while (cyw43_arch_init()) {
        printf(".");
        sleep_ms(500);
    }
    printf("\nCYW43 ARCH inicializado com sucesso!\n");

    // GPIO do CI CYW43 em nível alto
    cyw43_arch_gpio_put(LED_PIN, 1);

    // struct para MQTT
    bool res = mqtt_start(
        &mqtt,
        DEVICE_NAME,
        KEEP_ALIVE,
        MQTT_USERNAME,
        MQTT_PASSWORD,
        WILL_TOPIC,
        WILL_MSG,
        WILL_QOS,
        SUB_QOS,
        PUB_QOS,
        RETAIN_MSG,
        UNIQUE_TOPIC
    );

    printf("client_id: %s\n", mqtt.client_info.client_id);
    printf("client_user: %s\n", mqtt.client_info.client_user);
    printf("client_pass: %s\n", mqtt.client_info.client_pass);
    printf("keep_alive: %d\n", mqtt.client_info.keep_alive);
    printf("will_topic: %s\n", mqtt.client_info.will_topic);
    printf("will_msg: %s\n", mqtt.client_info.will_msg);
    printf("will_qos: %d\n", mqtt.client_info.will_qos);
    printf("will_retain: %d\n", mqtt.client_info.will_retain);

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

    mqtt_start_client(&mqtt, MQTT_SERVER, mqtt_g_connection_cb, mqtt_g_in_pub_cb, data_cb, mqtt_dns_found);
    printf("MQTT CLIENT ON\n");

    while(true){
        printf("rodando aplicacao...\n");
        cyw43_arch_poll();
        cyw43_arch_wait_for_work_until(make_timeout_time_ms(10000));
        sleep_ms(2000);
    }

    printf("MQTT CLIENT EXIT\n");

    // Desligar a arquitetura CYW43.
    cyw43_arch_deinit();
    return 0;
}