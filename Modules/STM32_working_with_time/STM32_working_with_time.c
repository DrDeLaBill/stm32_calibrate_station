/*
 *	Автор:						FERTVIST Overset-ankl
 *	Дата последнего изменения:	22.06.2024
 */

#include "STM32_working_with_time.h"



/*
	#define __TIME_RESET(time_variable)		(time_variable) = HAL_GetTick()	// обнулить временную переменную
*/



// статическая задержка с началом отсчитывания времени
void time_static_start_delay(uint32_t time) {
	uint32_t time_variable;
	__TIME_RESET(time_variable);

	while ((uint32_t)(HAL_GetTick() - time_variable) < time);
}


// статическая задержка
void time_static_delay(uint32_t *time_variable, uint32_t time) {
	while ((uint32_t)(HAL_GetTick() - *time_variable) < time);
}

// динамическая задержка
uint8_t time_dynamic_delay(uint32_t *time_variable, uint32_t time) {
	return (uint32_t)(HAL_GetTick() - *time_variable) > time;
}


// статическая задержка с периодическим срабатыванием
void time_static_periodic_delay(uint32_t *time_variable, uint32_t time) {
	while ((uint32_t)(HAL_GetTick() - *time_variable) < time);

	__TIME_RESET(*time_variable);
}

// динамическая задержка с периодическим срабатыванием
uint8_t time_dynamic_periodic_delay(uint32_t *time_variable, uint32_t time) {
	if ((uint32_t)(HAL_GetTick() - *time_variable) > time) {
		__TIME_RESET(*time_variable);
		return 1;
	}
	else return 0;
}
