#include <jni.h>
#include "all.h"

extern struct Runner runner;

JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeSetKeyboardEnabled(JNIEnv *env, jclass clazz, jboolean enabled) {
    if (!runner.core) return;
	core_setKeyboardEnabled(runner.core, enabled ? 1 : 0);
}

JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeSetKeyboardHeight(JNIEnv *env, jclass clazz, jint height) {
    if (!runner.core) return;
	core_setKeyboardHeight(runner.core, (int)height);
}

JNIEXPORT jint JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeMachinePeek(JNIEnv *env, jclass clazz, jint address) {
    if (!runner.core) return -1;
    return (jint)machine_peek(runner.core, (int)address);
}

JNIEXPORT jboolean JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeMachinePoke(JNIEnv *env, jclass clazz, jint address, jint value) {
    if (!runner.core) return JNI_FALSE;
    return machine_poke(runner.core, (int)address, (int)value) ? JNI_TRUE : JNI_FALSE;
}
