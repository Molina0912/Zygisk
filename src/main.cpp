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

using namespace zygisk;

class PlayIntegrityModule : public ModuleBase {
public:
    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
        LOGD("Módulo Play Integrity cargado");
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Ocultar rastros antes de que la app se especialice
        hideMagiskTraces();
        spoofDeviceProperties();
    }

    void postAppSpecialize(const AppSpecializeArgs *args) override {
        // Instalar hooks después de la especialización
        installJniHooks(env);
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override {
        // Configuración para proceso de sistema
        hideMagiskTraces();
    }

private:
    Api *api = nullptr;
    JNIEnv *env = nullptr;

    void hideMagiskTraces() {
        // Delegar a las utilidades comunes
        hideMagiskFiles();
        hideMagiskProcesses();
        cleanEnvironment();

        // Ocultar propiedades adicionales
        __system_property_set("ro.magisk.disable", "true");
        __system_property_set("persist.magisk.disable", "true");
    }
};

REGISTER_ZYGISK_MODULE(PlayIntegrityModule)
