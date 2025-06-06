UNAME:=$(shell uname -s)
PROG_EXT=
LIB_EXT=dylib
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

paul:
	$(CC) -shared -fpic -x objective-c -fno-objc-arc -Ideps/paul deps/paul/native/macos/*.m -framework Cocoa -framework IOKit -o build/libpaul.$(LIB_EXT)

app: paul shaders
	$(CC) $(INC) $(CFLAGS) $(SOURCE) $(SCENES) -Lbuild -lcimgui -lpaul -o $(EXE)

run: $(EXE)
	./$(EXE)

.PHONY: all app shaders run
