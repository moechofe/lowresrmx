#include <jni.h>
#include "all.h"

extern struct Runner runner;

JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeSetKeyboardEnabled(JNIEnv *env, jclass clazz, jboolean enabled) {
	core_setKeyboardEnabled(runner.core, enabled ? 1 : 0);
}

JNIEXPORT void JNICALL
Java_lowresrmx_author_1name_game_1name_MyActivity_nativeSetKeyboardHeight(JNIEnv *env, jclass clazz, jint height) {
	core_setKeyboardHeight(runner.core, (int)height);
}
