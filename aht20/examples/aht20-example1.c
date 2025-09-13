#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "aht20.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

int main() {
    stdio_init_all();

    // Inicializa o I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o AHT20
    bool is_active_aht20 = aht20_reset(I2C_PORT);
    AHT20_Data data;
    while (true) {

        if (aht20_read(I2C_PORT, &data) && is_active_aht20) {
            printf("AHT20 ---- Temperatura: %.2f C ||| Umidade: %.2f %%\n", data.temperature, data.humidity);
        } else if (is_active_aht20) {
            printf("AHT20 ---- Erro na leitura do AHT20!\n");
        } else {
            printf("AHT20 ---- Sensor AHT20 não está ativo!\n");
        }

        sleep_ms(1000);
    }
}