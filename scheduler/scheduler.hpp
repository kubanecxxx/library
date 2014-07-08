#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP

#include "scheduler.h"

class Scheduler
{
private:
	delay_t del;
public:
	Scheduler()
	{
		Setup(NULL, NULL, 0);
	}

	Scheduler(method_t method, arg_t arg, systime_t cycles,
			sh_t type = PERIODIC)
	{
		Setup(method, arg, cycles, type);
	}

	inline void Setup(method_t method, arg_t arg, systime_t cycles, sh_t type =
			PERIODIC)
	{
		shFillStruct(&del, method, arg, cycles, type);
	}

	inline void Register()
	{
		shRegisterStruct(&del);
	}

	inline void Unregister()
	{
		shUnregisterStruct(&del);
	}

	inline void Rearm()
	{
		shRearm(&del);
	}

	inline bool IsRegistered() const
	{
		return shIsRegistered(&del);
	}

	inline void SetMethod(method_t method)
	{
		del.method = method;
	}

	inline void SetArg(arg_t arg)
	{
		del.arg = arg;
	}

	inline void SetPeriod(systime_t period)
	{
		del.period = period;
	}
#ifdef MS2ST
	inline void SetPeriodMilliseconds(systime_t ms)
	{
		del.period = MS2ST(ms);
	}
#endif
	inline void SetType(sh_t type)
	{
		del.type = type;
	}

	inline static void Play(void)
	{
		shPlay();
	}

	inline static systime_t Now(void)
	{
		return shNow();
	}
};

#endif // SCHEDULER_HPP
