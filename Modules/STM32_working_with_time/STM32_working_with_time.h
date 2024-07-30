/*
 *	Автор:						FERTVIST Overset-ankl
 *	Дата последнего изменения:	22.06.2024
 */

#ifndef	_STM32_WORKING_WITH_TIME_H
#define _STM32_WORKING_WITH_TIME_H

#include "main.h"



#define __TIME_RESET(time_variable)		(time_variable) = HAL_GetTick()	// обнулить временную переменную



// статическая задержка с началом отсчитывания времени
void time_static_start_delay(uint32_t time);


// статическая задержка
void time_static_delay(uint32_t *time_variable, uint32_t time);

// динамическая задержка
uint8_t time_dynamic_delay(uint32_t *time_variable, uint32_t time);


// статическая задержка с периодическим срабатыванием
void time_static_periodic_delay(uint32_t *time_variable, uint32_t time);

// динамическая задержка с периодическим срабатыванием
uint8_t time_dynamic_periodic_delay(uint32_t *time_variable, uint32_t time);



#endif	// _STM32_WORKING_WITH_TIME_H
