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
	runner->runningError=err_makeCoreError(ErrorUserDeviceDiskFull,-1, -1);
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
	if(!runner->core) return err_makeCoreError(ErrorCouldNotOpenProgram,-1,-1);
	return core_compileProgram(runner->core,code,false);
}

// TODO: remove the Runner*
FFI_PLUGIN_EXPORT const char* runnerGetError(Runner *runner,enum ErrorCode code)
{
	if(!runner->core) return "Runner not ready";
	return err_getString(code);
}

FFI_PLUGIN_EXPORT void runnerStart(Runner *runner,Input *input,int scondsSincePowerOn,const char *originalDataDisk,size_t originalDataDiskSize)
{
	if(!runner->core) return;
	runner->runningError=err_makeCoreError(ErrorNone,-1,-1);
	if(input) core_handleInput(runner->core,input);
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
	if(!runner->core) return err_makeCoreError(ErrorNone,-1,-1);
	core_update(runner->core,input);
	return runner->runningError;
}

FFI_PLUGIN_EXPORT bool runnerShouldRender(Runner *runner)
{
	if(!runner->core) return false;
	return core_shouldRender(runner->core);
}

#if __ANDROID__
#include <android/native_window_jni.h>
#endif

typedef struct NativeTexture {
    int64_t textureId;
    void *nativeHandle;
    int width;
    int height;
    int pitch;
} NativeTexture;

#define MAX_NATIVE_TEXTURES 4
static NativeTexture nativeTextures[MAX_NATIVE_TEXTURES];

FFI_PLUGIN_EXPORT void runnerRegisterNativeTexture(int64_t textureId, void* nativeHandle)
{
    for (int i = 0; i < MAX_NATIVE_TEXTURES; i++) {
        if (nativeTextures[i].textureId == textureId) {
            // Same texture, new surface memory: iOS hands over a new pixel buffer on resize,
            // Android a new ANativeWindow. Only the render isolate gets here, so the window it
            // was blitting into can be released right away. The geometry belongs to
            // runnerSetTextureGeometry() and must survive this.
#if __ANDROID__
            if (nativeTextures[i].nativeHandle && nativeTextures[i].nativeHandle != nativeHandle) {
                ANativeWindow_release((ANativeWindow *)nativeTextures[i].nativeHandle);
            }
#endif
            nativeTextures[i].nativeHandle = nativeHandle;
            return;
        }
        if (nativeTextures[i].textureId == 0) {
            nativeTextures[i].textureId = textureId;
            nativeTextures[i].nativeHandle = nativeHandle;
            nativeTextures[i].width = SCREEN_WIDTH;
            nativeTextures[i].height = SCREEN_HEIGHT;
            nativeTextures[i].pitch = 0;
            return;
        }
    }
}

FFI_PLUGIN_EXPORT void runnerSetTextureGeometry(int64_t textureId, int width, int height, int pitch)
{
    if (width <= 0 || height <= 0) return;
    for (int i = 0; i < MAX_NATIVE_TEXTURES; i++) {
        if (nativeTextures[i].textureId == textureId) {
            nativeTextures[i].width = width;
            nativeTextures[i].height = height;
            nativeTextures[i].pitch = pitch;
#if __ANDROID__
            // Only the render isolate calls this, and it is also the only thread that locks the
            // window, so re-requesting the buffer size here never races a lock.
            if (nativeTextures[i].nativeHandle) {
                ANativeWindow_setBuffersGeometry((ANativeWindow *)nativeTextures[i].nativeHandle,
                    width, height, WINDOW_FORMAT_RGBA_8888);
            }
#endif
            return;
        }
    }
}

FFI_PLUGIN_EXPORT void runnerUnregisterNativeTexture(int64_t textureId)
{
    for (int i = 0; i < MAX_NATIVE_TEXTURES; i++) {
        if (nativeTextures[i].textureId == textureId) {
#if __ANDROID__
            if (nativeTextures[i].nativeHandle) {
                ANativeWindow_release((ANativeWindow *)nativeTextures[i].nativeHandle);
            }
#endif
            nativeTextures[i].textureId = 0;
            nativeTextures[i].nativeHandle = NULL;
            nativeTextures[i].width = 0;
            nativeTextures[i].height = 0;
            nativeTextures[i].pitch = 0;
            return;
        }
    }
}

