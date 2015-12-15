/*
 * scheduler.c
 *
 *  Created on: 11.4.2013
 *      Author: kubanec
 */

#include "scheduler.h"
#include "string.h"
#ifdef QT_CORE_LIB
#include "assert.h"
#define chDbgAssert(x,y,z) assert(x)
#else
#include "ch.h"
#endif
#ifndef MS2ST
#define chDbgAssert(x,y,z)
#endif

/**
 * @defgroup schedulerGroup
 * @brief sada pro jednoduchy software based plánování na pozadí kde není potřeba
 * časová přesnost
 */

/**
 * @ingroup schedulerGroup
 * @defgroup scheduler
 * @brief jednoduchej plánovač kterej postupně volá registrovany callbacky v nastavenym
 * časovym intervalu periodicky nebo one shot
 * @{
 */

extern systime_t sysTime;
static void shRunMethod(delay_t * method);

/// první delay objekt ve frontě
static delay_t * first = NULL;
/// poslední delay objekt ve frontě
static delay_t * last = NULL;

/**
 * @brief vrátí hodnotu systémovyho času
 */
systime_t shNow(void)
{
	return sysTime;
}

/**
 * @brief nutno umístit do hlavní smyčky kde se bude pořád volat
 * funkce potom volá příslušné callbacky ve správnym čase
 */
void shPlay(void)
{
	//projet celej seznam
	//zavolat shRunMethod
	delay_t * del = first;
	while (del)
	{
		shRunMethod(del);
		del = del->next;
	}
}

/**
 * @brief inicializuje objekt \ref delay_t <br/>
 * <b>je potřeba ho potom ještě registrovat </b>
 * pomocí \ref shRegisterStruct
 * @param [in] del obkjekt delaye
 * @param [in] method callback kterej se bude volat po uplynuti času
 * @param [in] arg hodnota argumentu která přinde do callbacku
 * @param [in] cycles počet ticků systémového času než se má zavolat callback
 * @param [in] type jesli se má callback volat periodicky nebo jenom jednou
 */
void shFillStruct(delay_t * del, method_t method, arg_t arg, systime_t cycles,
		sh_t type)
{
	del->method = method;
	del->arg = arg;
	del->type = type;
	del->next = NULL;
	del->period = cycles;
	del->last = 0;
}

/**
 * @brief zařadí objekt delay do fronty procházení
 * objektů a začne se volat jeho callback
 * @param [in] del nastavený objekt delaye
 */
void shRegisterStruct(delay_t * del)
{
	if (shIsRegistered(del))
		return;

	if (first == NULL)
		first = del;
	if (last)
		last->next = del;
	last = del;
	del->last = sysTime;
	del->next = NULL;
}

/**
 * @brief funkce vrátí jesli je objekt registrován ve frontě
 * @return
 *  + pokud je registrován vypadne 1
 *  + pokud není registrován vypande 0
 */
uint8_t shIsRegistered(const delay_t * del)
{
    if (del == first)
    {
        return 1;
    }

    delay_t * temp = first;
    while (temp)
    {
        if (temp->next == del)
        {
            return 1;
        }
        temp = temp->next;
    }

    return 0;
}

/**
 * @brief odregistruje objekt z fronty a přestane se volat callback
 */
void shUnregisterStruct(delay_t * del)
{
	if (!shIsRegistered(del))
		return;

	delay_t * temp = first;

	if (del == first)
	{
		first = del->next;
		return;
	}

	while (temp)
	{
		if (temp->next == del)
		{
			temp->next = del->next;
			if (del == last)
				last = temp;

			return;
		}
		temp = temp->next;
	}
}

/**
 * @brief spustí callback i s jeho argumentem
 */
void shRunMethod(delay_t * method)
{
    chDbgAssert(method->method,"method is NULL");
	if (sysTime - method->last > method->period)
	{
		if (method->type == ONCE)
					shUnregisterStruct(method);

		method->last = sysTime;
		method->method(method->arg);
	}
}

/**
 * @brief přemaže už uplynutý čas objektu \ref delay_t
 * a začne počítat od znova
 */
void shRearm(delay_t * del)
{
	del->last = sysTime;
}

/**
 * @}
 */
