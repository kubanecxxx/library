/**
 * @file fail_messages.c
 * @author kubanec
 * @date 21. 11. 2014
 *
 */

/* Includes ------------------------------------------------------------------*/
#include "picoc_platform.h"
#include "interpreter.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/


static void PrintSourceTextErrorLine(Picoc * pc, const char *SourceText, int Line,
		int CharacterPos)
{
	int LineCount;
	const char *LinePos;
	const char *CPos;
	int CCount;

	if (SourceText != NULL)
	{
		/* find the source line */
		for (LinePos = SourceText, LineCount = 1;
				*LinePos != '\0' && LineCount < Line; LinePos++)
		{
			if (*LinePos == '\n')
				LineCount++;
		}

		/* display the line */
		 for (CPos = LinePos; *CPos != '\n' && *CPos != '\0'; CPos++)
		 PlatformPrintf(pc,"%c", *CPos);
		 PlatformPrintf(pc, " ");


		/* display the error position */
		for (CPos = LinePos, CCount = 0;
				*CPos != '\n' && *CPos != '\0'
						&& (CCount < CharacterPos || *CPos == ' ');
				CPos++, CCount++)
		{
			if (*CPos == '\t')
				PlatformPrintf(pc, "\t");
			else
				PlatformPrintf(pc, " ");
		}
	}
	else
	{
		/* assume we're in interactive mode - try to make the arrow match up with the input text */
		for (CCount = 0;
				CCount
						< CharacterPos
								+ (int) strlen(INTERACTIVE_PROMPT_STATEMENT);
				CCount++)
			PlatformPrintf(pc, " ");
	}
	PlatformPrintf(pc, "%d:%d ", Line, CharacterPos);
}

/* exit with a message */
void ProgramFail(struct ParseState *Parser, const char *Message, ...)
{
	va_list Args;

	PrintSourceTextErrorLine(Parser->pc, Parser->SourceText, Parser->Line,
			Parser->CharacterPos);
	va_start(Args, Message);

	PlatformVPrintf(Parser->pc, Message, Args);
	va_end(Args);
	PlatformPrintf(Parser->pc, "\n");
	PlatformExit(Parser->pc, 1);
}

/* exit with a message, when we're not parsing a program */
void ProgramFailNoParser(Picoc *pc, const char *Message, ...)
{
	va_list Args;

	va_start(Args, Message);
	PlatformVPrintf(pc, Message, Args);
	va_end(Args);
	PlatformPrintf(pc, "\n");
	PlatformExit(pc, 1);
}

/* like ProgramFail() but gives descriptive error messages for assignment */
void AssignFail(struct ParseState *Parser, const char *Format,
		struct ValueType *Type1, struct ValueType *Type2, int Num1, int Num2,
		const char *FuncName, int ParamNo)
{
	Picoc * pc = Parser->pc;
	PrintSourceTextErrorLine(pc, Parser->SourceText, Parser->Line,
			Parser->CharacterPos);
	PlatformPrintf(pc, "can't %s ", (FuncName == NULL) ? "assign" : "set");

	if (Type1 != NULL)
		PlatformPrintf(pc, Format, Type1, Type2);
	else
		PlatformPrintf(pc, Format, Num1, Num2);

	if (FuncName != NULL)
		PlatformPrintf(pc, " in argument %d of call to %s()", ParamNo,
				FuncName);

	PlatformPrintf(pc, "\n");
	PlatformExit(Parser->pc, 1);
}

/* exit lexing with a message */
void LexFail(Picoc *pc, struct LexState *Lexer, const char *Message, ...)
{
	va_list Args;

	PrintSourceTextErrorLine(pc, Lexer->SourceText, Lexer->Line,
			Lexer->CharacterPos);
	va_start(Args, Message);
	PlatformVPrintf(pc, Message, Args);
	va_end(Args);
	PlatformPrintf(pc, "\n");
	PlatformExit(pc, 1);
}
