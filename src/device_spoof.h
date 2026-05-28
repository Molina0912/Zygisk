#ifndef DEVICE_SPOOF_H
#define DEVICE_SPOOF_H

struct DeviceProfile {
    const char* manufacturer;
    const char* model;
    const char* device;
    const char* product;
    const char* brand;
    const char* hardware;
    const char* board;
    const char* bootloader;
    const char* fingerprint;
    const char* securityPatch;
    int sdkInt;
    int firstApiLevel;
    const char* verifiedBootState;
    const char* vbmetaState;
};

void spoofDeviceProperties();

// Hooks
int hooked_system_property_get(const char* name, char* value);
bool hooked_isDeviceRooted();
bool hooked_isEmulator();
bool hooked_isUsbDebuggingEnabled();
int hooked_attestationCheck(const char* nonce, const char* packageName, 
                             char* result, size_t resultLen);

#endif // DEVICE_SPOOF_H
