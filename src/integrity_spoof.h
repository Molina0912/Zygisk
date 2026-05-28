#ifndef INTEGRITY_SPOOF_H
#define INTEGRITY_SPOOF_H

#include <jni.h>

// Constantes de verificación
#define INTEGRITY_SUCCESS 0
#define INTEGRITY_FAILURE -1
#define BOOTLOADER_LOCKED 0
#define BOOTLOADER_UNLOCKED 1
#define PLAY_PROTECT_ENABLED 0
#define PLAY_PROTECT_DISABLED 1

// Funciones de spoofing
jobject spoofIntegrityTokenResponse(JNIEnv* env, jobject request);
jstring getCallingPackageName(JNIEnv* env);

// Hooks
jobject hooked_requestIntegrityToken(JNIEnv* env, jobject thiz, jobject request);
int hooked_checkIntegrity(const char* packageName, int flags);
int hooked_verifyBootloader();
int hooked_checkPlayProtect();

#endif // INTEGRITY_SPOOF_H
