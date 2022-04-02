/**
 * I2C Library for Wisconsin Racing
 * Created by Eric Li
 *  on March 31, 2023
 */

#ifndef __WR_I2C_H_
#define __WR_I2C_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * Defines can be made here
 * Example: #define PCA9536_ADDRESS 0x41
 */

/**
 * Instructions: 
 *      1. Set SysCtlClockSet
 *      2. Call init() and InitI2C0()
 *      3. I2CReceive(SLAVE_ADDR, reg num) to read from slave's port
 *      4. I2CSendString(SLAVE_ADDR, command) to write command to device
 */

//*****************************************************************************
//
// Function Declarations
//
//*****************************************************************************

void init_gpio_periph(void);

void InitI2C0(void);

uint32_t I2CReceive(uint32_t slave_addr, uint8_t reg);

void I2CSendString(uint32_t slave_addr, char array[]);

void InitConsole(void);

void init(void);

void FlashLED(int portnum);

#endif /* WR_I2C_H_ */
