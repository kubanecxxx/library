/*
 * sequencer.c
 *
 *  Created on: 17.9.2013
 *      Author: kubanec
 */

#include "sequencer.h"
#include "scheduler.h"
#include "string.h"

/**
 * @ingroup schedulerGroup
 * @defgroup sequencer
 * @brief na pozadí běží sekvencer pomocí \ref delay_t
 * @details v kterym se volá callback pro uživatele v ruzně dlouhejch časech a
 * s ruznou hodnotou argumentu záleži jak si user nastaví \ref seqItem_t
 * dycky se nastaví jaký argument + za jak dlouho se má zavolat callback od předchozího callbacku
 * nebo od začátku \ref seqPlay pokud je první
 * @{
 */

/**
 * @brief interní callback po dokončení delaye naplánuje další spuštění a hodí cb pro usera
 */
static void seqCb(arg_t arg)
{
    sequencer_t * seq = (sequencer_t*)arg;
    seqItem_t item;
    memcpy(&item,seq->items+seq->actualState,sizeof(seqItem_t));

    seq->cb(item.arg);
    seq->actualState++;
    memcpy(&item,seq->items+seq->actualState,sizeof(seqItem_t));
    if(item.cycles)
    {
        shFillStruct(seq->del,seqCb,seq,item.cycles,ONCE);
        shRegisterStruct(seq->del);
    }
    else
    {
    	if (seq->endCb)
    		seq->endCb(NULL);
    }
}

/**
 * @brief rozjede na pozadí sequencer
 * @param [in] sequencer objekt sekvenceru
 * @param [in] delay objekt delaye které bude sekvencer používat pro sebe
 * @param [in] items pole itemů ktery kde je napsána za jak dlouho a s jakym argumentem se má volat cb
 * @param [in] cb callback pro uživatele bude se volat podle věcí z items v nějakym čase a s argumentem
 * @param [in] end callback pro uživatele že sekvencer skončil (pokud NULL nevolá se nic)
 */
void seqPlay(sequencer_t *sequencer, delay_t *delay, const seqItem_t *items, method_t cb, method_t end)
{
    sequencer->actualState = 0;
    sequencer->cb = cb;
    sequencer->del = delay;
    sequencer->items = items;
    sequencer->endCb = end;

    shFillStruct(delay,seqCb,sequencer,items->cycles,ONCE);
    shRegisterStruct(delay);
}

/**
 * @}
 */
