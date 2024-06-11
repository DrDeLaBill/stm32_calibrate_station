/* Copyright © 2024 Georgy E. All rights reserved. */

#include "ginterface.h"

#include "settings.h"


size_t ginterface::dv_type::key = 0;
uint16_t ginterface::dv_type::get(const uint8_t)
{
	return settings.dv_type;
}
void ginterface::dv_type::set(const uint16_t value, const uint8_t)
{
	settings.dv_type = value;
}


size_t ginterface::sw_id::key = 0;
uint8_t ginterface::sw_id::get(const uint8_t)
{
	return settings.sw_id;
}
void ginterface::sw_id::set(const uint8_t value, const uint8_t)
{
	settings.sw_id = value;
}


size_t ginterface::fw_id::key = 0;
uint8_t ginterface::fw_id::get(const uint8_t)
{
	return settings.fw_id;
}
void ginterface::fw_id::set(const uint8_t value, const uint8_t)
{
	settings.fw_id = value;
}


size_t ginterface::target_ml::key = 0;
uint32_t ginterface::target_ml::get(const uint8_t)
{
	return 0;
}
void ginterface::target_ml::set(const uint32_t value, const uint8_t)
{

}


size_t ginterface::level_ml::key = 0;
uint32_t ginterface::level_ml::get(const uint8_t)
{
	return 0;
}
void ginterface::level_ml::set(const uint32_t value, const uint8_t)
{

}


size_t ginterface::work_ml::key = 0;
uint32_t ginterface::work_ml::get(const uint8_t)
{
	return 0;
}
void ginterface::work_ml::set(const uint32_t value, const uint8_t)
{

}
