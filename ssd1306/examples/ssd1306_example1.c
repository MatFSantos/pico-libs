#include <stdio.h>
#include <string.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#include "ssd1306.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15

#define WIDTH 128
#define HEIGHT 64

int main() {
    stdio_init_all();

    // Inicializa o I2C
    i2c_init(I2C_PORT, 400 * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    // Inicializa o display SSD1306
    ssd1306_t ssd = ssd1306_init(WIDTH, HEIGHT, SSD1306_ADDRESS, I2C_PORT);
    bool success = ssd1306_config(&ssd);
    if (success) {
        char *message = "Hello, World!";
        ssd1306_draw_centered(&ssd, message, 27, true, true);
        ssd1306_send_data(&ssd);
    }

    while (1) {
        if (!success) {
            printf("SSD1306 initialization failed!\n");
        }
        sleep_ms(1000);
    }
}