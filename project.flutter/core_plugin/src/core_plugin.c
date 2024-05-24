#include "core_plugin.h"





// TextureBox







// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b) { return a + b; }

// A longer-lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b) {
  // Simulate work.
#if _WIN32
  Sleep(5000);
#else
  usleep(5000 * 1000);
#endif
  return a + b;
}

// Only one Runner for now, can change.
static Runner* runner = NULL;
// Allow to have multiple Runner, not sure if it usefull
#define MAX_RID 0

void interpreterDidFail(void *context,struct CoreError coreError)
{
	int a=12;
}

bool diskDriveWillAccess(void *context,struct DataManager *diskDataManager)
{
	int a=12;
	return true;
}

void diskDriveDidSave(void *context,struct DataManager *diskDataManager)
{
	int a=12;
}

void diskDriveIsFull(void *context,struct DataManager *diskDataManager)
{
	int a=12;
}

void controlsDidChange(void *context,struct ControlsInfo controlsInfo)
{
	int a=12;
}

void persistentRamWillAccess(void *context,uint8_t *destination,int size)
{
	int a=12;
}

void persistentRamDidChange(void *context,uint8_t *data,int size)
{
	int a=12;
}

FFI_PLUGIN_EXPORT void runnerInit(Runner *runner)
{
	struct Core *core=calloc(1,sizeof(struct Core));
	if(!core) return;
	runner->core=core;
	runner->delegate.context=runner;
	runner->delegate.interpreterDidFail=interpreterDidFail;
	runner->delegate.diskDriveWillAccess=diskDriveWillAccess;
	runner->delegate.diskDriveDidSave=diskDriveDidSave;
	runner->delegate.diskDriveIsFull=diskDriveIsFull;
	runner->delegate.controlsDidChange=controlsDidChange;
	runner->delegate.persistentRamWillAccess=persistentRamWillAccess;
	runner->delegate.persistentRamDidChange=persistentRamDidChange;
	core_init(core);
	core_setDelegate(core,&runner->delegate);
}

FFI_PLUGIN_EXPORT void runnerDeinit(Runner *runner)
{
	if(!runner->core) return;
	core_deinit(runner->core);
	free(runner->core);
	runner->core=NULL;
}

FFI_PLUGIN_EXPORT struct CoreError runnerCompileProgram(Runner *runner,const char *code)
{
	if(!runner->core) return err_makeCoreError(ErrorCouldNotOpenProgram,-1);
	return core_compileProgram(runner->core,code,false);
}

FFI_PLUGIN_EXPORT const char* runnerGetError(Runner *runner,enum ErrorCode code)
{
	if(!runner->core) return "Runner not ready";
	return err_getString(code);
}

FFI_PLUGIN_EXPORT void runnerStart(Runner *runner,int scondsSincePowerOn)
{
	if(!runner->core) return;
	core_willRunProgram(runner->core, scondsSincePowerOn);
}

FFI_PLUGIN_EXPORT void runnerUpdate(Runner *runner,Input *input)
{
	if(!runner->core) return;
	core_update(runner->core,input);
}

FFI_PLUGIN_EXPORT void runnerRender(Runner *runner,void *pixels)
{
	if(!runner->core) return;
	if(!pixels) return;

	video_renderScreen(runner->core,pixels);
}

