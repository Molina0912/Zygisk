#ifndef ZYGISK_HPP
#define ZYGISK_HPP

#include <jni.h>

namespace zygisk {

class ModuleBase {
public:
    struct Api;
    struct AppSpecializeArgs;
    struct ServerSpecializeArgs;
    
    virtual void onLoad(Api* api, JNIEnv* env) {}
    virtual void preAppSpecialize(AppSpecializeArgs* args) {}
    virtual void postAppSpecialize(const AppSpecializeArgs* args) {}
    virtual void preServerSpecialize(ServerSpecializeArgs* args) {}
    virtual void postServerSpecialize(const ServerSpecializeArgs* args) {}
};

struct Api {
    void* impl;
    
    void (*hookJniNativeMethods)(JNIEnv*, const char*, JNINativeMethod*, int);
    void (*pltHookRegister)(const char*, const char*, void*, void**);
    void (*pltHookCommit)();
};

struct AppSpecializeArgs {
    jint& uid;
    jint& gid;
    jintArray& gids;
    jint& runtime_flags;
    jobjectArray& rlimits;
    jint& mount_external_mode;
    jstring& se_info;
    jstring& nice_name;
    jstring& instruction_set;
    jstring& app_data_dir;
    jstring& package_name;
    jobjectArray& packages_to_truncate;
};

struct ServerSpecializeArgs {
    jint& uid;
    jint& gid;
    jintArray& gids;
    jint& runtime_flags;
    jlong& permitted_capabilities;
    jlong& effective_capabilities;
};

#define REGISTER_ZYGISK_MODULE(className) \
    extern "C" [[gnu::visibility("default")]] void zygisk_module_entry(zygisk::Api* api, JNIEnv* env) { \
        static className module; \
        module.onLoad(api, env); \
    }

} // namespace zygisk

#endif // ZYGISK_HPP
