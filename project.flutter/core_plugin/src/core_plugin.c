#include <stdlib.h>
#include <string.h>

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
	Runner *runner=(Runner*)context;
	if(!runner->core) return false;
	if(!runner->dataDisk) return false;
	runner->runningError=data_import(diskDataManager,runner->dataDisk,true);
	return true;
}

void diskDriveDidSave(void *context,struct DataManager *diskDataManager)
{
	Runner *runner=(Runner*)context;
	if(!runner->core) return;
	if(!runner->dataDisk) return;
	char *output=data_export(diskDataManager);
	if(output)
	{
		if(runner->dataDisk) free(runner->dataDisk);
		runner->dataDisk=output;
		runner->dataDiskSize=strlen(output);
		runner->shouldSaveDisk=true;
	}
}

void diskDriveIsFull(void *context,struct DataManager *diskDataManager)
{
	Runner *runner=(Runner*)context;
	if(!runner->core) return;
	runner->runningError=err_makeCoreError(ErrorUserDeviceDiskFull,-1);
}

void controlsDidChange(void *context,struct ControlsInfo controlsInfo)
{
	Runner *runner=(Runner*)context;
	if(!runner->core) return;
	runner->shouldOpenKeyboard=controlsInfo.keyboardMode==KeyboardModeOn;
	runner->shouldEnableInputMode=controlsInfo.isInputState;
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
	runner->dataDisk=NULL;
	runner->dataDiskSize=0;
	runner->shouldSaveDisk=false;
	runner->shouldOpenKeyboard=false;
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
	if(runner->dataDisk) { free(runner->dataDisk); runner->dataDiskSize=0; }
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

FFI_PLUGIN_EXPORT void runnerStart(Runner *runner,int scondsSincePowerOn,const char *originalDataDisk,size_t originalDataDiskSize)
{
	if(!runner->core) return;
	runner->runningError=err_makeCoreError(ErrorNone,-1);
	core_willRunProgram(runner->core,scondsSincePowerOn);
	// originalDataDisk memory is managed by the caller.
	// runner->dataDisk memory is managed by the callee.
	if(runner->dataDisk) { free(runner->dataDisk); runner->dataDisk=NULL; runner->dataDiskSize=0; }
	if(originalDataDisk)
	{
		runner->dataDisk = calloc(1, originalDataDiskSize+1);
		if(runner->dataDisk)
		{
			memcpy(runner->dataDisk,originalDataDisk,originalDataDiskSize);
			runner->dataDiskSize=originalDataDiskSize;
		}
	}
	runner->shouldSaveDisk=false;
}

FFI_PLUGIN_EXPORT struct CoreError runnerUpdate(Runner *runner,Input *input)
{
	if(!runner->core) return err_makeCoreError(ErrorNone,-1);
	core_update(runner->core,input);
	return runner->runningError;
}

FFI_PLUGIN_EXPORT bool runnerShouldRender(Runner *runner)
{
	if(!runner->core) return false;
	return core_shouldRender(runner->core);
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

FFI_PLUGIN_EXPORT int runnerGetSymbolCount(Runner *runner)
{
	if(!runner->core) return 0;
	struct Tokenizer *tokenizer=&runner->core->interpreter->tokenizer;
	return tokenizer->numJumpLabelItems + tokenizer->numSubItems;
}

FFI_PLUGIN_EXPORT const char* runnerGetSymbolName(Runner *runner,int i)
{
	if(!runner->core) return NULL;
	struct Tokenizer *tokenizer=&runner->core->interpreter->tokenizer;

	if (i > tokenizer->numJumpLabelItems + tokenizer->numSubItems)
	{
		return NULL;
	}
	else if (i < tokenizer->numJumpLabelItems)
	{
		int s = tokenizer->jumpLabelItems[i].symbolIndex;
		return tokenizer->symbols[s].name;
	}
	else
	{
		int s = tokenizer->subItems[i - tokenizer->numJumpLabelItems].symbolIndex;
		return tokenizer->symbols[s].name;
	}
}

FFI_PLUGIN_EXPORT int runnerGetSymbolPosition(Runner *runner,int i)
{
	if(!runner->core) return -1;
	struct Tokenizer *tokenizer=&runner->core->interpreter->tokenizer;

	if (i > tokenizer->numJumpLabelItems + tokenizer->numSubItems)
	{
		return -1;
	}
	else if (i < tokenizer->numJumpLabelItems)
	{
		return tokenizer->jumpLabelItems[i].token->sourcePosition;
	}
	else
	{
		return tokenizer->subItems[i - tokenizer->numJumpLabelItems].token->sourcePosition;
	}
}

FFI_PLUGIN_EXPORT void inputKeyDown(Input *input,int ascii)
{
	input->key=(char)(ascii & 0xff);
}
