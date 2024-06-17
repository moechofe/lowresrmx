#include "core_plugin.h"

// Only one Runner for now, can change.
static Runner* runner = NULL;

void interpreterDidFail(void *context,struct CoreError coreError)
{
	Runner *runner=(Runner*)context;
	if(!runner->core) return;
	runner->runningError=coreError;
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
	core->delegate=&runner->delegate;
}

FFI_PLUGIN_EXPORT void runnerSetDelegate(Runner *runner,struct CoreDelegate *delegate)
{
	core_setDelegate(runner->core,delegate);
	int a=12;
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

// TODO: remove the Runner*
FFI_PLUGIN_EXPORT const char* runnerGetError(Runner *runner,enum ErrorCode code)
{
	if(!runner->core) return "Runner not ready";
	return err_getString(code);
}

FFI_PLUGIN_EXPORT void runnerStart(Runner *runner,int scondsSincePowerOn)
{
	if(!runner->core) return;
	runner->runningError=err_makeCoreError(ErrorNone,-1);
	core_willRunProgram(runner->core, scondsSincePowerOn);
}

FFI_PLUGIN_EXPORT struct CoreError runnerUpdate(Runner *runner,Input *input)
{
	if(!runner->core) return err_makeCoreError(ErrorNone,-1);
	core_update(runner->core,input);
	return runner->runningError;
}

FFI_PLUGIN_EXPORT void runnerRender(Runner *runner,void *pixels)
{
	if(!runner->core) return;
	if(!pixels) return;
	video_renderScreen(runner->core,pixels);
}

FFI_PLUGIN_EXPORT void runnerTrace(Runner *runner,bool enabled)
{
	if(!runner->core) return;
	core_setDebug(runner->core,enabled);
}

