/* all platform-specific includes and defines go in this file */
#ifndef PLATFORM_H
#define PLATFORM_H

/* configurable options */
#include "ch.h"
#include "hal.h"
#include "chprintf.h"
#include <string.h>

#define LARGE_INT_POWER_OF_TEN 1000000000   /* the largest power of ten which fits in an int on this architecture */
#define ALIGN_TYPE void *                   /* the default data type to use for alignment */

#define GLOBAL_TABLE_SIZE 97                /* global variable table */
#define STRING_TABLE_SIZE 97                /* shared string table size */
#define STRING_LITERAL_TABLE_SIZE 97        /* string literal table size */
#define PARAMETER_MAX 16                    /* maximum number of parameters to a function */
#define LINEBUFFER_MAX 256                  /* maximum number of characters on a line */
#define LOCAL_TABLE_SIZE 11                 /* size of local variable table (can expand) */
#define STRUCT_TABLE_SIZE 11                /* size of struct/union member table (can expand) */
#define RESERVED_WORD_TABLE_SIZE 97         /* reserved word table size */

#define INTERACTIVE_PROMPT_START "starting picoc " PICOC_VERSION "\n"
#define INTERACTIVE_PROMPT_STATEMENT "picoc> "
#define INTERACTIVE_PROMPT_LINE "     > "

#define NO_FP				//no floating point support
#define NO_CALLOC
#define NO_REALLOC
#define NO_STRING_FUNCTIONS
#define NO_CTYPE
#define NO_MODULUS
#define BROKEN_FLOAT_CASTS
#define BUILTIN_MINI_STDLIB
#define NO_PRINT_FUNCTIONS

//debugging functions and messages
//#define DEBUG_HEAP
//#define DEBUG_ARRAY_INITIALIZER

//printf redefinition
#if defined DEBUG_HEAP || defined DEBUG_ARRAY_INITIALIZER
#define printf(fmt,... ) PlatformDebugPrintf(pc,fmt,__VA_ARGS__)
#define fprintf(stderr,fmt,... ) PlatformDebugPrintf(pc,fmt,__VA_ARGS__)
#endif

//asserts definition
#if 0
#define assert(x) chDbgCheck(x)
#else
#define assert(xb)
#endif

#define cfg_check(x) chDbgCheck(x)



#endif /* PLATFORM_H */
