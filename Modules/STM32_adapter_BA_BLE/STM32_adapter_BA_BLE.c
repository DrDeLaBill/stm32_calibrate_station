/*
 *	Автор:						FERTVIST Overset-ankl
 *	Дата последнего изменения:
 */

#include "STM32_adapter_BA_BLE.h"

#include "STM32_working_with_time.h"



#define ADAPTER_BA_BLE_BAUD_RATE	19200

#define ADAPTER_BA_BLE_TIME_EXPECTATION		120
#if ADAPTER_BA_BLE_TIME_EXPECTATION < 101
#error
#endif



typedef enum _Adapter_BA_BLE_NumberByte {
	PREFIX				= 0,
	NETWORK_ADDRESS		= 1,
	OPERATION_CODE		= 2,
	DATA				= 3,

} Adapter_BA_BLE_NumberByte;

typedef enum _Adapter_BA_BLE_CodePrefix {
	INCOMING	= 0x31,
	OUTGOING	= 0x3E,

} Adapter_BA_BLE_CodePrefix;

typedef enum _Adapter_BA_BLE_OperationCode {
	SINGLE_DATA_READ_CODE										= 0x06,
	PERIODIC_DATA_OUTPUT_CODE									= 0x07,
	CHANGING_THE_PERIODIC_ISSUE_INTERVAL_CODE					= 0x13,
	DEFAULT_DATA_OUTPUT_MODE_CODE								= 0x17,
	SETTING_THE_FILTRATION_DEPTH_CODE							= 0x0E,
	READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS_CODE		= 0x0F,
	READING_THE_CURRENT_SENSOR_SETTINGS_CODE					= 0x10,

} Adapter_BA_BLE_OperationCode;


typedef uint8_t Adapter_BA_BLE_CRC;



Adapter_BA_BLE_SizeMessage adapter_BA_BLE_size_tx[ALL_MODE] = {
		SINGLE_DATA_READ_TX, PERIODIC_DATA_OUTPUT_TX, CHANGING_THE_PERIODIC_ISSUE_INTERVAL_TX, DEFAULT_DATA_OUTPUT_MODE_TX, SETTING_THE_FILTRATION_DEPTH_TX,
		READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS_TX, READING_THE_CURRENT_SENSOR_SETTINGS_TX
};

Adapter_BA_BLE_SizeMessage adapter_BA_BLE_size_rx[ALL_MODE] = {
		SINGLE_DATA_READ_RX
};





void adapter_BA_BLE_initialization(Adapter_BA_BLE *adapter_ba_ble, UART_HandleTypeDef *huart);


static Adapter_BA_BLE_CRC _adapter_BA_BLE_CRC(uint8_t array[], Adapter_BA_BLE_SizeBuffer number_byte_CRC);

static void _adapter_BA_BLE_transmit(Adapter_BA_BLE *adapter_ba_ble, Adapter_BA_BLE_SizeBuffer size_rx);

static void _adapter_BA_BLE_error_handler(Adapter_BA_BLE *adapter_ba_ble);


void adapter_BA_BLE_while(Adapter_BA_BLE *adapter_ba_ble);

void adapter_BA_BLE_rx_cplt_callback(Adapter_BA_BLE *adapter_ba_ble);


void adapter_BA_BLE_single_data_read(Adapter_BA_BLE *adapter_ba_ble, NetworkAddress network_adress);




void adapter_BA_BLE_initialization(Adapter_BA_BLE *adapter_ba_ble, UART_HandleTypeDef *huart) {
	huart->Instance->BRR = UART_BRR_SAMPLING8(HAL_RCC_GetPCLK2Freq(), ADAPTER_BA_BLE_BAUD_RATE);
	huart->Instance->BRR = huart->Instance->BRR / 2 + huart->Instance->BRR % 2;


	adapter_ba_ble->_buffer_tx[PREFIX] = INCOMING;

	adapter_ba_ble->_huart = huart;
	adapter_ba_ble->status = ADAPTER_BA_BLE_INITIALIZATION;

	adapter_ba_ble->_count_error = 0;
}

Adapter_BA_BLE_CRC _adapter_BA_BLE_CRC(uint8_t array[], Adapter_BA_BLE_SizeBuffer number_byte_CRC) {
	Adapter_BA_BLE_CRC crc = 0x00;

	uint8_t variable;

	for (Adapter_BA_BLE_SizeBuffer i = 0; i < number_byte_CRC; ++i) {
		variable = array[i] ^ (uint8_t)crc;

		crc = 0;
		if(variable & (0x01 << 0)) crc ^= 0x5e;
		if(variable & (0x01 << 1)) crc ^= 0xbc;
		if(variable & (0x01 << 2)) crc ^= 0x61;
		if(variable & (0x01 << 3)) crc ^= 0xc2;
		if(variable & (0x01 << 4)) crc ^= 0x9d;
		if(variable & (0x01 << 5)) crc ^= 0x23;
		if(variable & (0x01 << 6)) crc ^= 0x46;
		if(variable & (0x01 << 7)) crc ^= 0x8c;
	};

	return crc;
}

