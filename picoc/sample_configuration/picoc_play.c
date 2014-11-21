/* picoc main program - this varies depending on your operating system and
 * how you're using picoc */

/* include only picoc.h here - should be able to use it with only the external interfaces, no internals from interpreter.h */
#include "picoc.h"

extern const Picoc_Callbacks pc_cb;
extern const Picoc_IO pc_io;

extern struct LibraryFunction BasicFunctions[];
const Picoc_User_Library_Table libs[] =
{
{ "basic.h", NULL, NULL, BasicFunctions },
{ NULL, NULL, NULL, NULL } };

static const Picoc_User_Config auto_cfg =
{ (uint8_t *) 0x10000000, 1024 * 48, &pc_io, &pc_cb, libs };

static const Picoc_User_Config interactive_cfg =
{ (uint8_t *) 0x10000000 + 48 * 1024, 1024 * 16, &pc_io, &pc_cb, libs };

/* platform-dependent code for running programs is in this file */
int picoc(char *SourceStr)
{
	static Picoc pc;
	char *pos;

	PicocInitialise(&pc, &auto_cfg);
	pc.name = (const char *) "auto";
	if (SourceStr)
	{
		for (pos = SourceStr; *pos != 0; pos++)
		{
			if (*pos == 0x1a)
			{
				*pos = 0x20;
			}
		}
	}

	PicocIncludeAllSystemHeaders(&pc);
	if (SourceStr)
	{
		PicocParse(&pc, "nofile", "int i = 3;", strlen(SourceStr), TRUE, TRUE,
		FALSE, FALSE);
		PicocParse(&pc, "nofile", SourceStr, strlen(SourceStr), TRUE, TRUE,
		FALSE, FALSE);
	}

	PicocCleanup(&pc);

	return 0;
}

int picoc_interactive(void)
{
	static Picoc pc;

	PicocInitialise(&pc, &interactive_cfg);
	pc.name = (const char *) "interactive";
	PicocIncludeAllSystemHeaders(&pc);
	PicocParseInteractive(&pc);
	PicocCleanup(&pc);

	return 0;
}
