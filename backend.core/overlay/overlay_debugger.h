#ifndef overlay_debugger_h
#define overlay_debugger_h

#include <stdbool.h>

struct Core;

void trigger_debugger(struct Core *core);
void overlay_debugger(struct Core *core);
void log_goto(struct Core *core,int symbolIndex);
void log_gosub(struct Core *core,int symbolIndex);
void log_return(struct Core *core,bool clear);

#endif /* overlay_debugger_h */
