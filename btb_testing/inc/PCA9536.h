/*
 * PCA9536.h
 *
 * Created on: Mar 31, 2022
 *  Author: Eric Li
 */

#ifndef PCA9536_H_
#define PCA9536_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include "inc/hw_i2c.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"

#include "driverlib/interrupt.h" // These should be inside your CCS workspace directory
#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"

#define PCA9536_ADDRESS_READ (0x41 << 1) & 0xfe
#define PCA9536_ADDRESS_WRITE (0x41 << 1) & 0xff
#define PCA9536_ADDRESS 0x41

#define PCA9536_PIN_INPUT 0x00
#define PCA9536_PIN_OUTPUT 0x01
#define PCA9536_PIN_POLARITY 0x02
#define PCA9536_PIN_CONFIG 0x03 
#define PCA9536_PIN_CONFIG_ALL (PCA9536_PIN_CONFIG << 4) | 0b0000 // Enable all pins as configs

#define PCA9536_PIN_1 0x1
#define PCA9536_PIN_2 0x2
#define PCA9536_PIN_3 0x4
#define PCA9536_PIN_4 0x8

#define PCA9536_PORT1 PCA9536_PIN_3 | PCA9536_PIN_4 // 1100
#define PCA9536_PORT2 PCA9536_PIN_1 | PCA9536_PORT1 // 1101
#define PCA9536_PORT3 PCA9536_PIN_2 | PCA9536_PORT1 // 1110
#define PCA9536_PORT4 PCA9536_PIN_1 | PCA9536_PIN_2 | PCA9536_PORT1 // 1111

#define PCA9536_NUM_I2C_DATA 3 // I2C transmits at most 3 bytes of data: Address, Command, Data

void init_gpio_periph(void);

void InitI2C0(void);

uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg);

void I2CSendString(uint32_t slave_addr, char array[]);

void InitConsole(void);

void init(void);

void FlashLED(int portnum);

#endif /* PCA9536_H_ */
