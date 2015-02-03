/* picoc's interface to the underlying platform. most platform-specific code
 * is in platform/platform_XX.c and platform/library_XX.c */

#include "picoc.h"
#include "interpreter.h"
#include "ch.h"
#include "hal.h"
#include "chprintf.h"

extern SerialUSBDriver SDU1;
static SerialUSBDriver * drv = &SDU1;
static BaseSequentialStream * usb = (BaseSequentialStream*) &SDU1;

static void pc_exit(Picoc * pc, int Retval);
static int pc_getch(void);
static char *pc_readline(char *Buf, int MaxLen, const char *Prompt);
static char *pc_tempname(Picoc *pc, char *TempNameBuffer);
static void pc_printvf(Picoc *pc, const char *format, va_list Args);
static void pc_dprintf(Picoc *pc, const char *format, ...);
static void pc_printf(Picoc *pc, const char *format, ...);

const Picoc_IO pc_io =
{ pc_printf, pc_dprintf, pc_printvf, pc_readline, pc_tempname, pc_getch };

const Picoc_Callbacks pc_cb =
{
NULL, NULL, pc_exit };

static bool PrintPossible(void)
{
	chSysLock();
	bool a = (drv->config->usbp->state == USB_ACTIVE && drv
			&& !chIQIsFullI(&drv->iqueue));
	chSysUnlock();
	return a;
}

/* printf for compiler error reporting */
void pc_printf(Picoc *pc, const char *format, ...)
{
	(void)pc;
	va_list Args;
	va_start(Args, format);
	if (PrintPossible())
		chvprintf(usb, format, Args);
	/*
	if (pc->lcdStream)
		chvprintf(usb, format, Args);
		*/
	va_end(Args);
}

uint8_t debug_enabled = 0;

void pc_dprintf(Picoc *pc, const char *format, ...)
{
	(void)pc;
	if (!debug_enabled)
		return;
	va_list Args;
	va_start(Args, format);
	if (PrintPossible())
		chvprintf(usb, format, Args);
	va_end(Args);
}

void pc_printvf(Picoc *pc, const char *format, va_list Args)
{
	(void)pc;
	if (PrintPossible())
		chvprintf(usb, format, Args);
	/*
	if (pc->lcdStream)
		chvprintf(pc->lcdStream, format, Args);
		*/
}

/* make a new temporary name. takes a static buffer of char [7] as a parameter. should be initialised to "XX0000"
 * where XX can be any characters */

char *pc_tempname(Picoc *pc, char *TempNameBuffer)
{
	int CPos = 5;

	while (CPos > 1)
	{
		if (TempNameBuffer[CPos] < '9')
		{
			TempNameBuffer[CPos]++;
			return TableStrRegister(pc, TempNameBuffer);
		}
		else
		{
			TempNameBuffer[CPos] = '0';
			CPos--;
		}
	}

	return TableStrRegister(pc, TempNameBuffer);
}

/* get a line of interactive input */
char *pc_readline(char *Buf, int MaxLen, const char *Prompt)
{
	if (PrintPossible())
	{
		chprintf((BaseSequentialStream *) drv, "\r\n");
		chprintf((BaseSequentialStream *) drv, Prompt);
	}
	char * b = Buf;
	uint8_t c = pc_getch();
	while (c && c != '\r' && MaxLen-- > 0)
	{
		if (c != '\n' && (c < 127))
		{
			*Buf = c;
			Buf++;
		}

		c = pc_getch();
	}

	*Buf = 0;

	return b;
}

/* get a character of interactive input */
int pc_getch(void)
{
	return chSequentialStreamGet(drv);
}

/* exit the program */
void pc_exit(Picoc * pc, int Retval)
{
	(void) Retval;
	(void) pc;
	PlatformPrintf(pc, "exiting picoc");
	chThdSleepMilliseconds(200);

	chThdExit(1);
}

