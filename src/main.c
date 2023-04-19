/* includes that will always be required */
#include <zephyr/kernel.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <errno.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/conn.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/gatt.h>
#include <zephyr/drivers/gpio.h>

#include "I2C_Functions.h"
#include "MLX90640_API.h"

/* includes for debugging/temporary */
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(ttpms);


// IMPORTANT: define which corner the sensor will go
// options = TTPMS_EFL, TTPMS_EFR
#define TTPMS_EFL


#define TEMP_THREAD_STACKSIZE 8192
#define TEMP_THREAD_PRIORITY 4

extern void temp_thread(void);
K_THREAD_STACK_DEFINE(temp_stack_area, TEMP_THREAD_STACKSIZE);
struct k_thread temp_thread_data;
k_tid_t temp_thread_id;


static bool volatile connection_status;

// Temp values have 0.5 scale, 0 offset (ie 0xAF = 175 = 87.5 C)
uint8_t tire_temp[16];

// frequency of temp sensor updates (0x00 = 0.5Hz, 0x01 = 1Hz, 0x02 = 2Hz, 0x03 = 4Hz, 0x04 = 8Hz, 0x05 = 16Hz (default), 0x06 = 32Hz, 0x07 = 64Hz)
// note that only one subpage is updated. 8Hz (0x04) seems to be the limit for NRF52833. 40ms required for I2C transmission, 60ms for processing
uint8_t temp_freq = 0x04;

float ta_shift = 8;			// Default shift for MLX90641 in open air
float emissivity = 0.95;	// Tune this with testing

paramsMLX90640 mlx90640;

static float mlx90640To[MLX90640_PIXEL_NUM];

float Vdd;
float Ta;


/* --- BLUETOOTH SHIT START --- */

#define BLE_ADV_INTERVAL_MIN 1200	// 1200 * 0.625ms = 0.75s
#define BLE_ADV_INTERVAL_MAX 2000	// 2000 * 0.625ms = 1.25s

// First hex char must be C for random static address
#ifdef TTPMS_EFL
	#define TTPMS_SENSOR_BT_ID "CA:69:F1:F1:11:22"
	char bt_device_name[28] = "TTPMS External Sensor FL";
#endif
#ifdef TTPMS_EFR
	#define TTPMS_SENSOR_BT_ID "CA:69:F1:F1:11:23"
	char bt_device_name[28] = "TTPMS External Sensor FR";
#endif

// Custom BT Service UUIDs
#define BT_UUID_CUSTOM_SERVICE_VAL \
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef0)

static struct bt_uuid_128 primary_service_uuid = BT_UUID_INIT_128(
	BT_UUID_CUSTOM_SERVICE_VAL);

static struct bt_uuid_128 temp_characteristic_uuid = BT_UUID_INIT_128(
	BT_UUID_128_ENCODE(0x12345678, 0x1234, 0x5678, 0x1234, 0x56789abcdef1));


static struct bt_le_adv_param adv_param;


static ssize_t read_temp(struct bt_conn *conn, const struct bt_gatt_attr *attr, void *buf, uint16_t len, uint16_t offset)
{
	int *value = &tire_temp;

	return bt_gatt_attr_read(conn, attr, buf, len, offset, value,
				 sizeof(tire_temp));
}


// Vendor Primary Service Declaration
BT_GATT_SERVICE_DEFINE(primary_service,
	BT_GATT_PRIMARY_SERVICE(&primary_service_uuid),
	BT_GATT_CHARACTERISTIC(&temp_characteristic_uuid.uuid,
			       BT_GATT_CHRC_READ,
			       BT_GATT_PERM_READ,
			       read_temp, NULL, NULL),
);

static const struct bt_data ad[] = {
	BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
	BT_DATA_BYTES(BT_DATA_UUID128_ALL, BT_UUID_CUSTOM_SERVICE_VAL),
};

static void connected(struct bt_conn *conn, uint8_t err)
{
	if (err) {
		LOG_WRN("Connection failed (err 0x%02x)", err);
	} else {
		LOG_INF("Connected");
		connection_status = true;
		temp_thread_id = k_thread_create(&temp_thread_data, temp_stack_area,
                                 K_THREAD_STACK_SIZEOF(temp_stack_area),
                                 temp_thread,
                                 NULL, NULL, NULL,
                                 TEMP_THREAD_PRIORITY, 0, K_NO_WAIT);
	}
}

