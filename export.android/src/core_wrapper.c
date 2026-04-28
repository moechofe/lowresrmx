#include <jni.h>
#include "all.h"

// CoreWrapper struct to hold references
typedef struct {
  struct Core core;
  struct CoreInput input;
  struct CoreDelegate delegate;
} CoreWrapper;

// Global instance
static CoreWrapper g_coreWrapper;

// Helper to initialize CoreWrapper and set delegate
void corewrapper_init() {
  core_init(&g_coreWrapper.core);
  // Setup delegate as needed (fill function pointers, context, etc.)
  // For now, zero-init is enough for basic use
  core_setDelegate(&g_coreWrapper.core, &g_coreWrapper.delegate);
}

// JNI method to initialize CoreWrapper (call once from Java, e.g. in onCreate)
JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeInitCoreWrapper(JNIEnv *env, jobject thiz) {
  corewrapper_init();
}

// JNI method to call core_update
JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeCoreUpdate(JNIEnv *env, jobject thiz) {
  core_update(&g_coreWrapper.core, &g_coreWrapper.input);
}

// static JavaVM* g_vm = NULL;
// static jmethodID g_onControlsDidChangeMethod = NULL;
// static jobject g_coreWrapperObj = NULL;

// JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
//     g_vm = vm;
//     JNIEnv* env;
//     if ((*vm)->GetEnv(vm, (void**)&env, JNI_VERSION_1_6) != JNI_OK) {
//         return JNI_ERR;
//     }

//     jclass cls = (*env)->FindClass(env, "lowresrmx/author_name/game_name/CoreWrapper");
//     if (cls) {
//         g_onControlsDidChangeMethod = (*env)->GetMethodID(env, cls, "onControlsDidChange", "(IIZZZ)V");
//     }

//     return JNI_VERSION_1_6;
// }

// JNIEXPORT void JNICALL
// Java_lowresrmx_author_1name_1game_1name_CoreWrapper_nativeRegister(JNIEnv *env, jobject thiz) {
//     if (g_coreWrapperObj) {
//         (*env)->DeleteGlobalRef(env, g_coreWrapperObj);
//     }
//     g_coreWrapperObj = (*env)->NewGlobalRef(env, thiz);
// }

// void jni_notify_controls_changed(struct ControlsInfo controlsInfo) {
//     if (!g_vm || !g_coreWrapperObj || !g_onControlsDidChangeMethod) {
//         return;
//     }

//     JNIEnv* env;
//     jint res = (*g_vm)->GetEnv(g_vm, (void**)&env, JNI_VERSION_1_6);
//     int attached = 0;
//     if (res == JNI_EDETACHED) {
//         if ((*g_vm)->AttachCurrentThread(g_vm, &env, NULL) != JNI_OK) {
//             return;
//         }
//         attached = 1;
//     }

//     (*env)->CallVoidMethod(env, g_coreWrapperObj, g_onControlsDidChangeMethod,
//                            (jint)controlsInfo.keyboardMode,
//                            (jint)controlsInfo.hapticMode,
//                            (jboolean)controlsInfo.isAudioEnabled,
//                            (jboolean)controlsInfo.isInputState,
//                            (jboolean)controlsInfo.isCompatMode);

//     if (attached) {
//         (*g_vm)->DetachCurrentThread(g_vm);
//     }
// }
