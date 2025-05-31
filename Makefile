UNAME:=$(shell uname -s)
PROG_EXT=
CFLAGS=-x objective-c -DSOKOL_METAL -fobjc-arc -framework Metal -framework Cocoa -framework MetalKit -framework Quartz -framework AudioToolbox
ARCH:=$(shell uname -m)
ifeq ($(ARCH),arm64)
	ARCH=osx_arm64
else
	ARCH=osx
endif
SHDC_FLAGS=metal_macos

SOURCE=$(wildcard src/*.c)
SCENES=$(wildcard scenes/*.c)
EXE=build/jeff_$(ARCH)$(PROG_EXT)
INC=-Isrc -Iscenes -Ibuild -Ideps -Ideps/cimgu

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

cimgui:
	$(CC)++ -shared -fpic -Ideps/cimgui deps/cimgui/*.cpp deps/cimgui/imgui/*.cpp -o build/libcimgui.dylib

app: cimgui shaders
	$(CC) $(INC) $(CFLAGS) $(SOURCE) $(SCENES) -Lbuild -lcimgui -o $(EXE)

run: $(EXE)
	./$(EXE)

.PHONY: all app cimgui shaders run
