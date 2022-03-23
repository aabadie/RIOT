PKG_NAME=edge-impulse-sdk
PKG_URL=https://github.com/edgeimpulse/inferencing-sdk-cpp
PKG_VERSION=f0e62138e992e863c7447cc539f7e560c2190d7d  # SDK Release 28 Dec 2021
PKG_LICENSE=Apache 2.0

include $(RIOTBASE)/pkg/pkg.mk

CFLAGS += -Wno-cast-align
CFLAGS += -Wno-maybe-uninitialized
CFLAGS += -Wno-pedantic
CFLAGS += -Wno-unused-parameter

EI_MODULES := \
    ei-classifier \
    ei-dsp \
    ei-dsp-dct \
    ei-dsp-image \
    ei-dsp-kissfft \
    ei-tflite-c \
    ei-tflite-core-api \
    ei-tflite-kernels \
    ei-tflite-kernels-internal \
    ei-tflite-micro \
    ei-tflite-micro-kernels \
    ei-tflite-micro-memory-planner \
    #

DIR_ei-classifier                   := classifier
DIR_ei-dsp                          := dsp
DIR_ei-dsp-dct                      := dsp/dct
DIR_ei-dsp-image                    := dsp/image
DIR_ei-dsp-kissfft                  := dsp/kissfft
DIR_ei-tflite-c                     := tensorflow/lite/c
DIR_ei-tflite-core-api              := tensorflow/lite/core/api
DIR_ei-tflite-kernels               := tensorflow/lite/kernels
DIR_ei-tflite-kernels-internal      := tensorflow/lite/kernels/internal
DIR_ei-tflite-micro                 := tensorflow/lite/micro
DIR_ei-tflite-micro-kernels         := tensorflow/lite/micro/kernels
DIR_ei-tflite-micro-memory-planner  := tensorflow/lite/micro/memory_planner

EI_MODULES_USED := $(filter $(EI_MODULES),$(USEMODULE))

all: $(EI_MODULES_USED)
	@:

.PHONY: ei-%

ei-%:
	$(QQ)"$(MAKE)" -C $(PKG_SOURCE_DIR)/$(DIR_$@) -f $(CURDIR)/$@.mk