void _adapter_BA_BLE_transmit(Adapter_BA_BLE *adapter_ba_ble, Adapter_BA_BLE_SizeBuffer size_rx) {
	adapter_ba_ble->_buffer_rx[PREFIX] = !OUTGOING;
	HAL_UART_Transmit_IT(adapter_ba_ble->_huart, adapter_ba_ble->_buffer_tx, adapter_BA_BLE_size_tx[adapter_ba_ble->_mode]);
	HAL_UART_Receive_IT(adapter_ba_ble->_huart, adapter_ba_ble->_buffer_rx, size_rx);
	__TIME_RESET(adapter_ba_ble->_time);
}

void _adapter_BA_BLE_error_handler(Adapter_BA_BLE *adapter_ba_ble) {
	if (adapter_ba_ble->_count_error >= ADAPTER_BA_BLE_SIZE_ERROR) {
		adapter_ba_ble->status = ADAPTER_BA_BLE_ERROR;
	} else {
		++adapter_ba_ble->_count_error;
	};
}


void adapter_BA_BLE_while(Adapter_BA_BLE *adapter_ba_ble) {
	if ((adapter_ba_ble->status == ADAPTER_BA_BLE_EXPECTATION || adapter_ba_ble->status == ADAPTER_BA_BLE_ERROR)&& time_dynamic_delay(&adapter_ba_ble->_time, ADAPTER_BA_BLE_TIME_EXPECTATION)) {
		_adapter_BA_BLE_error_handler(adapter_ba_ble);
		_adapter_BA_BLE_transmit(adapter_ba_ble, adapter_BA_BLE_size_rx[adapter_ba_ble->_mode]);
	};
}

void adapter_BA_BLE_rx_cplt_callback(Adapter_BA_BLE *adapter_ba_ble) {
	if (adapter_ba_ble->_buffer_rx[PREFIX] != OUTGOING ||
		adapter_ba_ble->_buffer_rx[NETWORK_ADDRESS] != adapter_ba_ble->_buffer_tx[NETWORK_ADDRESS] ||
		adapter_ba_ble->_buffer_rx[OPERATION_CODE] != adapter_ba_ble->_buffer_tx[OPERATION_CODE] ||
		adapter_ba_ble->_buffer_rx[adapter_BA_BLE_size_rx[adapter_ba_ble->_mode] - 1] != _adapter_BA_BLE_CRC(adapter_ba_ble->_buffer_rx, adapter_BA_BLE_size_rx[adapter_ba_ble->_mode] - 1)) {

		_adapter_BA_BLE_error_handler(adapter_ba_ble);
		_adapter_BA_BLE_transmit(adapter_ba_ble, adapter_BA_BLE_size_rx[adapter_ba_ble->_mode]);
		return;
	}


	adapter_ba_ble->_count_error = 0;
	switch(adapter_ba_ble->_mode) {
		case SINGLE_DATA_READ:
			adapter_ba_ble->data_sensor.temperature = *(DataSensorTemperature*)(adapter_ba_ble->_buffer_rx + DATA);
			adapter_ba_ble->data_sensor.level = *(DataSensorLevel*)(adapter_ba_ble->_buffer_rx + DATA + sizeof(DataSensorTemperature));
			adapter_ba_ble->data_sensor.frequency = *(DataSensorFrequency*)(adapter_ba_ble->_buffer_rx + DATA + sizeof(DataSensorTemperature) + sizeof(DataSensorLevel));
			break;
		case PERIODIC_DATA_OUTPUT:
		case CHANGING_THE_PERIODIC_ISSUE_INTERVAL:
		case DEFAULT_DATA_OUTPUT_MODE:
		case SETTING_THE_FILTRATION_DEPTH:
		case READING_THE_HISTORY_OF_CHANGES_TO_SENSOR_SETTINGS:
		case READING_THE_CURRENT_SENSOR_SETTINGS:
		case ALL_MODE:
		default:
			break;
	};

	adapter_ba_ble->status = ADAPTER_BA_BLE_OK;
}


void adapter_BA_BLE_single_data_read(Adapter_BA_BLE *adapter_ba_ble, NetworkAddress network_adress) {
	adapter_ba_ble->_mode = SINGLE_DATA_READ;
	adapter_ba_ble->status = ADAPTER_BA_BLE_EXPECTATION;

	adapter_ba_ble->_buffer_tx[NETWORK_ADDRESS] = network_adress;
	adapter_ba_ble->_buffer_tx[OPERATION_CODE] = SINGLE_DATA_READ_CODE;

	adapter_ba_ble->_buffer_tx[adapter_BA_BLE_size_tx[SINGLE_DATA_READ] - 1] = _adapter_BA_BLE_CRC(adapter_ba_ble->_buffer_tx, adapter_BA_BLE_size_tx[SINGLE_DATA_READ] - 1);

	_adapter_BA_BLE_transmit(adapter_ba_ble, adapter_BA_BLE_size_rx[SINGLE_DATA_READ]);
}
