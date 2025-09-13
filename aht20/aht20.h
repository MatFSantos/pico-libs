/**
 * @file aht20.h
 * 
 * @brief Biblioteca para o sensor de temperatura e umidade AHT20.
 *       Esta biblioteca permite a inicialização, leitura e verificação do sensor AHT20
 *      via comunicação I2C.
 * @author Mateus Fernandes Santos
 */
#ifndef AHT20_H
#define AHT20_H

#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/i2c.h>

// I2C address for AHT20
#define AHT20_I2C_ADDR 0x38

// AHT20 command definitions
#define AHT20_CMD_INIT      0xBE
#define AHT20_CMD_TRIGGER   0xAC
#define AHT20_CMD_RESET     0xBA
#define AHT20_STATUS_BUSY 0x80       // Bit de status ocupado
#define AHT20_STATUS_CALIBRATED 0x08 // Bit de calibração

/**
 * @brief Estrutura para armazenar os dados lidos do sensor AHT20.
 */
typedef struct {
    float temperature; // Temperature in Celsius
    float humidity;    // Relative Humidity in percentage
} AHT20_Data;

/**
 * @brief Inicializa o sensor AHT20.
 * 
 * @param i2c Ponteiro para a instância I2C.
 * @return true se a inicialização for bem-sucedida, false caso contrário.
 */
bool aht20_init(i2c_inst_t *i2c);

/**
 * @brief Lê os dados de temperatura e umidade do sensor AHT20.
 * 
 * @param i2c Ponteiro para a instância I2C.
 * @param data Ponteiro para a estrutura onde os dados serão armazenados.
 * @return true se a leitura for bem-sucedida, false caso contrário.
 */
bool aht20_read(i2c_inst_t *i2c, AHT20_Data *data);

/**
 * @brief Reseta o sensor AHT20 e inicia ele novamente.
 * 
 * @param i2c Ponteiro para a instância I2C.
 * 
 * @return true se o reset e a reinicialização forem bem-sucedidos, false caso contrário.
 */
bool aht20_reset(i2c_inst_t *i2c);

/**
 * @brief Verifica se o sensor AHT20 está presente e respondendo.
 * 
 * @param i2c Ponteiro para a instância I2C.
 * @return true se o sensor responder corretamente, false caso contrário.
 */
bool aht20_check(i2c_inst_t *i2c);

#endif // AHT20_H