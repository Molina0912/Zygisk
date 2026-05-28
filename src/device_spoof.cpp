#include "device_spoof.h"
#include <android/log.h>
#include <sys/system_properties.h>
#include <cstring>
#include <string>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PIB-Device", __VA_ARGS__)

// Propiedades del dispositivo a spoofear
static DeviceProfile certifiedDevice = {
    /* manufacturer       */ "Google",
    /* model              */ "Pixel 8",
    /* device             */ "shiba",
    /* product            */ "shiba",
    /* brand              */ "google",
    /* hardware           */ "shiba",
    /* board              */ "shiba",
    /* bootloader         */ "shiba-1.0-12345678",
    /* fingerprint        */ "google/shiba/shiba:14/UQ1A.231205.015/12345678:user/release-keys",
    /* securityPatch      */ "2024-01-05",
    /* sdkInt             */ 34,
    /* firstApiLevel      */ 32,
    /* verifiedBootState  */ "green",
    /* vbmetaState        */ "locked"
};

void spoofDeviceProperties() {
    LOGD("Spoofeando propiedades del dispositivo");

    // Propiedades de build
    __system_property_set("ro.product.manufacturer", certifiedDevice.manufacturer);
    __system_property_set("ro.product.model", certifiedDevice.model);
    __system_property_set("ro.product.device", certifiedDevice.device);
    __system_property_set("ro.product.name", certifiedDevice.product);
    __system_property_set("ro.product.brand", certifiedDevice.brand);
    __system_property_set("ro.product.board", certifiedDevice.board);

    // Propiedades de boot
    __system_property_set("ro.bootloader", certifiedDevice.bootloader);
    __system_property_set("ro.boot.verifiedbootstate", certifiedDevice.verifiedBootState);
    __system_property_set("ro.boot.vbmeta.device_state", certifiedDevice.vbmetaState);
    __system_property_set("ro.boot.veritymode", "enforcing");

    // Propiedades de seguridad
    __system_property_set("ro.build.version.security_patch", certifiedDevice.securityPatch);
    __system_property_set("ro.build.version.sdk", std::to_string(certifiedDevice.sdkInt).c_str());
    __system_property_set("ro.product.first_api_level",
                          std::to_string(certifiedDevice.firstApiLevel).c_str());

    // Propiedades de fingerprint
    __system_property_set("ro.build.fingerprint", certifiedDevice.fingerprint);
    __system_property_set("ro.build.tags", "release-keys");
    __system_property_set("ro.build.type", "user");
    __system_property_set("ro.build.keys", "release-keys");

    LOGD("Propiedades spoofeadas correctamente");
}

// Hook para __system_property_get
int hooked_system_property_get(const char* name, char* value) {
    if (!name || !value) {
        return __system_property_get(name, value);
    }

    static const char* spoofedProps[] = {
        "ro.boot.verifiedbootstate",
        "ro.boot.vbmeta.device_state",
        "ro.boot.veritymode",
        "ro.secure",
        "ro.debuggable",
        "ro.build.tags",
        "ro.build.type"
    };

    for (const auto& prop : spoofedProps) {
        if (std::strcmp(name, prop) == 0) {
            if (std::strstr(prop, "verifiedbootstate")) {
                std::strcpy(value, "green");
            } else if (std::strstr(prop, "vbmeta")) {
                std::strcpy(value, "locked");
            } else if (std::strstr(prop, "veritymode")) {
                std::strcpy(value, "enforcing");
            } else if (std::strstr(prop, "secure")) {
                std::strcpy(value, "1");
            } else if (std::strstr(prop, "debuggable")) {
                std::strcpy(value, "0");
            } else if (std::strstr(prop, "tags")) {
                std::strcpy(value, "release-keys");
            } else if (std::strstr(prop, "type")) {
                std::strcpy(value, "user");
            }
            return (int)std::strlen(value);
        }
    }

    return __system_property_get(name, value);
}

bool hooked_isDeviceRooted() {
    LOGD("Interceptada verificación de root");
    return false;
}

bool hooked_isEmulator() {
    LOGD("Interceptada verificación de emulador");
    return false;
}

bool hooked_isUsbDebuggingEnabled() {
    LOGD("Interceptada verificación de USB debugging");
    return false;
}

int hooked_attestationCheck(const char* /*nonce*/, const char* /*packageName*/,
                            char* result, size_t resultLen) {
    LOGD("Interceptada verificación de atestación");
    if (!result || resultLen == 0) {
        return -1;
    }

    const char* spoofedResult =
        "{\"basicIntegrity\":true,\"ctsProfileMatch\":true,"
        "\"evaluationType\":\"BASIC\",\"timestampMs\":1234567890}";

    std::strncpy(result, spoofedResult, resultLen - 1);
    result[resultLen - 1] = '\0';
    return 0;
}
