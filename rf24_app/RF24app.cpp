/**
 * @file RF24app.cpp
 * @author kubanec
 * @date 3. 4. 2014
 *
 */
#include "ch.h"
#include "hal.h"
#include "RF24.h"
#include <RF24app.h>
#include <string.h>

RF24_app::RF24_app(RF24 * rf24, uint64_t add, uint8_t chan) :
		rf(rf24), address(add), channel(chan)
{
}

void RF24_app::start()
{
	rf->begin();
	rf->enableAckPayload();
	rf->enableDynamicPayloads();
	rf->setChannel(channel);
	rf->openWritingPipe(address);
}

bool RF24_app::send_packet(uint16_t command, const void * load, uint8_t size)
{
	if (size > 30)
		return false;
	uint8_t buffer[32];
	buffer[0] = command & 0xff;
	buffer[1] = (command >> 8) & 0xff;
	memcpy(buffer + 2 , load, size);

	return rf->write(buffer, size + 2);
}

uint8_t RF24_app::read_packet(uint16_t & command, void * load)
{
	uint8_t buffer[32];
	uint8_t size = rf->getDynamicPayloadSize();

	rf->read(buffer, size);
	command = buffer[0] | (buffer[1] << 8);

	if (load)
		memcpy(load, buffer + 2, size - 2);

	return size - 2;
}