#if __ANDROID__
#include <jni.h>
#include <android/log.h>

#define JNI_EXPORT __attribute__((visibility("default")))

/// Wraps a Surface in an ANativeWindow at the requested device-pixel size and hands the pointer
/// to Dart, which registers it from the render isolate. ANativeWindow_fromSurface() already
/// carries a reference; runnerRegisterNativeTexture()/runnerUnregisterNativeTexture() release it.
JNI_EXPORT jlong JNICALL
Java_com_lowresrmx_core_1plugin_CorePlugin_nativeSurfaceHandle(JNIEnv *env, jobject thiz, jobject surface, jint width, jint height) {
	ANativeWindow *window = ANativeWindow_fromSurface(env, surface);
	if (!window) return 0;
	// Force RGBA_8888 at the device-pixel size the surface was created with
	ANativeWindow_setBuffersGeometry(window, width, height, WINDOW_FORMAT_RGBA_8888);
	return (jlong)(intptr_t)window;
}

JNI_EXPORT void JNICALL
Java_com_lowresrmx_core_1plugin_CorePlugin_nativeUnregisterTexture(JNIEnv *env, jobject thiz, jlong texture_id) {
    runnerUnregisterNativeTexture(texture_id);
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    __android_log_print(ANDROID_LOG_INFO, "core_plugin", "JNI_OnLoad called");
    return JNI_VERSION_1_6;
}
#endif

// Opaque black in both byte orders the engine can emit: the alpha byte is the high byte of the
// 32-bit word for ABGR=0 (B,G,R,A) and ABGR=1 (R,G,B,A) alike.
#define OPAQUE_BLACK 0xff000000u

// One fantasy-resolution frame, upscaled from here into the device-resolution surface.
static uint32_t screenBuffer[SCREEN_WIDTH * SCREEN_HEIGHT];
static bool screenBufferCleared = false;

FFI_PLUGIN_EXPORT void screenBlitScaled(const uint32_t *src, uint32_t *dst, int dstWidth, int dstHeight, int dstPitch)
{
    if (!src || !dst || dstWidth <= 0 || dstHeight <= 0) return;
    if (dstPitch <= 0) dstPitch = dstWidth * 4;

    // The fantasy screen covers the device screen: scaled by whichever axis needs the larger
    // factor, pinned top-left, the other axis clipped. Same factor Runtime.resize() derives from
    // the logical size in runtime.dart (216/384 == 9/16 exactly).
    //
    // 1/scale is kept as the exact rational num/den = min(SCREEN_WIDTH/dstWidth,
    // SCREEN_HEIGHT/dstHeight), so a source coordinate is exactly floor(dst * num / den).
    // Stepping the remainder (Bresenham) gets there with one add and one compare per pixel:
    // no division, and no drift, which truncated 16.16 fixed point does accumulate — at
    // 1080x2340 a 16.16 step lands on source column 31 where the exact mapping says 32.
    int num, den;
    if ((int64_t)SCREEN_WIDTH * dstHeight <= (int64_t)SCREEN_HEIGHT * dstWidth)
    {
        num = SCREEN_WIDTH;
        den = dstWidth;
    }
    else
    {
        num = SCREEN_HEIGHT;
        den = dstHeight;
    }

    int prevSrcY = -1;
    uint32_t *prevRow = NULL;
    int srcY = 0, errY = 0;
    for (int y = 0; y < dstHeight; y++)
    {
        uint32_t *out = (uint32_t *)((uint8_t *)dst + (size_t)y * (size_t)dstPitch);
        const int rowY = srcY < SCREEN_HEIGHT ? srcY : SCREEN_HEIGHT - 1;
        // every source row lands on several device rows: copy the one just built
        if (rowY == prevSrcY && prevRow)
        {
            memcpy(out, prevRow, (size_t)dstWidth * 4);
        }
        else
        {
            const uint32_t *in = src + (size_t)rowY * SCREEN_WIDTH;
            int srcX = 0, errX = 0;
            for (int x = 0; x < dstWidth; x++)
            {
                out[x] = in[srcX < SCREEN_WIDTH ? srcX : SCREEN_WIDTH - 1];
                errX += num;
                while (errX >= den) { errX -= den; srcX++; }
            }
            prevSrcY = rowY;
            prevRow = out;
        }
        errY += num;
        while (errY >= den) { errY -= den; srcY++; }
    }
}

