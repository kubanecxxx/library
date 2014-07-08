/**
 * @file packetHandling.cpp
 * @author kubanec
 * @date 4. 5. 2014
 *
 */
#include "ch.h"
#include "hal.h"

#include "packetHandling.h"
#include "table_conf.h"
#include "RF24.h"
#include "RF24app.h"
#include "platform.h"

packetHandling::packetHandling(RF24_app * app, const callback_table_t * cfg) :
		ap(app), table(cfg)
{
	chDbgAssert(app, "driver for nrf is NULL", "");
	chDbgAssert(cfg, "config is NULL", "");

	auto_idle.Setup(idle, this, MS2ST(2000), PERIODIC);
	wdt = NULL;
	last = chTimeNow();
}

void packetHandling::HandlePacketLoop()
{
	if (!ap->packet_available())
		return;

	uint16_t command;
	uint8_t load[32];
	uint8_t size = ap->read_packet(command, load);

	const callback_table_t * t = table;

	while (t->cb)
	{
		if ((command & ~WRITE_FLAG) == t->command)
		{
			t->cb(this, static_cast<nrf_commands_t>(command), load, size,
					t->userData);
			return;
		}

		t++;
	}
}

void packetHandling::StartAutoIdle(uint16_t ms)
{
	auto_idle.SetPeriodMilliseconds(ms);
	auto_idle.Register();

	if (ms == 0)
		auto_idle.Unregister();
}

bool packetHandling::RequestData(nrf_commands_t command)
{
	return ap->send_packet(command);
}

bool packetHandling::WriteData(nrf_commands_t command, void * load,
		uint8_t size)
{
	return ap->send_packet(command | WRITE_FLAG, load, size);
}

bool packetHandling::WriteData(nrf_commands_t command, uint32_t data,
		uint8_t len)
{
	chDbgAssert(len <= 4, "bad int length", "");
	chDbgAssert(len != 0, "int length 0", "");

	uint8_t buf[len];
	for (int i = 0; i < len; i++)
	{
		buf[i] = data & 0xff;
		data >>= 8;
	}

	return ap->send_packet(command | WRITE_FLAG, buf, len);
}

void packetHandling::idle(arg_t arg)
{
	packetHandling * p = (packetHandling*) arg;
	bool ok = p->RequestData();
	palWritePad(TEST_LED_PORT3, TEST_LED_PIN3, ok);

	if (p->wdt)
	{
		if (ok)
		{
			((p->wdt))();
			p->last = chTimeNow();
		}
		else
		{
			if (!(chTimeNow() - p->last > S2ST(300)))
				p->wdt();
		}
	}
}
