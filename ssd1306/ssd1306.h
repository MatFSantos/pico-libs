#ifndef SSD1306_H
#define SSD1306_H

#include <stdlib.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

#define I2C_ADDRESS 0x3C

typedef enum {
    SET_CONTRAST = 0x81,
    SET_ENTIRE_ON = 0xA4,
    SET_NORM_INV = 0xA6,
    SET_DISP = 0xAE,
    SET_MEM_ADDR = 0x20,
    SET_COL_ADDR = 0x21,
    SET_PAGE_ADDR = 0x22,
    SET_DISP_START_LINE = 0x40,
    SET_SEG_REMAP = 0xA0,
    SET_MUX_RATIO = 0xA8,
    SET_COM_OUT_DIR = 0xC0,
    SET_DISP_OFFSET = 0xD3,
    SET_COM_PIN_CFG = 0xDA,
    SET_DISP_CLK_DIV = 0xD5,
    SET_PRECHARGE = 0xD9,
    SET_VCOM_DESEL = 0xDB,
    SET_CHARGE_PUMP = 0x8D
} ssd1306_command_t;

typedef struct {
    uint8_t width;          // Largura do display em pixels
    uint8_t height;         // Altura do display em pixels
    uint8_t pages;          // Número de páginas
    uint8_t address;        // Endereço I2C do display
    i2c_inst_t *i2c;        // Instância I2C
    uint8_t *ram_buffer;    // Buffer de RAM para o display
    size_t bufsize;         // Tamanho do buffer de RAM
    uint8_t port_buffer[2]; // Buffer
} ssd1306_t;

/**
 * @brief Inicializa a estrutura do display SSD1306.
 * 
 * @param width Largura do display em pixels (geralmente 128).
 * @param height Altura do display em pixels (geralmente 64 ou 32).
 * @param address Endereço I2C do display (geralmente 0x3C).
 * @param i2c Ponteiro para a instância I2C a ser usada.
 * 
 * @return ssd1306_t Estrutura inicializada do display SSD1306.
 */
ssd1306_t ssd1306_init(uint8_t width, uint8_t height, uint8_t address, i2c_inst_t *i2c);

/**
 * @brief Configura o display SSD1306 com os comandos iniciais.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * 
 * @return true Se todas as configurações foram bem-sucedidas.
 * @return false Se alguma configuração falhou.
 */
bool ssd1306_config(ssd1306_t *ssd);

/**
 * @brief Envia um comando para o display SSD1306. Esse comando tem largura de 2 bytes.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param command Comando a ser enviado. (Exemplo: SET_DISP)
 * 
 * @return true Se o comando foi enviado com sucesso.
 * @return false Se houve falha ao enviar o comando.
 */
bool ssd1306_command(ssd1306_t *ssd, uint8_t command);

/**
 * @brief Envia o buffer de dados para o display SSD1306.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * 
 * @return true Se os dados foram enviados com sucesso.
 * @return false Se houve falha ao enviar os dados.
 */
bool ssd1306_send_data(ssd1306_t *ssd);

/**
 * @brief Define o estado de um pixel específico no buffer de RAM.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param x Coordenada x do pixel (0 a width-1).
 * @param y Coordenada y do pixel (0 a height-1).
 * @param color true para acender o pixel, false para apagar.
 */
void ssd1306_pixel(ssd1306_t *ssd, uint8_t x, uint8_t y, bool color);

/**
 * @brief Preenche toda a tela com a cor especificada.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param color true para acender todos os pixels, false para apagar todos.
 */
void ssd1306_fill(ssd1306_t *ssd, bool color);

/**
 * @brief Desenha um retângulo na tela.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param top Coordenada y do topo do retângulo. De 0 a height-1, sendo 0 o topo.
 * @param left Coordenada x do lado esquerdo do retângulo. De 0 a width-1, sendo 0 a esquerda.
 * @param width Largura do retângulo em pixels.
 * @param height Altura do retângulo em pixels.
 * @param color true para acender os pixels do retângulo, false para apagar.
 * @param fill true para preencher o retângulo, false para apenas contornar.
 */
void ssd1306_rect(ssd1306_t *ssd, uint8_t top, uint8_t left, uint8_t width, uint8_t height, bool color, bool fill);

/**
 * @brief Desenha uma linha entre dois pontos usando o algoritmo de Bresenham.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param x0 Coordenada x do ponto inicial.
 * @param y0 Coordenada y do ponto inicial.
 * @param x1 Coordenada x do ponto final.
 * @param y1 Coordenada y do ponto final.
 * @param color true para acender os pixels da linha, false para apagar.
 */
void ssd1306_line(ssd1306_t *ssd, uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, bool color);

/**
 * @brief Desenha uma linha horizontal.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param x0 Coordenada x do início da linha.
 * @param x1 Coordenada x do fim da linha.
 * @param y Coordenada y da linha.
 * @param color true para acender os pixels da linha, false para apagar.
 */
void ssd1306_hline(ssd1306_t *ssd, uint8_t x0, uint8_t x1, uint8_t y, bool color);

/**
 * @brief Desenha uma linha vertical.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param x Coordenada x da linha.
 * @param y0 Coordenada y do início da linha.
 * @param y1 Coordenada y do fim da linha.
 * @param color true para acender os pixels da linha, false para apagar.
 */
void ssd1306_vline(ssd1306_t *ssd, uint8_t x, uint8_t y0, uint8_t y1, bool color);

/**
 * @brief Desenha um caractere na tela usando a fonte embutida. Vide `ssd1306_font.h`.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param c Caractere a ser desenhado. Deve estar na faixa ASCII de 32 a 126.
 * @param x Coordenada x do canto superior esquerdo do caractere.
 * @param y Coordenada y do canto superior esquerdo do caractere.
 */
void ssd1306_draw_char(ssd1306_t *ssd, char c, uint8_t x, uint8_t y);

/**
 * @brief Desenha uma string na tela usando a fonte embutida. Vide `ssd1306_font.h`.
 * 
 * @param ssd Ponteiro para a estrutura do display SSD1306.
 * @param str Ponteiro para a string a ser desenhada. Deve estar na faixa ASCII de 32 a 126.
 * @param x Coordenada x do canto superior esquerdo do primeiro caractere.
 * @param y Coordenada y do canto superior esquerdo do primeiro caractere.
 */
void ssd1306_draw_string(ssd1306_t *ssd, const char *str, uint8_t x, uint8_t y);

#endif // SSD1306_H