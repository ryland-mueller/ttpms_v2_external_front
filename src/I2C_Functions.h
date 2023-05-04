#ifndef _I2C_Functions_H_
#define _I2C_Functions_H_

#include <stdint.h>

    extern void I2C_Init(void);
    extern int I2C_GeneralReset(void);
    extern int MLX_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
    extern int MLX_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data);
    
    #define MLX90641_ADDR   0x33
    #define MLX90640_ADDR   0x33
    #define MLX90621_ADDR   0x60

#endif