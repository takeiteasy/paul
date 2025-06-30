UNAME:=$(shell uname -s)
PROG_EXT=
LIB_EXT=dylib
CFLAGS=-x objective-c -DSOKOL_METAL -fno-objc-arc -framework Metal -framework Cocoa -framework IOKit -framework MetalKit -framework Quartz -framework AudioToolbox
ARCH:=$(shell uname -m)
ifeq ($(ARCH),arm64)
	ARCH=osx_arm64
else
	ARCH=osx
endif
SHDC_FLAGS=metal_macos

SOURCE=$(wildcard src/*.c) $(wildcard deps/hal/native/macos/*.m)
SCENES=$(wildcard scenes/*.c)
EXE=build/paul_$(ARCH)$(PROG_EXT)
LIB=build/libpaul_$(ARCH).$(LIB_EXT)
INC=-Isrc -Iscenes -Ibuild -Lbuild -Ideps -Ideps/hal

ARCH_PATH=./bin/$(ARCH)
SHDC_PATH=$(ARCH_PATH)/sokol-shdc$(PROG_EXT)
SHADERS=$(wildcard shaders/*.glsl)
SHADER_OUTS=$(patsubst shaders/%,src/%.h,$(SHADERS))

all: app library

.SECONDEXPANSION:
SHADER=$(patsubst src/%.h,shaders/%,$@)
SHADER_OUT=$@
%.glsl.h: $(SHADERS)
	$(SHDC_PATH) -i $(SHADER) -o $(SHADER_OUT) -l $(SHDC_FLAGS)

shaders: $(SHADER_OUTS)

app: shaders
	$(CC) $(INC) $(CFLAGS) $(SOURCE) $(SCENES) -o $(EXE)

library: shaders
	$(CC) -shared -fpic -DPAUL_NO_SCENES $(INC) $(CFLAGS) $(SOURCE) -o $(LIB)

default: app

.PHONY: default all app library shaders
