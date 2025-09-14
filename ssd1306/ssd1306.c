#include "ssd1306.h"
#include "ssd1306_font.h"

ssd1306_t ssd1306_init(uint8_t width, uint8_t height, uint8_t address, i2c_inst_t *i2c) {
    ssd1306_t ssd;
    ssd.width = width;
    ssd.height = height;
    ssd.pages = height / 8U;
    ssd.address = address;
    ssd.i2c = i2c;
    ssd.bufsize = ssd.pages * ssd.width + 1;
    ssd.ram_buffer = calloc(ssd.bufsize, sizeof(uint8_t));
    ssd.ram_buffer[0] = 0x40; // Control byte for data
    ssd.port_buffer[0] = 0x80; // Control byte for command

    return ssd;
}

bool ssd1306_command(ssd1306_t *ssd, uint8_t command) {
    ssd->port_buffer[1] = command;
    int ret = i2c_write_blocking(ssd->i2c, ssd->address, ssd->port_buffer, 2, false);
    return ret == 2; // Retorna true se 2 bytes foram escritos
}

bool ssd1306_config(ssd1306_t *ssd) {
    bool success = true;
    success &= ssd1306_command(ssd, SET_DISP | 0x00);
    success &= ssd1306_command(ssd, SET_MEM_ADDR);
    success &= ssd1306_command(ssd, 0x01);
    success &= ssd1306_command(ssd, SET_DISP_START_LINE | 0x00);
    success &= ssd1306_command(ssd, SET_SEG_REMAP | 0x01);
    success &= ssd1306_command(ssd, SET_MUX_RATIO);
    success &= ssd1306_command(ssd, ssd->height - 1);
    success &= ssd1306_command(ssd, SET_COM_OUT_DIR | 0x08);
    success &= ssd1306_command(ssd, SET_DISP_OFFSET);
    success &= ssd1306_command(ssd, 0x00);
    success &= ssd1306_command(ssd, SET_COM_PIN_CFG);
    success &= ssd1306_command(ssd, 0x12);
    success &= ssd1306_command(ssd, SET_DISP_CLK_DIV);
    success &= ssd1306_command(ssd, 0x80);
    success &= ssd1306_command(ssd, SET_PRECHARGE);
    success &= ssd1306_command(ssd, 0xF1);
    success &= ssd1306_command(ssd, SET_VCOM_DESEL);
    success &= ssd1306_command(ssd, 0x30);
    success &= ssd1306_command(ssd, SET_CONTRAST);
    success &= ssd1306_command(ssd, 0xFF);
    success &= ssd1306_command(ssd, SET_ENTIRE_ON);
    success &= ssd1306_command(ssd, SET_NORM_INV);
    success &= ssd1306_command(ssd, SET_CHARGE_PUMP);
    success &= ssd1306_command(ssd, 0x14);
    success &= ssd1306_command(ssd, SET_DISP | 0x01);
    return success; // Retorna true se todas as configurações foram bem-sucedidas
}

bool ssd1306_send_data(ssd1306_t *ssd) {
    bool success = true;
    success &= ssd1306_command(ssd, SET_COL_ADDR);
    success &= ssd1306_command(ssd, 0);
    success &= ssd1306_command(ssd, ssd->width - 1);
    success &= ssd1306_command(ssd, SET_PAGE_ADDR);
    success &= ssd1306_command(ssd, 0);
    success &= ssd1306_command(ssd, ssd->pages - 1);
    int ret = i2c_write_blocking(ssd->i2c, ssd->address, ssd->ram_buffer, ssd->bufsize, false);
    success &= (ret == (int)ssd->bufsize); // Verifica se todos os bytes foram escritos
    return success;
}

void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool color) {
    uint16_t index = (y >> 3) + (x << 3) + 1;
    uint8_t pixel = (y & 0b111);
    if (color) {
        ssd->ram_buffer[index] |= (1 << pixel);
    } else {
        ssd->ram_buffer[index] &= ~(1 << pixel);
    }
}

void ssd1306_fill(ssd1306_t *ssd, bool color) {
    for (uint8_t y = 0; y < ssd->height; ++y) {
        for (uint8_t x = 0; x < ssd->width; ++x) {
            ssd1306_pixel(ssd, x, y, color);
        }
    }
}

void ssd1306_rect(
    ssd1306_t *ssd,
    uint8_t top,
    uint8_t left,
    uint8_t width,
    uint8_t height,
    bool color,
    bool fill
) {
    // duas linhas horizontais paralelas, com uma distância de 'height' entre elas
    for (uint8_t x = left; x < left + width; ++x) {
        ssd1306_pixel(ssd, x, top, color);
        sdd1306_pixel(ssd, x, top + height - 1, color);
    }

    // duas linhas verticais paralelas, com uma distância de 'width' entre elas
    for (uint8_t y = top; y < top + height; ++y) {
        ssd1306_pixel(ssd, left, y, color);
        ssd1306_pixel(ssd, left + width - 1, y, color);
    }

    // se preencher, preenche o retângulo
    if (fill) {
        for (uint8_t y = top + 1; y < top + height - 1; ++y) {
            for (uint8_t x = left + 1; x < left + width - 1; ++x) {
                ssd1306_pixel(ssd, x, y, color);
            }
        }
    }
}

void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color) {
    int dx = abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;

    int dy = abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;

    int err = dx - dy;

    while (true) {
        ssd1306_pixel(ssd, x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break; // termina o loop quando o ponto final é alcançado
        }

        int e2 = err * 2;
        if (e2 > -dy) {
            err -= dy;
            x0 += sx;
        }

        if (e2 < dx) {
            err += dx;
            y0 += sy;
        }
    }
}

void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool color) {
    for (uint8_t x = x0; x <= x1; ++x){
        ssd1306_pixel(ssd, x, y, color);
    }
}

void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool color) {
    for (uint8_t y = y0; y <= y1; ++y){
        ssd1306_pixel(ssd, x, y, color);
    }
}

void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y) {
    uint16_t index = 0;

    // Verifica o caractere e calcula o índice correspondente na fonte
    if (c >= ' ' && c <= '~') { // Verifica se o caractere está na faixa ASCII válida
        index = (c - ' ') * 8; // Calcula o índice baseado na posição do caractere na tabela ASCII
    } else { // Caractere inválido, desenha um espaço (ou pode ser tratado de outra forma)
        index = 0; // Índice 0 corresponde ao caractere "nada" (espaço)
    }

    // Desenha o caractere na tela
    for (uint8_t i = 0; i < 8; ++i) {
        uint8_t line = font[index + i]; // Acessa a linha correspondente do caractere na fonte
        for (uint8_t j = 0; j < 8; ++j) {
            ssd1306_pixel(ssd, x + i, y + j, line & (1 << j)); // Desenha cada pixel do caractere
        }
    }
}

void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y) {
    while (*str) {
        ssd1306_draw_char(ssd, *str++, x, y);
        x += 8;
        if (x + 8 >= ssd->width) {
            x = 0;
            y += 8;
        }
        if (y + 8 >= ssd->height) {
            break;
        }
    }
}