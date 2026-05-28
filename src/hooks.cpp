#include "hooks.h"
#include "integrity_spoof.h"
#include <android/log.h>
#include <sys/mman.h>
#include <dlfcn.h>
#include <string.h>
#include <vector>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PIB-Hooks", __VA_ARGS__)

// Estructura para almacenar hooks
static std::vector<HookInfo> g_hooks;

// Implementación de trampolín para ARM64
static void* createTrampoline(void* /*target*/, void* replacement) {
    // Asignar memoria ejecutable para el trampolín
    void* trampoline = mmap(nullptr, 4096, PROT_READ | PROT_WRITE | PROT_EXEC,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (trampoline == MAP_FAILED) {
        return nullptr;
    }

    // Escribir código de trampolín ARM64:
    //   ldr x16, [pc, #0]
    //   br  x16
    //   <addr low>
    //   <addr high>
    unsigned char code[] = {
        0x50, 0x00, 0x00, 0x58, // ldr x16, .+8
        0x00, 0x02, 0x1f, 0xd6, // br x16
        0x00, 0x00, 0x00, 0x00, // addr low
        0x00, 0x00, 0x00, 0x00  // addr high
    };

    memcpy(trampoline, code, sizeof(code));
    *(void**)((char*)trampoline + 8) = replacement;

    __builtin___clear_cache((char*)trampoline, (char*)trampoline + sizeof(code));

    return trampoline;
}

bool installHook(void* target, void* replacement, void** original) {
    // Guardar original
    if (original) {
        *original = target;
    }

    // Crear trampolín
    void* trampoline = createTrampoline(target, replacement);
    if (!trampoline) {
        return false;
    }

    // Registrar hook (el parcheo real depende del entorno Zygisk/PLT)
    HookInfo info{};
    info.target = target;
    info.replacement = replacement;
    info.trampoline = trampoline;
    info.original = original ? *original : nullptr;
    g_hooks.push_back(info);

    LOGD("Hook instalado: %p -> %p", target, replacement);
    return true;
}

// Hook para JNI_OnLoad
jint hooked_JNI_OnLoad(JavaVM* vm, void* /*reserved*/) {
    LOGD("JNI_OnLoad interceptado");

    JNIEnv* env = nullptr;
    if (vm->GetEnv(reinterpret_cast<void**>(&env), JNI_VERSION_1_6) == JNI_OK) {
        // Instalar hooks adicionales
        installPlayIntegrityHooks(env);
    }

    return JNI_VERSION_1_6;
}

void hookPlayIntegrityMethods(JNIEnv* env, jclass clazz) {
    if (!env || !clazz) {
        return;
    }

    // Hook para requestIntegrityToken
    jmethodID requestToken = env->GetMethodID(
        clazz,
        "requestIntegrityToken",
        "(Lcom/google/android/gms/integrity/IntegrityTokenRequest;)Lcom/google/android/gms/tasks/Task;");

    if (requestToken) {
        LOGD("Hook para requestIntegrityToken encontrado");
        // El registro nativo real depende de Zygisk Api->hookJniNativeMethods
    }

    // Limpiar excepción si el método no existía
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
}

void installPlayIntegrityHooks(JNIEnv* env) {
    if (!env) {
        return;
    }
    LOGD("Instalando hooks específicos de Play Integrity...");

    jclass integrityClass = env->FindClass("com/google/android/gms/internal/playintegrity/zzf");
    if (env->ExceptionCheck()) {
        env->ExceptionClear();
    }
    if (integrityClass) {
        hookPlayIntegrityMethods(env, integrityClass);
        env->DeleteLocalRef(integrityClass);
    }
}

bool installJniHooks(JNIEnv* env) {
    LOGD("Instalando hooks JNI...");

    // Hook para JNI_OnLoad genérico (si está disponible)
    void* jniOnLoad = dlsym(RTLD_DEFAULT, "JNI_OnLoad");
    if (jniOnLoad) {
        installHook(jniOnLoad, (void*)hooked_JNI_OnLoad, nullptr);
    }

    // Hooks específicos de Play Integrity
    installPlayIntegrityHooks(env);

    return true;
}
