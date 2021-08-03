# settings here:

BUILD_MODE?=DEBUG

CC    :=g++
CFLAGS?=-Wall -Wno-narrowing
CSTD  ?=-std=c++17
RELEASE_OPTIM?= -O2

SRC_DIR         ?=src/
BUILD_DIR       ?=build/make/
OBJ_DIR         ?=$(BUILD_DIR)objs/Arduboy_Emulator_HL/
DEPENDENCIES_DIR?=dependencies/

OUT_NAME?=Arduboy_Emulator_HL.a
OUT_DIR ?=$(BUILD_DIR)

# you dont need to worry about this stuff:

# detect OS
ifeq ($(OS),Windows_NT) 
    detected_OS := Windows
else
    detected_OS := $(shell sh -c 'uname 2>/dev/null || echo Unknown')
endif

# get current dir
current_dir :=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

BUILD_MODE_CFLAGS:=
ifeq ($(BUILD_MODE),DEBUG)
	BUILD_MODE_CFLAGS += -g
else
	BUILD_MODE_CFLAGS +=$(RELEASE_OPTIM)
endif

MAKE_CMD:=make
ifeq ($(detected_OS),Windows)
	MAKE_CMD:=mingw32-make
	BASH_PREFX:=bash -c 
endif

CDEPFLAGS=-MMD -MF ${@:.o=.d}

OUT_PATH:=$(OUT_DIR)$(OUT_NAME)

SRC_FILES:=$(shell $(BASH_PREFX)"find $(SRC_DIR) -name '*.cpp'")
OBJ_FILES:=$(addprefix $(OBJ_DIR),${SRC_FILES:.cpp=.o})
DEP_FILES:=$(patsubst %.o,%.d,$(OBJ_FILES))

DEPENDENCIES_INCLUDE_PATHS:=dependencies/ATmega32u4_Emulator/src
DEPENDENCIES_LIBS_DIR:=$(BUILD_DIR)dependencies/libs

DEP_LIBS_DIRS:=$(addprefix $(DEPENDENCIES_DIR),ATmega32u4_Emulator/)

DEP_INCLUDE_FLAGS:=$(addprefix -I,$(DEPENDENCIES_INCLUDE_PATHS))
DEP_BUILD_DIR:=$(BUILD_DIR)

# rules:

.PHONY:all clean

all: $(OUT_PATH)

$(OUT_PATH): deps $(OBJ_FILES)
	# BUILDING Arduboy_Emulator_HL
	$(BASH_PREFX)"mkdir -p $(OUT_DIR)"
	ar rvs $@ $(OBJ_FILES)

$(OBJ_DIR)%.o:%.cpp
	$(BASH_PREFX)"mkdir -p $(dir $@)"
	$(CC) $(CFLAGS) $(CSTD) $(BUILD_MODE_CFLAGS) $(DEP_INCLUDE_FLAGS) -c $< -o $@ $(CDEPFLAGS)

-include $(DEP_FILES)

deps:
	$(MAKE_CMD) -C $(DEP_LIBS_DIRS) BUILD_MODE=$(BUILD_MODE) CC=$(CC) CFLAGS="$(CFLAGS)" CSTD=$(CSTD) RELEASE_OPTIM=$(RELEASE_OPTIM) BUILD_DIR=$(DEP_BUILD_DIR)

clean:
	$(MAKE_CMD) -C $(DEP_LIBS_DIRS) clean BUILD_DIR=$(DEP_BUILD_DIR)
	rm -rf $(BUILD_DIR)