/**
 * @file picoc.c
 * @author kubanec
 * @date 21. 11. 2014
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "picoc.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/* initialise everything */
void PicocInitialise(Picoc *pc, const Picoc_User_Config * cfg)
{
	memset(pc, '\0', sizeof(*pc));

	cfg_check(cfg);
	cfg_check(cfg->heapBase);
	cfg_check(cfg->heapSize);
	cfg_check(cfg->cb);
	cfg_check(cfg->io);
	cfg_check(cfg->io->tempName);
	cfg_check(cfg->io->readline);
	cfg_check(cfg->io->dprintf);
	cfg_check(cfg->io->getch);
	cfg_check(cfg->io->printVf);
	cfg_check(cfg->io->printf);
	cfg_check(cfg->cb->exit);

	pc->platform = cfg;

	if (cfg->cb->init)
		PlatformInit(pc);
	HeapInit(pc, cfg->heapSize, cfg->heapBase);
	TableInit(pc);
	VariableInit(pc);
	LexInit(pc);
	TypeInit(pc);
	LibraryInit(pc);

	//list of libraries register here
	if (!cfg->lib_table)
		return;

	const Picoc_User_Library_Table * line = cfg->lib_table;
	while (line && line->filename  && line->table)
	{
		cfg_check(line);
		cfg_check(line->filename);
		cfg_check(line->table);
		IncludeRegister(pc, line->filename, line->include_init, line->table,
		NULL);
		line++;
	}
}

/* free memory */
void PicocCleanup(Picoc *pc)
{
	const Picoc_User_Config * cfg = pc->platform;
	if (!cfg->lib_table)
		return;

	const Picoc_User_Library_Table * line = cfg->lib_table;
	while (line && line->filename)
	{
		if (line->clean)
			line->clean(pc);
		line++;
	}

	IncludeCleanup(pc);
	ParseCleanup(pc);
	LexCleanup(pc);
	VariableCleanup(pc);
	TypeCleanup(pc);
	TableStrFree(pc);
	HeapCleanup(pc);
	if (pc->platform->cb->clean)
		PlatformCleanup(pc);
}

