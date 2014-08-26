/**
 * @file RF24app.h
 * @author kubanec
 * @date 3. 4. 2014
 *
 */
#ifndef RF24APP_H_
#define RF24APP_H_


class RF24;
class RF24_app
{
public:
	RF24_app(RF24 * rf24, uint64_t address, uint8_t channel);
	void start();

	inline bool send_packet(uint16_t command, uint8_t load)
	{
		return send_packet(command, &load, 1);
	}
	inline bool send_packet(uint16_t command, uint16_t load)
	{
		return send_packet(command, &load, 2);
	}
	inline bool send_packet(uint16_t command, uint32_t load)
	{
		return send_packet(command, &load, 4);
	}
	bool send_packet(uint16_t command, const void * load = NULL, uint8_t size = 0);

	inline bool packet_available()
	{
		return rf->isAckPayloadAvailable();
	}
	uint8_t read_packet(uint16_t & command, void * load = NULL);
	inline RF24* getRF() const {return rf;}

private:
	RF24 * rf;
	uint64_t address;
	uint8_t channel;
};

#endif /* RF24APP_H_ */
