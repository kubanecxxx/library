/*
 * ringBuffer.c
 *
 *  Created on: 11.5.2013
 *      Author: kubanec
 */

//#include "porting.h"
#include "ch.h"
#include "ringBuffer.h"
#include <string.h>

/**
 * @brief založí data do bufferu - natvrdo nakopíruje
 * @return 1 když je buffer plné
 * 	0 když se to povede tam narvat
 */
uint8_t bufPut(buffer_t * handle, const void * data)
{
    uint8_t ret = 0;
	if (bufIsFull(handle))
    {
        ret = 1;
        //zahodit posledni data
        if (handle->tail++ == handle->maximum)
            handle->tail = 0;
    }

    if (handle->head++ == handle->maximum)
        handle->head = 0;

    memcpy(handle->data + (handle->head*handle->dataSize), data, handle->dataSize);
    return ret;
}

/**
 * @brief vytáhne data z bufferu
 * @return 1 když je buffer prázdné
 * 	0 když něco vrátí
 */
uint8_t bufGet(buffer_t * handle, void ** retData, buffer_cpy_t cpy)
{
	if (bufIsEmpty(handle))
		return 1;

    if (handle->tail++ == handle->maximum)
        handle->tail = 0;

	void * temp;
    temp = handle->data + (handle->tail * handle->dataSize);



	if (cpy == COPY)
		memcpy(*retData,temp,handle->dataSize);
	else
		*retData = temp;

	return 0;
}

/**
 * @brief zjištuje jestli je v bufferu ještě místo
 */
uint8_t bufIsFull(const buffer_t * handle)
{
	uint8_t head = handle->head;
	uint8_t tail = handle->tail;

	if (tail == 0 && head == handle->maximum)
		return 1;

	if (tail - 1 == head)
		return 1;

	return 0;
}

/**
 * @brief zjistí jestli je buffer prázdné
 */
uint8_t bufIsEmpty(const buffer_t * handle)
{
	return (handle->head == handle->tail);
}
