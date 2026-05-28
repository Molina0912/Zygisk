#include "utils.h"
#include <android/log.h>
#include <fstream>
#include <string>
#include <cstdio>
#include <cstring>

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "PIB-Utils", __VA_ARGS__)

bool hideMagiskFiles() {
    const char* paths[] = {
        "/data/adb/magisk",
        "/sbin/.magisk",
        "/dev/.magisk.unblock",
        "/system/bin/magisk",
        "/system/xbin/su",
        "/data/adb/modules",
        "/data/adb/post-fs-data.d",
        "/data/adb/service.d"
    };

    for (const auto& path : paths) {
        if (access(path, F_OK) == 0) {
            LOGD("Ocultando: %s", path);
            // El bind mount real lo realiza service.sh; aquí solo se registra.
        }
    }
    return true;
}

bool hideMagiskProcesses() {
    DIR* proc = opendir("/proc");
    if (!proc) return false;

    struct dirent* entry;
    while ((entry = readdir(proc)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {
                char cmdline[256];
                std::snprintf(cmdline, sizeof(cmdline), "/proc/%d/cmdline", pid);

                std::ifstream file(cmdline);
                std::string processName;
                if (std::getline(file, processName)) {
                    if (processName.find("magisk") != std::string::npos ||
                        processName.find("su") != std::string::npos) {
                        LOGD("Ocultando proceso: %d (%s)", pid, processName.c_str());
                        // Ocultar proceso (bind mount /dev/null sobre cmdline en service.sh)
                    }
                }
            }
        }
    }

    closedir(proc);
    return true;
}

bool unmountMagiskPaths() {
    const char* mounts[] = {
        "/system/bin",
        "/system/xbin",
        "/system/app",
        "/vendor/bin"
    };

    for (const auto& mount : mounts) {
        umount2(mount, MNT_DETACH);
    }

    return true;
}

void cleanEnvironment() {
    unsetenv("MAGISK_VER");
    unsetenv("MAGISK_VER_CODE");
    unsetenv("MAGISK_ALLOW_APK");
    unsetenv("MAGISK_PATH");
}

bool isTargetApp(const char* packageName) {
    if (!packageName) return false;

    static const char* targets[] = {
        "gr.nikolasspyr.integritycheck",
        "com.google.android.gms",
        "com.android.vending",
    };

    for (const auto& target : targets) {
        if (std::strcmp(packageName, target) == 0) {
            return true;
        }
    }
    return false;
}

jclass findLoadedClass(JNIEnv* env, const char* className) {
    if (!env || !className) return nullptr;

    jclass classLoader = env->FindClass("java/lang/ClassLoader");
    if (!classLoader) return nullptr;

    jmethodID getSystemClassLoader = env->GetStaticMethodID(
        classLoader, "getSystemClassLoader", "()Ljava/lang/ClassLoader;");
    if (!getSystemClassLoader) {
        env->DeleteLocalRef(classLoader);
        return nullptr;
    }

    jobject sysLoader = env->CallStaticObjectMethod(classLoader, getSystemClassLoader);
    if (!sysLoader) {
        env->DeleteLocalRef(classLoader);
        return nullptr;
    }

    jmethodID loadClass = env->GetMethodID(classLoader, "loadClass",
                                           "(Ljava/lang/String;)Ljava/lang/Class;");
    if (!loadClass) {
        env->DeleteLocalRef(sysLoader);
        env->DeleteLocalRef(classLoader);
        return nullptr;
    }

    jstring name = env->NewStringUTF(className);
    jclass clazz = (jclass)env->CallObjectMethod(sysLoader, loadClass, name);
    if (env->ExceptionCheck()) env->ExceptionClear();

    env->DeleteLocalRef(name);
    env->DeleteLocalRef(sysLoader);
    env->DeleteLocalRef(classLoader);

    return clazz;
}
