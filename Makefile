# settings here:

BUILD_MODE?=DEBUG
PLATFORM?=PLATFORM_DESKTOP

ifeq ($(PLATFORM),PLATFORM_DESKTOP)
    CC?=gcc
    CXX?=g++
	AR:=ar
endif
ifeq ($(PLATFORM),PLATFORM_WEB)
    CC:=emcc
    CXX:=em++
	AR:=emar
endif
CFLAGS:=-Wall -Wextra -Wpedantic -Wno-narrowing $(CUSTOM_CFLAGS)
CXXFLAGS:=-Wall -Wextra -Wpedantic -Wno-narrowing $(CUSTOM_CXXFLAGS)
CSTD:=-std=c99
CXXSTD:=-std=c++17
RELEASE_OPTIM?= -O3 -flto

SRC_DIR         ?=src/
BUILD_DIR       ?=build/make/$(PLATFORM)_$(BUILD_MODE)/
OBJ_DIR         ?=$(BUILD_DIR)objs/Arduboy_Emulator_HL/
DEPENDENCIES_DIR?=dependencies/

OUT_NAME?=libArduboy_Emulator_HL.a
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

DEF_FLAGS:=

BUILD_MODE_FLAGS:=
ifeq ($(BUILD_MODE),DEBUG)
	BUILD_MODE_FLAGS +=-g
	DEF_FLAGS += -D_DEBUG
else
	BUILD_MODE_FLAGS +=$(RELEASE_OPTIM)
endif

DEP_FLAGS=-MMD -MF ${@:.o=.d}

OUT_PATH:=$(OUT_DIR)$(OUT_NAME)

SRC_FILES:=$(shell find $(SRC_DIR) -name '*.cpp')
OBJ_FILES:=$(addprefix $(OBJ_DIR),${SRC_FILES:.cpp=.o})
DEP_FILES:=$(patsubst %.o,%.d,$(OBJ_FILES))

DEPENDENCIES_INCLUDE_PATHS:=dependencies/ATmega32u4_Emulator/src dependencies/ATmega32u4_Emulator/dependencies/CPP_Utils/src
DEPENDENCIES_LIBS_DIR:=$(BUILD_DIR)dependencies/libs

DEP_LIBS_DIRS:=$(addprefix $(DEPENDENCIES_DIR),ATmega32u4_Emulator/)

DEP_INCLUDE_FLAGS:=$(addprefix -I,$(DEPENDENCIES_INCLUDE_PATHS))
DEP_BUILD_DIR:=$(BUILD_DIR)


CFLAGS += $(BUILD_MODE_FLAGS)
CXXFLAGS += $(BUILD_MODE_FLAGS)

# rules:

.PHONY:all clean

all: $(OUT_PATH)

$(OUT_PATH): deps $(OBJ_FILES)
	# BUILDING Arduboy_Emulator_HL
	mkdir -p $(OUT_DIR)
	$(AR) rvs $@ $(OBJ_FILES)

$(OBJ_DIR)%.o:%.cpp
	mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(CXXSTD) $(DEP_INCLUDE_FLAGS) -c $< -o $@ $(DEP_FLAGS)

-include $(DEP_FILES)

deps:
	$(MAKE) -C $(DEP_LIBS_DIRS) PLATFORM=$(PLATFORM) BUILD_MODE=$(BUILD_MODE) RELEASE_OPTIM=$(RELEASE_OPTIM) BUILD_DIR=$(DEP_BUILD_DIR) CUSTOM_CFLAGS="$(CUSTOM_CFLAGS)" CUSTOM_CXXFLAGS="$(CUSTOM_CXXFLAGS)" CSTD="$(CSTD)" CXXSTD="$(CXXSTD)"

clean:
	$(MAKE) -C $(DEP_LIBS_DIRS) clean BUILD_DIR=$(DEP_BUILD_DIR)
	rm -rf $(BUILD_DIR)