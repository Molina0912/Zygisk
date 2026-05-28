#include <android/log.h>
#include <sys/system_properties.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <jni.h>
#include "zygisk.hpp"
#include "hooks.h"
#include "integrity_spoof.h"
#include "device_spoof.h"
#include "utils.h"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PIB", __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO,  "PIB", __VA_ARGS__)

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

class PlayIntegrityModule : public zygisk::ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGI("Módulo Play Integrity cargado");
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        const char *process = nullptr;
        if (args && args->nice_name) {
            process = env->GetStringUTFChars(args->nice_name, nullptr);
        }

        // Solo aplicar el bypass a procesos relevantes; en el resto
        // descargamos la lib para minimizar la huella.
        bool target = false;
        if (process) {
            if (strcmp(process, "com.google.android.gms.unstable") == 0 ||
                strcmp(process, "com.google.android.gms") == 0 ||
                strcmp(process, "com.android.vending") == 0) {
                target = true;
            }
        }

        if (target) {
            LOGI("Aplicando bypass a: %s", process ? process : "?");
            hideMagiskTraces();
            spoofDeviceProperties();
        } else {
            api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
        }

        if (process && args && args->nice_name) {
            env->ReleaseStringUTFChars(args->nice_name, process);
        }
    }

    void postAppSpecialize(const AppSpecializeArgs * /*args*/) override {
        // En el contexto post-specialize ya tenemos los permisos del proceso destino.
        installJniHooks(env);
    }

    void preServerSpecialize(ServerSpecializeArgs * /*args*/) override {
        // No nos interesa system_server para este módulo.
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

private:
    Api *api = nullptr;
    JNIEnv *env = nullptr;

    void hideMagiskTraces() {
        hideMagiskFiles();
        hideMagiskProcesses();
        cleanEnvironment();

        __system_property_set("ro.magisk.disable", "true");
        __system_property_set("persist.magisk.disable", "true");
    }
};

REGISTER_ZYGISK_MODULE(PlayIntegrityModule)
