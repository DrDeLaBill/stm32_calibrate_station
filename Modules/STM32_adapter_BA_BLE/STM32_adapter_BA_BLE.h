/*
 *	Автор:						FERTVIST Overset-ankl
 *	Дата последнего изменения:
 */

#ifndef	_INCLUDE_STM32_ADAPTER_BA_BLE_H
#define _INCLUDE_STM32_ADAPTER_BA_BLE_H

#include "main.h"


#define ADAPTER_BA_BLE_SIZE_ERROR		5
typedef uint_least8_t _Adapter_BA_BLE_SizeError;


typedef enum _Adapter_BA_BLE_Mode {
	SINGLE_DATA_READ										= 0,
	PERIODIC_DATA_OUTPUT,
	CHANGING_THE_PERIODIC_ISSUE_INTERVAL,
	DEFAULT_DATA_OUTPUT_MODE,
	SETTING_THE_FILTRATION_DEPTH,
	READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS,
	READING_THE_CURRENT_SENSOR_SETTINGS,

	ALL_MODE

} Adapter_BA_BLE_Mode;


typedef enum _Adapter_BA_BLE_SizeMessage {
	SINGLE_DATA_READ_TX												= 4,
	SINGLE_DATA_READ_RX												= 9,

	PERIODIC_DATA_OUTPUT_TX											= 4,
	PERIODIC_DATA_OUTPUT_RX											= 5,
	PERIODIC_DATA_OUTPUT_RX_DATA									= 9,

	CHANGING_THE_PERIODIC_ISSUE_INTERVAL_TX							= 5,
	CHANGING_THE_PERIODIC_ISSUE_INTERVAL_RX							= 5,

	DEFAULT_DATA_OUTPUT_MODE_TX										= 5,
	DEFAULT_DATA_OUTPUT_MODE_RX										= 5,

	SETTING_THE_FILTRATION_DEPTH_TX									= 5,
	SETTING_THE_FILTRATION_DEPTH_RX									= 5,

	READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS_TX			= 4,
	READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS_RX_LOG		= 15,
	READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS_RX_ERROR		= 5,

	READING_THE_CURRENT_SENSOR_SETTINGS_TX							= 4,
	READING_THE_CURRENT_SENSOR_SETTINGS_RX							= 44,

} Adapter_BA_BLE_SizeMessage;

#define ADAPTER_BA_BLE_SIZE_BUFFER_TX	SINGLE_DATA_READ_TX
#define ADAPTER_BA_BLE_SIZE_BUFFER_RX	SINGLE_DATA_READ_RX
typedef uint_least8_t Adapter_BA_BLE_SizeBuffer;


typedef enum _Adapter_BA_BLE_Status {
	ADAPTER_BA_BLE_OK					= 0,
	ADAPTER_BA_BLE_ERROR,
	ADAPTER_BA_BLE_EXPECTATION,

	ADAPTER_BA_BLE_INITIALIZATION,

} Adapter_BA_BLE_Status;




typedef int8_t DataSensorTemperature;
typedef uint16_t DataSensorLevel;
typedef uint16_t DataSensorFrequency;

typedef struct _Adapter_BA_BLE_DataSensor {
	DataSensorLevel level;
	DataSensorFrequency frequency;
	DataSensorTemperature temperature;

} Adapter_BA_BLE_DataSensor;




typedef struct _Adapter_BA_BLE {
	union {
		Adapter_BA_BLE_DataSensor data_sensor;
	};
	UART_HandleTypeDef *_huart;

	uint32_t _time;

	Adapter_BA_BLE_Status status;

	Adapter_BA_BLE_Mode _mode;

	_Adapter_BA_BLE_SizeError _count_error;

	uint8_t _buffer_tx[ADAPTER_BA_BLE_SIZE_BUFFER_TX];
	uint8_t _buffer_rx[ADAPTER_BA_BLE_SIZE_BUFFER_RX];

} Adapter_BA_BLE;




typedef uint8_t NetworkAddress;



void adapter_BA_BLE_initialization(Adapter_BA_BLE *adapter_ba_ble, UART_HandleTypeDef *huart);


void adapter_BA_BLE_while(Adapter_BA_BLE *adapter_ba_ble);

void adapter_BA_BLE_rx_cplt_callback(Adapter_BA_BLE *adapter_ba_ble);


void adapter_BA_BLE_single_data_read(Adapter_BA_BLE *adapter_ba_ble, NetworkAddress network_adress);




#endif	// _INCLUDE_STM32_ADAPTER_BA_BLE_H
