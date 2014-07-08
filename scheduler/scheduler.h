/*
 * scheduler.h
 *
 *  Created on: 11.4.2013
 *      Author: kubanec
 */

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <inttypes.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @ingroup scheduler
 * @{
 */

/**
 * @brief volat callback periodicky nebo jenom jednou
 */
typedef enum
{
	/// jenom jednou
	ONCE,
	/// periodicky
	PERIODIC
} sh_t;

/// @brief systémový čas
typedef uint32_t systime_t;
/// @brief argument callbacku
typedef void * arg_t;
/// @brief callback
typedef void (*method_t)(arg_t);
/// @brief struktura pro delay
typedef struct delay_t delay_t;

/**
 * @brief struktura jednoho delaye
 */
struct delay_t
{
	/// callback co se bude volat
	method_t method;
	/// argument kterej dostane
	arg_t arg;
	/// doba v tickách timecounteru za jak dlouho se má volat
	systime_t period;
	/// uloženo kdy byl naposled volané
	systime_t last;
	/// typ jesli pořád dokola nebo jenom jednou
	sh_t type;
	/// další delay v seznamu - linkovaný seznam
	delay_t * next;
};

void shFillStruct(delay_t * del, method_t method, arg_t arg, systime_t cycles,
		sh_t type);
void shRegisterStruct(delay_t * del);
void shUnregisterStruct(delay_t * del);
void shPlay(void);
void shRearm(delay_t * del);
uint8_t shIsRegistered(const delay_t * del);
systime_t shNow(void);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* SCHEDULER_H_ */
