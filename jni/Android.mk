LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/nes/h
                    
LOCAL_MODULE    := NesSimon
LOCAL_SRC_FILES := com_falcon_nesSimon_NesSimu.c \
					InfoNES_System_Android.c \
					buffersoundpool.c \
					nes/c/InfoNES.c \
					nes/c/InfoNES_pAPU.c \
					nes/c/InfoNES_Mapper.c \
					nes/c/K6502.c

LOCAL_LDLIBS := -ljnigraphics -llog -lOpenSLES

include $(BUILD_SHARED_LIBRARY)
