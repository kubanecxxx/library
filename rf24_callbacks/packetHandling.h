/**
 * @file packetHandling.h
 * @author kubanec
 * @date 4. 5. 2014
 *
 */
#ifndef PACKETHANDLING_H_
#define PACKETHANDLING_H_

#include "ch.h"
#include "table_conf.h"
#include "scheduler.hpp"

typedef void (*clearWatchdog_t)(void);
typedef void (*transmitting_state_t)(bool);

class RF24_app;
class packetHandling
{
public:
	typedef void (*callback_t)(packetHandling * h, nrf_commands_t command,
			void * load, uint8_t size, void * userData);



	typedef struct
	{
		nrf_commands_t command;
		callback_t cb;
		void * userData;
	} callback_table_t;

	typedef struct
	{
		clearWatchdog_t clearwdt;
		clearWatchdog_t resetMcu;
		transmitting_state_t transmitting_state;
	} function_table_t;

	packetHandling(RF24_app * app, const callback_table_t * t);
	void HandlePacketLoop();

	bool WriteData(nrf_commands_t command, void * load =
	NULL, uint8_t size = 0);

	bool WriteData(nrf_commands_t command, uint32_t data, uint8_t bytes);

	bool RequestData(nrf_commands_t command = IDLE);
	void StartAutoIdle(uint16_t ms = 2000);
	inline void setFunctionTable(const function_table_t * t)
	{
		ft = t;
	}

private:
	static void idle(arg_t arg);

	RF24_app * ap;
	Scheduler auto_idle;
	const callback_table_t * table;
	const function_table_t * ft;
	systime_t last;

};

#endif /* PACKETHANDLING_H_ */
