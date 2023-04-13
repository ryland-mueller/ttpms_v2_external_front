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


float ta_shift = 8;			// Default shift for MLX90641 in open air
float emissivity = 0.95;	// Tune this with testing

paramsMLX90640 mlx90640;

static float mlx90640To[MLX90640_PIXEL_NUM];

float Vdd;
float Ta;

int counter = 0;


/* --- WORK AND TIMER FUNCTIONS START --- */

void MLX_temp_read_work_handler(struct k_work *work)
{
	int status;

	uint16_t frame[834];

	status = MLX90640_GetFrameData(MLX_ADDR, frame);
	if (status != 0) {
		LOG_ERR("Getting MLX frame data failed, MLX90640_GetFrameData() returned %d", status);
	}

	Vdd = MLX90640_GetVdd(frame, &mlx90640);

	Ta = MLX90640_GetTa(frame, &mlx90640);

	float tr = Ta - ta_shift; //Reflected temperature based on the sensor ambient temperature

	MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);
	

	// BLE notify
}

K_WORK_DEFINE(MLX_temp_read_work, MLX_temp_read_work_handler);

void MLX_temp_read_timer_handler(struct k_timer *dummy)
{
	k_work_submit(&MLX_temp_read_work);
}

K_TIMER_DEFINE(MLX_temp_read_timer, MLX_temp_read_timer_handler, NULL);

/* --- WORK AND TIMER FUNCTIONS END --- */


void MLX_start(void)
{
	int status;

	#ifdef BOARD_TTPMS_EXM_2_0
		// turn on MLX power MOSFET and wait short time
	#endif

	uint16_t eeMLX90640[MLX90640_EEPROM_DUMP_NUM];

	status = MLX90640_DumpEE(MLX_ADDR, eeMLX90640);
	if (status != 0) {
		LOG_ERR("Failed to load system parameters, MLX90640_DumpEE() returned %d", status);
	}

	status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
	if (status != 0) {
		LOG_ERR("Parameter extraction failed, MLX90640_ExtractParameters() returned %d", status);
	}

	status = MLX90640_SetRefreshRate(MLX_ADDR, temp_freq);
	if (status != 0) {
		LOG_ERR("Setting refresh rate failed, MLX90640_SetRefreshRate() returned %d", status);
	}

	status = MLX90640_SynchFrame(MLX_ADDR);
	if (status != 0) {
		LOG_ERR("Synchronizing MLX frame failed, MLX90640_SynchFrame() returned %d", status);
	}

	//start timer

	
}

void MLX_stop(void)
{
	#ifdef BOARD_TTPMS_EXM_2_0
		// turn off MLX power MOSFET
	#endif

	// stop timer
}


void main(void)
{
	LOG_INF("Running ttpms_v2_external_front on %s", CONFIG_BOARD);

	LOG_INF("Starting MLX sensor");
	MLX_start();


}
