CC := gcc
AR := ar

CSTD := -std=c11
WARNFLAGS := -Wall -Wextra -Wpedantic
INCLUDES := -Iinclude -Isrc
LIBS := -lbcrypt

BUILD_DIR := build
OBJ_DIR := $(BUILD_DIR)/obj
LIB_DIR := $(BUILD_DIR)/lib
BIN_DIR := $(BUILD_DIR)/bin

CORE_SRC := $(wildcard src/core/*.c)
PLATFORM_SRC := $(wildcard src/platform/win/*.c)
ALL_SRC := $(CORE_SRC) $(PLATFORM_SRC)

CORE_OBJ := $(patsubst src/core/%.c,$(OBJ_DIR)/core/%.o,$(CORE_SRC))
PLATFORM_OBJ := $(patsubst src/platform/win/%.c,$(OBJ_DIR)/platform/win/%.o,$(PLATFORM_SRC))
ALL_OBJ := $(CORE_OBJ) $(PLATFORM_OBJ)

STATIC_LIB := $(LIB_DIR)/libtalos.a
SHARED_LIB := $(LIB_DIR)/talos.dll
IMPORT_LIB := $(LIB_DIR)/libtalos.dll.a

TEST_BIN := $(BIN_DIR)/test_lifecycle.exe

.PHONY: all static shared test clean

all: static shared

static: $(STATIC_LIB)

shared: $(SHARED_LIB)

$(OBJ_DIR)/core/%.o: src/core/%.c | $(OBJ_DIR)/core
	$(CC) $(CSTD) $(WARNFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/platform/win/%.o: src/platform/win/%.c | $(OBJ_DIR)/platform/win
	$(CC) $(CSTD) $(WARNFLAGS) $(INCLUDES) -c $< -o $@

$(STATIC_LIB): $(ALL_OBJ) | $(LIB_DIR)
	$(AR) rcs $@ $^

$(SHARED_LIB): $(ALL_SRC) | $(LIB_DIR)
	$(CC) $(CSTD) $(WARNFLAGS) $(INCLUDES) -DTALOS_BUILD_SHARED -shared \
		-o $@ $(ALL_SRC) -Wl,--out-implib,$(IMPORT_LIB) $(LIBS)

test: $(TEST_BIN)
	$(TEST_BIN)

$(TEST_BIN): tests/test_lifecycle.c $(STATIC_LIB) | $(BIN_DIR)
	$(CC) $(CSTD) $(WARNFLAGS) $(INCLUDES) $< $(STATIC_LIB) -o $@ $(LIBS)

$(OBJ_DIR)/core $(OBJ_DIR)/platform/win $(LIB_DIR) $(BIN_DIR):
	mkdir -p $@

clean:
	rm -rf $(BUILD_DIR)
