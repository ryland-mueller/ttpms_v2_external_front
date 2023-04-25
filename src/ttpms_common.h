#ifndef _TTPMS_COMMON_
#define _TTPMS_COMMON_

// BLE addresses/identities
// First hex char must be C for random static address (to self-assign identity)

#define TTPMS_RX_BT_ID  "CA:69:F1:F1:69:69"		// Receiver BT ID
#define TTPMS_IFL_BT_ID "CA:69:F1:F1:33:42"		// Internal Front Left BT ID
#define TTPMS_IFR_BT_ID "CA:69:F1:F1:33:43"		// Internal Front Right BT ID
#define TTPMS_IRL_BT_ID "CA:69:F1:F1:33:44"		// Internal Rear Left BT ID
#define TTPMS_IRR_BT_ID "CA:69:F1:F1:33:45"		// Internal Rear Right BT ID
#define TTPMS_EFL_BT_ID "CA:69:F1:F1:11:22"		// External Front Left BT ID
#define TTPMS_EFR_BT_ID "CA:69:F1:F1:11:23"		// External Front Right BT ID
#define TTPMS_ERL_BT_ID "CA:69:F1:F1:11:24"		// External Rear Left BT ID
#define TTPMS_ERR_BT_ID "CA:69:F1:F1:11:25"		// External Rear Right BT ID


// BLE UUIDs
// In theory it should be ok that each sensor shares the same GATT service UUIDs,
// since the receiver distinguishes between them based on BT address/identity.

#define TTPMS_BASE_UUID_PART_1	0x3022c87e
#define TTPMS_BASE_UUID_PART_2	0x71b0
#define TTPMS_BASE_UUID_PART_3	0x4d57
#define TTPMS_BASE_UUID_PART_4	0x881f
#define TTPMS_BASE_UUID_PART_5	0x54996ce814ad


// Base UUID, randomly generated, used for the overall GATT service
#define TTPMS_SERVICE_BASE_UUID BT_UUID_128_ENCODE(	TTPMS_BASE_UUID_PART_1, \
													TTPMS_BASE_UUID_PART_2, \
													TTPMS_BASE_UUID_PART_3, \
													TTPMS_BASE_UUID_PART_4, \
													TTPMS_BASE_UUID_PART_5		)

// Base UUID + 1, used for the battery life GATT characteristic
#define TTPMS_SERVICE_BATT_UUID BT_UUID_128_ENCODE(	TTPMS_BASE_UUID_PART_1, \
													TTPMS_BASE_UUID_PART_2, \
													TTPMS_BASE_UUID_PART_3, \
													TTPMS_BASE_UUID_PART_4, \
													TTPMS_BASE_UUID_PART_5 + 1	)

// Base UUID + 2, used for the temperature GATT characteristic
#define TTPMS_SERVICE_TEMP_UUID BT_UUID_128_ENCODE(	TTPMS_BASE_UUID_PART_1, \
													TTPMS_BASE_UUID_PART_2, \
													TTPMS_BASE_UUID_PART_3, \
													TTPMS_BASE_UUID_PART_4, \
													TTPMS_BASE_UUID_PART_5 + 2	)

// Base UUID + 3, used for the pressure GATT characteristic
#define TTPMS_SERVICE_PRES_UUID BT_UUID_128_ENCODE(	TTPMS_BASE_UUID_PART_1, \
													TTPMS_BASE_UUID_PART_2, \
													TTPMS_BASE_UUID_PART_3, \
													TTPMS_BASE_UUID_PART_4, \
													TTPMS_BASE_UUID_PART_5 + 3	)

#endif