#ifndef _I2C_Functions_H_
#define _I2C_Functions_H_

#include <stdint.h>
#include "MLX90640_API.h"

    extern void I2C_Init(void);
    extern int MLX90640_I2CGeneralReset(void);
    extern int MLX90640_I2CRead(uint8_t slaveAddr,uint16_t startAddress, uint16_t nMemAddressRead, uint16_t *data);
    extern int MLX90640_I2CWrite(uint8_t slaveAddr,uint16_t writeAddress, uint16_t data);
    
    #define MLX_ADDR 0x33   // MLX90640/90641 = 0x33    MLX90621 = 0x60

#endif