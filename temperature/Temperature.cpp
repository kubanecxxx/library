/*
 * Temperature.cpp
 *
 *  Created on: 11.9.2012
 *      Author: kubanec
 */

#include "ch.h"
#include "hal.h"
#include "scheduler.hpp"
#include "Temperature.h"

static const I2CConfig i2conf =
{ OPMODE_I2C, 10000, STD_DUTY_CYCLE };

//int16_t Temperature::temperature = 0;

void Temperature::Init(I2CDriver * i2c, uint8_t address)
{
	slave_address = address;
	i2cd = i2c;
	uint8_t txbuf[3];
	i2cStart(i2cd, &i2conf);

	//sleep mode
	txbuf[0] = 1;
	txbuf[1] = 1;
	i2cMasterTransmit(i2cd, slave_address, txbuf, 2, NULL, 0);
	i2cStop(i2cd);

	temperature = 0;

	s.Setup(machine, this, MS2ST(200), ONCE);
}

int16_t Temperature::GetTemperature()
{
	return temperature;
}

void Temperature::machine(arg_t arg)
{
	Temperature * o = (Temperature*) arg;
	uint8_t txbuf[2];
	uint8_t ja[2];

	txbuf[0] = 0;
	i2cStart(o->i2cd, &i2conf);
	i2cMasterTransmit(o->i2cd, o->slave_address, txbuf, 1, ja, 2);

	//sleep mode
	txbuf[0] = 1;
	txbuf[1] = 1;

	i2cMasterTransmit(o->i2cd, o->slave_address, txbuf, 2, NULL, 0);
	i2cStop(o->i2cd);

	int16_t t = (ja[1] & 0x80) | (ja[0] << 8);
	t >>= 7;
	o->temperature = t;
}

void Temperature::RefreshTemperature()
{
	uint8_t txbuf[2];

	//wake up
	txbuf[0] = 1;
	txbuf[1] = 0;
	i2cStart(i2cd, &i2conf);
	i2cMasterTransmit(i2cd, slave_address, txbuf, 2, NULL, 0);
	i2cStop(i2cd);

	s.SetPeriodMilliseconds(200);
	s.Register();

}
