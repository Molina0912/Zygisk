LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := playintegrity
LOCAL_SRC_FILES := main.cpp \
                   hooks.cpp \
                   integrity_spoof.cpp \
                   device_spoof.cpp \
                   utils.cpp
LOCAL_LDLIBS    := -llog -landroid
LOCAL_CFLAGS    := -O2 -fvisibility=hidden -fPIC -Wno-unused-parameter
LOCAL_CPPFLAGS  := -std=c++20
LOCAL_C_INCLUDES := $(LOCAL_PATH)

include $(BUILD_SHARED_LIBRARY)
