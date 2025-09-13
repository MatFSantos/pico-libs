/**
 * @file max6675.h
 * 
 * @brief Biblioteca para o sensor de temperatura MAX6675.
 *      Esta biblioteca permite a inicialização e leitura do sensor MAX6675
 *     via comunicação SPI bit-bang.
 * @author Mateus Fernandes Santos
 */
#ifndef MAX6675_H
#define MAX6675_H

#include <stdio.h>
#include <pico/stdlib.h>

/**
 * @brief Estrutura para configuração do sensor MAX6675.
 */
typedef struct {
    int8_t sckl; // Pino do clock (SCK)
    int8_t cs;   // Pino de chip select (CS)
    int8_t miso; // Pino de dados mestre para escravo (MISO)
} MAX6675_t;

/**
 * @brief Estrutura para armazenar os dados lidos do sensor MAX6675.
 */
typedef struct {
    float t_celsius;    // Temperatura em graus Celsius
    float t_fahrenheit; // Temperatura em graus Fahrenheit
} MAX6675_Data;

/**
 * @brief Inicializa o sensor MAX6675.
 * 
 * @param sckl Pino do clock (SCK).
 * @param cs Pino de chip select (CS).
 * @param miso Pino de dados mestre para escravo (MISO).
 * 
 * @return Estrutura MAX6675_t configurada.
 */
MAX6675_t max6675_init(uint8_t sckl, int8_t cs, int8_t miso);

/**
 * @brief Lê a temperatura em graus Celsius do sensor MAX6675.
 * 
 * @param max6675 Estrutura do sensor MAX6675.
 * @param data Ponteiro para a estrutura onde os dados serão armazenados.
 * 
 * @return true se a leitura for bem-sucedida, false caso contrário.
 */
bool max6675_read(MAX6675_t max6675, MAX6675_Data *data);

/**
 * @brief Função interna para ler um byte do sensor MAX6675.
 * 
 * @param max6675 Estrutura do sensor MAX6675.
 * @return Byte lido do sensor.
*/
uint8_t _max6675_read_byte(MAX6675_t max6675);

#endif // MAX6675_H