#include "interpreter.h"

static uint8_t _data[256];
void pif_save_data(struct ParseState *Parser, struct Value *ReturnValue,
		struct Value **Param, int NumArgs)
{
	(void) NumArgs;
	(void) Parser;
	(void) ReturnValue;
	size_t idx = Param[0]->Val->UnsignedCharacter;
	uint8_t data = Param[1]->Val->UnsignedCharacter;

	if (idx < sizeof(_data))
		_data[idx] = data;
}

void pif_load_data(struct ParseState *Parser, struct Value *ReturnValue,
		struct Value **Param, int NumArgs)
{
	(void) NumArgs;
	(void) Parser;
	uint8_t idx = Param[0]->Val->UnsignedCharacter;
	ReturnValue->Val->UnsignedCharacter = _data[idx];
}

void pif_free_mem(struct ParseState *Parser, struct Value *ReturnValue,
		struct Value **Param, int NumArgs)
{
	(void) NumArgs;
	(void) ReturnValue;
	(void) Param;

	size_t mem = Parser->pc->HeapBottom - Parser->pc->HeapStackTop;
	PlatformPrintf(Parser->pc, "free picoc memory %d", mem);
}


/* list of all library functions and their prototypes */
struct LibraryFunction BasicFunctions[] =
{
{ pif_save_data, "void save_data(unsigned char, unsigned char);" },
{ pif_load_data, "unsigned char load_data(unsigned char);" },
{ pif_free_mem, "void memory(void);" },
{ NULL, NULL } };

