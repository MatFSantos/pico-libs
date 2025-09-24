#ifndef ST7735_H
#define ST7735_H

#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/spi.h>
#include "st7735_font.h"

//#define USE_SPI_DMA // Caso for usar DMA para SPI

#define ST7735_COLOR565(r, g, b) (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))
#define SWAP_INT16_T(a, b) { int16_t t = a; a = b; b = t; }
#define DELAY 0x80

typedef struct {
    spi_inst_t * spi;
    uint8_t cs;
    uint8_t sck;
    uint8_t mosi;
    uint8_t rst;
    uint8_t dc;
    uint8_t blk;
    uint16_t width;
    uint16_t height;
    uint8_t x_start;
    uint8_t y_start;
    uint8_t data_rotation;
    uint8_t value_rotation;
} st7735_t;

typedef enum {
    ST7735_MADCTL_MY  = 0x80,
    ST7735_MADCTL_MX  = 0x40,
    ST7735_MADCTL_MV  = 0x20,
    ST7735_MADCTL_RGB = 0x00,
    ST7735_MADCTL_BGR = 0x08
} st7735_rotation_t;

typedef enum {
    ST7735_1_8_DEFAULT_ORIENTATION  = 0, // AliExpress/eBay 1.8" display, default orientation
    ST7735S_1_8_DEFAULT_ORIENTATION = 1, // WaveShare ST7735S-based 1.8" display, default orientation
    ST7735_1_44_DEFAULT_ORIENTATION = 2, // 1.44" display, default orientation
    ST7735_MINI_DEFAULT_ORIENTATION = 3  // mini 160x80 display (it's unlikely you want the default orientation)
} st7735_model_t;

typedef enum {
    ST7735_NOP     = 0x00,
    ST7735_SWRESET = 0x01,
    ST7735_RDDID   = 0x04,
    ST7735_RDDST   = 0x09,

    ST7735_SLPIN   = 0x10,
    ST7735_SLPOUT  = 0x11,
    ST7735_PTLON   = 0x12,
    ST7735_NORON   = 0x13,

    ST7735_INVOFF  = 0x20,
    ST7735_INVON   = 0x21,
    ST7735_DISPOFF = 0x28,
    ST7735_DISPON  = 0x29,
    ST7735_CASET   = 0x2A,
    ST7735_RASET   = 0x2B,
    ST7735_RAMWR   = 0x2C,
    ST7735_RAMRD   = 0x2E,

    ST7735_PTLAR   = 0x30,
    ST7735_COLMOD  = 0x3A,
    ST7735_MADCTL  = 0x36,

    ST7735_FRMCTR1 = 0xB1,
    ST7735_FRMCTR2 = 0xB2,
    ST7735_FRMCTR3 = 0xB3,
    ST7735_INVCTR  = 0xB4,
    ST7735_DISSET5 = 0xB6,

    ST7735_PWCTR1  = 0xC0,
    ST7735_PWCTR2  = 0xC1,
    ST7735_PWCTR3  = 0xC2,
    ST7735_PWCTR4  = 0xC3,
    ST7735_PWCTR5  = 0xC4,
    ST7735_VMCTR1  = 0xC5,

    ST7735_RDID1   = 0xDA,
    ST7735_RDID2   = 0xDB,
    ST7735_RDID3   = 0xDC,
    ST7735_RDID4   = 0xDD,

    ST7735_PWCTR6  = 0xFC,

    ST7735_GMCTRP1 = 0xE0,
    ST7735_GMCTRN1 = 0xE1
} st7735_command_t;

typedef enum {
    ST7735_BLACK   = 0x0000,
    ST7735_BLUE    = 0x001F,
    ST7735_RED     = 0xF800,
    ST7735_GREEN   = 0x07E0,
    ST7735_CYAN    = 0x07FF,
    ST7735_MAGENTA = 0xF81F,
    ST7735_YELLOW  = 0xFFE0,
    ST7735_WHITE   = 0xFFFF
} st7735_color_t;

void st7735_backlight(bool state);

st7735_t st7735_init(spi_inst_t *spi, uint8_t cs, uint8_t sck, uint8_t mosi, uint8_t rst, uint8_t dc, uint8_t blk, st7735_model_t model);

void st7735_draw_pixel(st7735_t *st, uint16_t x, uint16_t y, st7735_color_t color);

void st7735_draw_string(st7735_t *st, uint16_t x, uint16_t y, const char* str, st7735_font_def_t font, st7735_color_t color, st7735_color_t bgcolor);

void st7735_fill_screen(st7735_t *st, st7735_color_t color);

void st7735_draw_image(st7735_t *st, uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint16_t* data);

void st7735_invert_colors(st7735_t *st, bool invert);

void st7735_draw_circle(st7735_t *st, int16_t x0, int16_t y0, int16_t r, st7735_color_t color);

void st7735_draw_circle_helper(st7735_t *st, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, st7735_color_t color);

void st7735_fill_circle(st7735_t *st, int16_t x0, int16_t y0, int16_t r, st7735_color_t color);

void st7735_fill_circle_helper(st7735_t *st, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, st7735_color_t color);

void st7735_draw_ellipse(st7735_t *st, int16_t x0, int16_t y0, int16_t rx, int16_t ry, st7735_color_t color);

void st7735_fill_ellipse(st7735_t *st, int16_t x0, int16_t y0, int16_t rx, int16_t ry, st7735_color_t color);

void st7735_draw_rect(st7735_t *st, int16_t x, int16_t y, int16_t width, int16_t height, st7735_color_t color);

void st7735_fill_rect(st7735_t *st, int16_t x, int16_t y, int16_t width, int16_t height, st7735_color_t color);

void st7735_draw_round_rect(st7735_t *st, int16_t x, int16_t y, int16_t width, int16_t height, int16_t r, st7735_color_t color);

void st7735_fill_round_rect(st7735_t *st, int16_t x, int16_t y, int16_t width, int16_t height, int16_t r, st7735_color_t color);

void st7735_draw_triangle(st7735_t *st, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, st7735_color_t color);

void st7735_fill_triangle(st7735_t *st, int16_t x0, int16_t y0, int16_t x1, int16_t y1, int16_t x2, int16_t y2, st7735_color_t color);

void st7735_draw_line(st7735_t *st, int16_t x0, int16_t y0, int16_t x1, int16_t y1, st7735_color_t color);

void st7735_draw_vline(st7735_t *st, int16_t x, int16_t y, int16_t height, st7735_color_t color);

void st7735_draw_hline(st7735_t *st, int16_t x, int16_t y, int16_t width, st7735_color_t color);

void st7735_set_rotation(st7735_t *st, uint8_t m);

uint8_t st7735_get_rotation();

int16_t st7735_get_height();

int16_t st7735_get_width();

#endif // ST7735_H