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

SOURCE=$(wildcard src/*.c) $(wildcard deps/paul/native/macos/*.m) deps/gamepad/Gamepad_private.c deps/gamepad/Gamepad_macosx.c
SCENES=$(wildcard scenes/*.c)
EXE=build/jeff_$(ARCH)$(PROG_EXT)
INC=-Isrc -Iscenes -Ibuild -Lbuild -Ideps -Ideps/paul

ARCH_PATH=./bin/$(ARCH)
SHDC_PATH=$(ARCH_PATH)/sokol-shdc$(PROG_EXT)
SHADERS=$(wildcard assets/*.glsl)
SHADER_OUTS=$(patsubst assets/%,build/%.h,$(SHADERS))

all: app

.SECONDEXPANSION:
SHADER=$(patsubst build/%.h,assets/%,$@)
SHADER_OUT=$@
%.glsl.h: $(SHADERS)
	$(SHDC_PATH) -i $(SHADER) -o $(SHADER_OUT) -l $(SHDC_FLAGS)

shaders: $(SHADER_OUTS)

app: shaders
	$(CC) $(INC) $(CFLAGS) $(SOURCE) $(SCENES) -o $(EXE)

default: app

.PHONY: default all app shaders
