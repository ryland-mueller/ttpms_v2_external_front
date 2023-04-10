/* includes that will always be required */
#include <zephyr/kernel.h>

#include "I2C_Functions.h"
#include "MLX90640_API.h"

/* includes for debugging/temporary */
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ttpms);


// Temp values have 0.5 scale, 0 offset (ie 0xAF = 175 = 87.5 C)
uint8_t temp[16];

// frequency of temp sensor updates (0x00 = 0.5Hz, 0x01 = 1Hz, 0x02 = 2Hz, 0x03 = 4Hz, 0x04 = 8Hz, 0x05 = 16Hz (default), 0x06 = 32Hz, 0x07 = 64Hz)
uint8_t temp_freq = 0x05;


paramsMLX90640 mlx90640;


void MLX_init(void)
{
	int status;

	#ifdef BOARD_TTPMS_EXM_2_0
		// turn on MLX power MOSFET and wait short time
	#endif

	uint16_t eeMLX90640[MLX90640_EEPROM_DUMP_NUM];

	status = MLX90640_DumpEE(MLX_ADDR, eeMLX90640);
	if (status != 0) {
		LOG_ERR("Failed to load system parameters, MLX90641_DumpEE() returned %d", status);
	}

	status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
	if (status != 0) {
		LOG_ERR("Parameter extraction failed, MLX90641_ExtractParameters() returned %d", status);
	}
}


void main(void)
{
	LOG_INF("Running ttpms_v2_external_front on %s", CONFIG_BOARD);


}
