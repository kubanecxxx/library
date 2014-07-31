/*
 * Temperature.h
 *
 *  Created on: 11.9.2012
 *      Author: kubanec
 */

#ifndef TEMPERATURE_H_
#define TEMPERATURE_H_

//#define I2C_TEMP_ADDRESS 0b1001111
#include "scheduler.hpp"

typedef struct I2CDriver;

class Temperature
{
private:
	int16_t temperature;
public:
	void Init(I2CDriver * i2c, uint8_t address);
	int16_t GetTemperature(void);
	void RefreshTemperature(void);

private:
	static void machine(arg_t arg);
	I2CDriver * i2cd;
	Scheduler s;
	uint8_t slave_address;
};

#endif /* TEMPERATURE_H_ */
