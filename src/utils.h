#ifndef UTILS_H
#define UTILS_H

#include <jni.h>
#include <dirent.h>
#include <sys/mount.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

bool hideMagiskFiles();
bool hideMagiskProcesses();
bool unmountMagiskPaths();
void cleanEnvironment();
bool isTargetApp(const char* packageName);
jclass findLoadedClass(JNIEnv* env, const char* className);

#endif // UTILS_H
