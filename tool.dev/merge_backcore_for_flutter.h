#ifndef SL_GLOBMATCH_NEGATE
#define SL_GLOBMATCH_NEGATE '^'       /* std char set negation char */
#endif

#include "error.h"
#include "core.h"
#include "boot_intro.h"
#include "core_delegate.h"
#include "core_stats.h"
#include "string_utils.h"
#include "startup_sequence.h"
#include "charsets.h"
#include "interpreter_utils.h"
#include "cmd_text.h"
#include "default_characters.h"
#include "overlay_debugger.h"
#include "log.h"
#include "globmatch.h"

#include <stdio.h>

#ifndef LOGH
#define LOGH

FILE *logh=fopen("log.txt","w");

void log_message(const char *message)
{
	if (logh)
	{
		fprintf(logh, "%s\n", message);
		fflush(logh);
	}
}

#endif
