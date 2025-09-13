#include "max6675.h"

MAX6675_t max6675_init(uint8_t sckl, int8_t cs, int8_t miso) {
    gpio_init(sckl);
    gpio_set_dir(sckl, GPIO_OUT);
    gpio_put(sckl, 0);

    gpio_init(cs);
    gpio_set_dir(cs, GPIO_OUT);
    gpio_put(cs, 1);

    gpio_init(miso);
    gpio_set_dir(miso, GPIO_IN);

    return (MAX6675_t){ .sckl = sckl, .cs = cs, .miso = miso };
}

uint8_t _max6675_read_byte(MAX6675_t max6675) {
    uint8_t value = 0;
    for (int i = 7; i >= 0; i--) {
        gpio_put(max6675.sckl, 0);
        sleep_us(10);
        if (gpio_get(max6675.miso)) {
            value |= (1 << i);
        }
        gpio_put(max6675.sckl, 1);
        sleep_us(10);
    }
    return value;
}

bool max6675_read(MAX6675_t max6675, MAX6675_Data *data) {
    uint16_t value = 0;

    gpio_put(max6675.cs, 0);
    sleep_us(10);

    value = _max6675_read_byte(max6675);
    value <<= 8;
    value |= _max6675_read_byte(max6675);

    gpio_put(max6675.cs, 1);

    if (value & 0x4) {
        // Termopar desconectado
        return false;
    }

    // Os 3 bits menos significativos são status, então deslocamos para a direita
    value >>= 3;

    data->t_celsius = value * 0.25;
    data->t_fahrenheit = (data->t_celsius * 9.0 / 5.0) + 32.0;
    return true;
}
