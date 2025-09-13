#include <stdio.h>
#include <pico/stdlib.h>

#include "max6675.h"

#define SCKL_PIN 2 // GPIO 2
#define CS_PIN   3 // GPIO 3
#define MISO_PIN 4 // GPIO 4

int main() {
    stdio_init_all();

    MAX6675_t max6675 = max6675_init(SCKL_PIN, CS_PIN, MISO_PIN);
    MAX6675_Data data;

    while (true) {

        if(max6675_read(max6675, &data)) {
            printf("MAX6675 - Temperatura: %.2f °C || Temperatura: %.2f °F\n", data.t_celsius, data.t_fahrenheit);
        } else {
            printf("MAX6675 - Termopar desconectado!\n");
        }

        sleep_ms(1000);
    }
}
