# This will be added to the main integrated/jni/Android.mk file by the build tool.

include $(CLEAR_VARS)
LOCAL_MODULE := libopenal
LOCAL_SRC_FILES := libopenal.so
include $(PREBUILT_SHARED_LIBRARY)
