/*
 * ringBuffer.h
 *
 *  Created on: 11.5.2013
 *      Author: kubanec
 */

#ifndef RINGBUFFER_H_
#define RINGBUFFER_H_

/**
 * @ingroup schedulerGroup
 * @defgroup ringBuffer
 * @brief jednoduchý kruhový buffer pro objekty jakékoliv velikosti
 * @{
 */

/**
 * @brief objekt kruhovyho bufferu
 */
typedef struct buffer_t buffer_t;
/**
 * @brief objekt kruhovyho bufferu
 */
struct buffer_t
{
	/// velikost jedné položky v bytech
	uint8_t dataSize;
	/// pointer předku
	uint8_t head;
	/// pointer zadku
	uint8_t tail;
	/// velikost bufferu
	uint8_t maximum;
	/// pointer na data kam má ukládat
	void * data;
};

/**
 * @brief jesli má data vykopírovat úplně nebo jenom vyhodit pointer na ně
 */
typedef enum
{
	/// hard copy dat
	COPY,
	/// jenom pointer
	NO_COPY
} buffer_cpy_t;

//api
uint8_t bufPut(buffer_t * handle, const void * data);
uint8_t bufGet(buffer_t * handle, void ** retData, buffer_cpy_t cpy);
uint8_t bufIsFull(const buffer_t * handle);
uint8_t bufIsEmpty(const buffer_t * handle);

///deklarace novyho objektu bufferu i se statickou inicializací
#define buf_DECLARE(name, dataPtr,dataSze,count) \
	buffer_t name = {.data = dataPtr, .dataSize = dataSze, .head = 0, .tail = 0, .maximum = count-1}

/**
 * @}
 */

#endif /* RINGBUFFER_H_ */
