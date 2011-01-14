LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_PRELINK_MODULE := false
LOCAL_SRC_FILES := \
main.c \
event_handler.c \
event_dev.c \
uinput.c \
ballistics.c \
initialization.c \
process.c \
recognizer.c \
statemachine.c \
socketserver.c
 

LOCAL_CFLAGS := \
    -std=gnu99 -pedantic
# LOCAL_SHARED_LIBRARIES := libconfig libsvm libdbus

PRODUCT_COPY_FILES += \
       $(LOCAL_PATH)/excluded-input-devices.xml:system/etc/excluded-input-devices.xml
#        $(LOCAL_PATH)/example.model:system/etc/example.model \
        $(LOCAL_PATH)/unipoint.conf:system/etc/unipoint.conf \

LOCAL_C_INCLUDES := \
        $(LOCAL_PATH)/include  
#       $(call include-path-for, dbus) \
        $(call include-path-for, dbus)/dbus \
        external/libsvm 
LOCAL_MODULE := unipoint-daemon
include $(BUILD_EXECUTABLE)

include $(call all-makefiles-under,$(LOCAL_PATH))

# Build the demo app
#include $(LOCAL_PATH)/UnipointDemo/Android.mk

# Build the Unipoint Java service
#include $(LOCAL_PATH)/UnipointJavaService/Android.mk