static void disconnected(struct bt_conn *conn, uint8_t reason)
{
	LOG_INF("Disconnected (reason 0x%02x)", reason);
	connection_status = false;
}

BT_CONN_CB_DEFINE(conn_callbacks) = {
	.connected = connected,
	.disconnected = disconnected
};

/* --- BLUETOOTH SHIT END --- */


void main(void)
{
	LOG_INF("Running ttpms_v2_external_front on %s", CONFIG_BOARD);

	int err;

	bt_addr_le_t addr;

	err = bt_addr_le_from_str(TTPMS_SENSOR_BT_ID, "random", &addr);
	if (err) {
		LOG_ERR("Invalid BT address (err %d)", err);
	}

	err = bt_id_create(&addr, NULL);
	if (err < 0) {
		LOG_ERR("Creating new BT ID failed (err %d)", err);
	}

	err = bt_enable(NULL);
	if (err) {
		LOG_ERR("Bluetooth init failed (err %d)", err);
	} else {
		LOG_INF("Bluetooth initialized");
	}

	err = bt_set_name(bt_device_name);
	if (err) {
		LOG_ERR("Failed to set BT Device name (err %d)", err);
	} else {
		LOG_INF("Successfully set BT device name");
	}

	adv_param = *BT_LE_ADV_CONN_NAME;
	adv_param.interval_min = BLE_ADV_INTERVAL_MIN;
	adv_param.interval_max = BLE_ADV_INTERVAL_MAX;

	err = bt_le_adv_start(BT_LE_ADV_CONN_NAME, ad, ARRAY_SIZE(ad), NULL, 0);
	if (err) {
		LOG_ERR("Advertising failed to start (err %d)", err);
	} else {
		LOG_INF("Advertising successfully started");
	}

	while (1) {
		k_sleep(K_FOREVER);		// is this neccessary?
	}
}


extern void temp_thread(void)
{

	int status;

	#ifdef BOARD_TTPMS_EXM_2_0
		// turn on MLX power MOSFET and wait short time
	#endif

	//k_sleep(K_MSEC(80));	// wait 80ms after MLX90640 POR
	//k_sleep(K_MSEC(500));	// wait one refresh rate? default refresh rate?
	k_sleep(K_MSEC(1000));	//datasheet is weird so just wait 1 second to be safe

	uint16_t eeMLX90640[MLX90640_EEPROM_DUMP_NUM];

	status = MLX90640_DumpEE(MLX_ADDR, eeMLX90640);
	if (status != 0) {
		LOG_ERR("Failed to load system parameters, MLX90640_DumpEE() returned %d", status);
	} else {
		LOG_INF("Dumped MLX EE");
	}

	status = MLX90640_ExtractParameters(eeMLX90640, &mlx90640);
	if (status != 0) {
		LOG_ERR("Parameter extraction failed, MLX90640_ExtractParameters() returned %d", status);
	} else {
		LOG_INF("Extracted MLX parameters");
	}

	status = MLX90640_SetRefreshRate(MLX_ADDR, temp_freq);
	if (status != 0) {
		LOG_ERR("Setting refresh rate failed, MLX90640_SetRefreshRate() returned %d", status);
	} else {
		LOG_INF("Set MLX refresh rate");
	}

	status = MLX90640_SynchFrame(MLX_ADDR);
	if (status != 0) {
		LOG_ERR("Synchronizing MLX frame failed, MLX90640_SynchFrame() returned %d", status);
	} else {
		LOG_INF("Synchronized MLX frame");
	}


	while (connection_status)
	{

		uint16_t frame[834];

		//k_sleep(K_MSEC(10));

		status = MLX90640_GetFrameData(MLX_ADDR, frame);	// this function waits until data is available
		if (status < 0) {
			LOG_ERR("Getting MLX frame data failed, MLX90640_GetFrameData() returned %d", status);
		}

		Vdd = MLX90640_GetVdd(frame, &mlx90640);

		Ta = MLX90640_GetTa(frame, &mlx90640);

		float tr = Ta - ta_shift; //Reflected temperature based on the sensor ambient temperature

		//LOG_INF("started temp processing");
		MLX90640_CalculateTo(frame, &mlx90640, emissivity, tr, mlx90640To);
		//LOG_INF("finished temp processing");

		LOG_INF("Pixel 400: %f Pixel 401: %f", mlx90640To[399], mlx90640To[400]);

		// BLE indicate
	}


	#ifdef BOARD_TTPMS_EXM_2_0
		// turn off MLX power MOSFET
	#endif
}