FFI_PLUGIN_EXPORT void runnerRenderToTexture(Runner* runner, int64_t textureId)
{
    if (!runner->core) return;

    NativeTexture *texture = NULL;
    for (int i = 0; i < MAX_NATIVE_TEXTURES; i++) {
        if (nativeTextures[i].textureId == textureId) {
            texture = &nativeTextures[i];
            break;
        }
    }

    if (!texture || !texture->nativeHandle) return;

    // In COMPAT mode the core writes only the shown region, so the rest must hold something
    // deliberate rather than uninitialised memory (same reason as LowResRMXView.clear()).
    if (!screenBufferCleared) {
        for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) screenBuffer[i] = OPAQUE_BLACK;
        screenBufferCleared = true;
    }
    video_renderScreen(runner->core, screenBuffer, SCREEN_WIDTH * 4);

#if __ANDROID__
    ANativeWindow *window = (ANativeWindow *)texture->nativeHandle;
    ANativeWindow_Buffer buffer;
    if (ANativeWindow_lock(window, &buffer, NULL) != 0) {
        __android_log_print(ANDROID_LOG_ERROR, "core_plugin", "ANativeWindow_lock failed");
        return;
    }
    if (buffer.bits) {
        // the locked buffer carries its own geometry; ANativeWindow_setBuffersGeometry() asked
        // for texture->width/height and stride is in pixels
        screenBlitScaled(screenBuffer, (uint32_t *)buffer.bits, buffer.width, buffer.height, buffer.stride * 4);
    }
    ANativeWindow_unlockAndPost(window);
#else
    // iOS (and any other platform): nativeHandle is the CVPixelBuffer base address
    screenBlitScaled(screenBuffer, (uint32_t *)texture->nativeHandle, texture->width, texture->height, texture->pitch);
#endif
}

FFI_PLUGIN_EXPORT void runnerRender(Runner *runner,void *pixels)
{
	if(!runner->core) return;
	if(!pixels) return;
	video_renderScreen(runner->core,pixels,SCREEN_WIDTH*4);
}

FFI_PLUGIN_EXPORT void runnerTrace(Runner *runner,bool enabled)
{
	if(!runner->core) return;
	core_setDebug(runner->core,enabled);
}

FFI_PLUGIN_EXPORT int runnerNumAssertions(Runner *runner)
{
	if(!runner->core) return 0;
	return runner->core->interpreter->numAssertions;
}

FFI_PLUGIN_EXPORT int runnerState(Runner *runner)
{
	if(!runner->core) return (int)StateNoProgram;
	return (int)runner->core->interpreter->state;
}

FFI_PLUGIN_EXPORT void* syntaxCreate(void)
{
	struct Syntax *syntax=malloc(sizeof(struct Syntax));
	if(!syntax) return NULL;
	syntax_init(syntax);
	return syntax;
}

FFI_PLUGIN_EXPORT void syntaxDestroy(void *handle)
{
	if(!handle) return;
	syntax_deinit((struct Syntax*)handle);
	free(handle);
}

FFI_PLUGIN_EXPORT int syntaxScan(void *handle,const char *sourceCode)
{
	if(!handle) return 0;
	if(!sourceCode) return 0;
	struct Syntax *syntax=(struct Syntax*)handle;
	syntax_update(syntax,sourceCode);
	return syntax->numSpans;
}

FFI_PLUGIN_EXPORT const struct SyntaxSpan* syntaxSpans(void *handle)
{
	if(!handle) return NULL;
	return ((struct Syntax*)handle)->spans;
}

FFI_PLUGIN_EXPORT void inputKeyDown(Input *input,int ascii)
{
	input->key=(char)(ascii & 0xff);
}

FFI_PLUGIN_EXPORT void runnerRenderAudio(Runner* runner, int16_t* output, int numSamples, int outputFrequency, int volume)
{
	if (!runner || !runner->core) return;
	audio_renderAudio(runner->core, output, numSamples, outputFrequency, volume);
}
