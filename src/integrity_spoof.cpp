#include "integrity_spoof.h"
#include <android/log.h>
#include <string>
#include <cstring>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PIB-Spoof", __VA_ARGS__)

// Respuesta spoofeada para Play Integrity (referencia, JSON dentro del JWT)
static const char* SPOOFED_INTEGRITY_RESPONSE = R"({
    "appRecognitionVerdict": "VERIFIED",
    "deviceRecognitionVerdict": "VERIFIED",
    "deviceActivityLevel": "UNUSUAL",
    "appLicensingVerdict": "LICENSED",
    "playProtectVerdict": "NO_ISSUES",
    "deviceAttributes": {
        "deviceIntegrity": ["MEETS_DEVICE_INTEGRITY", "MEETS_BASIC_INTEGRITY"],
        "accountDetails": {
            "appLicensingStatus": "LICENSED"
        }
    },
    "appAccessRiskVerdict": {
        "riskLevel": "LOW",
        "riskFactors": []
    }
})";

// Implementación de hooks para Play Integrity API

jobject spoofIntegrityTokenResponse(JNIEnv* env, jobject /*request*/) {
    LOGD("Spoofeando respuesta de Play Integrity");
    (void)SPOOFED_INTEGRITY_RESPONSE; // referencia para inspección/logs futuros

    jclass responseClass = env->FindClass("com/google/android/gms/integrity/IntegrityTokenResponse");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    if (!responseClass) {
        LOGD("No se encontró clase IntegrityTokenResponse");
        return nullptr;
    }

    jmethodID constructor = env->GetMethodID(responseClass, "<init>", "(Ljava/lang/String;)V");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    if (!constructor) {
        LOGD("No se encontró constructor");
        env->DeleteLocalRef(responseClass);
        return nullptr;
    }

    jstring tokenString = env->NewStringUTF(
        "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9."
        "eyJhcHBSZWNvZ25pdGlvblZlcmRpY3QiOiJWRVJJRklFRCIsImRldmljZVJlY29nbml0aW9uVmVyZGljdCI6IlZFUklGSUVEIiwiYXBwTGljZW5zaW5nVmVyZGljdCI6IkxJQ0VOU0VEIiwicGxheVByb3RlY3RWZXJkaWN0IjoiTk9fSVNTVUVTIn0."
        "signature");

    jobject response = env->NewObject(responseClass, constructor, tokenString);

    env->DeleteLocalRef(tokenString);
    env->DeleteLocalRef(responseClass);

    return response;
}

void* original_requestIntegrityToken = nullptr;

jobject hooked_requestIntegrityToken(JNIEnv* env, jobject /*thiz*/, jobject request) {
    LOGD("Interceptada llamada a requestIntegrityToken");

    jstring packageName = getCallingPackageName(env);
    if (packageName) {
        const char* pkg = env->GetStringUTFChars(packageName, nullptr);
        if (pkg) {
            LOGD("Solicitud desde paquete: %s", pkg);
            env->ReleaseStringUTFChars(packageName, pkg);
        }
        env->DeleteLocalRef(packageName);
    }

    return spoofIntegrityTokenResponse(env, request);
}

jstring getCallingPackageName(JNIEnv* env) {
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (!activityThread) return nullptr;

    jmethodID currentApp = env->GetStaticMethodID(activityThread, "currentApplication",
                                                  "()Landroid/app/Application;");
    if (!currentApp) {
        env->DeleteLocalRef(activityThread);
        return nullptr;
    }

    jobject app = env->CallStaticObjectMethod(activityThread, currentApp);
    if (env->ExceptionCheck()) env->ExceptionClear();
    if (!app) {
        env->DeleteLocalRef(activityThread);
        return nullptr;
    }

    jclass appClass = env->GetObjectClass(app);
    jmethodID getPackageName = env->GetMethodID(appClass, "getPackageName", "()Ljava/lang/String;");

    jstring result = nullptr;
    if (getPackageName) {
        result = (jstring)env->CallObjectMethod(app, getPackageName);
        if (env->ExceptionCheck()) env->ExceptionClear();
    }

    env->DeleteLocalRef(app);
    env->DeleteLocalRef(appClass);
    env->DeleteLocalRef(activityThread);

    return result;
}

int hooked_checkIntegrity(const char* packageName, int /*flags*/) {
    LOGD("Interceptada verificación de integridad para: %s", packageName ? packageName : "(null)");
    return INTEGRITY_SUCCESS;
}

int hooked_verifyBootloader() {
    LOGD("Interceptada verificación de bootloader");
    return BOOTLOADER_LOCKED;
}

int hooked_checkPlayProtect() {
    LOGD("Interceptada verificación de Play Protect");
    return PLAY_PROTECT_ENABLED;
}
