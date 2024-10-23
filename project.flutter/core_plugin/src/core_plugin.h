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

#include "core.h"

typedef int RID;

/// @brief Hold stuff to control the core.
typedef struct Runner {
	/// @brief Pointer to the core.
	struct Core *core;
	/// @brief Pointer to the core's delegate. I am totally not using the delegate has it was designed. I never manage to make it work. On the c side some Flutter pointer wasn't readable.
	struct CoreDelegate delegate;
	/// @brief Used to report the error when a program is running. It is filled by a delegate
	struct CoreError runningError;
	/// @brief Holding the data disk, memory is own by the core.
	char *dataDisk;
	/// @brief Size of the data disk.
	int dataDiskSize;
	/// @brief Used to know if the data disk should be saved on Flutter side.
	bool shouldSaveDisk;
	/// @brief Used to know if the keyboard should be opened on Flutter side.
	bool shouldOpenKeyboard;
} Runner;

typedef struct CoreInput Input;

FFI_PLUGIN_EXPORT void runnerInit(Runner*);
FFI_PLUGIN_EXPORT void runnerDeinit(Runner*);

FFI_PLUGIN_EXPORT struct CoreError runnerCompileProgram(Runner*,const char*);
FFI_PLUGIN_EXPORT const char* runnerGetError(Runner*,enum ErrorCode);

FFI_PLUGIN_EXPORT void runnerStart(Runner*,int scondsSincePowerOn,const char *originalDataDisk,size_t originalDataDiskSize);

FFI_PLUGIN_EXPORT struct CoreError runnerUpdate(Runner*,Input*);

FFI_PLUGIN_EXPORT void runnerRender(Runner*,void*);

FFI_PLUGIN_EXPORT void runnerTrace(Runner*,bool);

FFI_PLUGIN_EXPORT int runnerGetSymbolCount(Runner*);
FFI_PLUGIN_EXPORT const char* runnerGetSymbolName(Runner*,int);
FFI_PLUGIN_EXPORT int runnerGetSymbolPosition(Runner*,int);

FFI_PLUGIN_EXPORT void inputKeyDown(Input*,int);

