/*
 * sequencer.h
 *
 *  Created on: 17.9.2013
 *      Author: kubanec
 */

#ifndef SEQUENCER_H_
#define SEQUENCER_H_

#include "scheduler.h"

/**
 * @ingroup sequencer
 * @{
 */

/**
 * @brief objekt sekvenceru
 */
typedef struct sequencer_t sequencer_t;

/**
 * @brief jedna položka seznamu itemů pro sekvencer
 * @details pro označení poslední položky v polu je potřeba dat oba
 * dva (arg, cycles) na nulu
 */
typedef struct
{
	///argument se kterym se má volat callback
    arg_t arg;
    /// doba za jak dlouho se má volat od předchoziho callbacku nebo když je první tak od začátku
    uint16_t cycles;
} seqItem_t;

/**
 * @brief objekt sekvenceru
 */
struct sequencer_t
{
	/// interní počítadlo pro sekvencer aby věděl v jaké je fázi
    uint8_t actualState;
    /// user callback se postupně volá podle pole items
    method_t cb;
    /// interní delay pro volání callbacků na pozadí
    delay_t * del;
    /// seznam kroků sekvenceru
    const seqItem_t * items;
    /// callback se volá až sekvencer skončí
    method_t endCb;
};

void seqPlay(sequencer_t * sequencer, delay_t * delay, const seqItem_t * items, method_t cb, method_t endCb);

/**
 * @}
 */

#endif /* SEQUENCER_H_ */
