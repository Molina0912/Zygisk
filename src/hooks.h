#ifndef HOOKS_H
#define HOOKS_H

#include <jni.h>

struct HookInfo {
    void* target;
    void* replacement;
    void* trampoline;
    void* original;
};

bool installHook(void* target, void* replacement, void** original);
bool installJniHooks(JNIEnv* env);
void installPlayIntegrityHooks(JNIEnv* env);
jint hooked_JNI_OnLoad(JavaVM* vm, void* reserved);
void hookPlayIntegrityMethods(JNIEnv* env, jclass clazz);

#endif // HOOKS_H
