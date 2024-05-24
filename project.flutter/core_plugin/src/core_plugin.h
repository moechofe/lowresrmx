#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#if _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#if _WIN32
#define FFI_PLUGIN_EXPORT __declspec(dllexport)
#else
#define FFI_PLUGIN_EXPORT
#endif

// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b);

// A longer lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b);

#include "core.h"

typedef int RID;

typedef struct Runner {
	struct Core *core;
	struct CoreDelegate delegate;
} Runner;

typedef struct CoreInput Input;

FFI_PLUGIN_EXPORT void runnerInit(Runner*);
FFI_PLUGIN_EXPORT void runnerDeinit(Runner*);

FFI_PLUGIN_EXPORT struct CoreError runnerCompileProgram(Runner*,const char*);

FFI_PLUGIN_EXPORT const char* runnerGetError(Runner*,enum ErrorCode);

FFI_PLUGIN_EXPORT void runnerStart(Runner*,int scondsSincePowerOn);

FFI_PLUGIN_EXPORT void runnerUpdate(Runner*,Input*);

FFI_PLUGIN_EXPORT void runnerRender(Runner*,void*);

