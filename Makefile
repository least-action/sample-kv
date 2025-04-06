SRC_DIR = src
BUILD_DIR = build
TEST_DIR = test
TARGET = a.out

CC = gcc
CFLAGS = -std=c99 -Wall -O2 -pthread -I$(SRC_DIR) -D_POSIX_C_SOURCE=200112L
OBJS = main.o

SRC_FILES = $(shell find $(SRC_DIR) -type f -name "*.c")
OBJ_FILES = $(patsubst $(SRC_DIR)/%, $(BUILD_DIR)/%, $(SRC_FILES:.c=.o))

.PHONY: test

all: $(TARGET)

$(TARGET): $(OBJ_FILES)
	mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

test:
	$(TEST_DIR)/run_test $(shell pwd) $(TARGET) $(TEST_DIR)

clean:
	rm -rf $(BUILD_DIR) $(TARGET) $(TEST_DIR)/__pycache__

