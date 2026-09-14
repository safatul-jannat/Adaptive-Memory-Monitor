CC      = gcc
CFLAGS  = -Wall -Wextra -O2 -std=c11

SRC_DIR   = src
BUILD_DIR = build
TEST_DIR  = tests
TEST_BIN  = tests/bin

MONITOR_SRC = \
    $(SRC_DIR)/main.c \
    $(SRC_DIR)/process_discovery.c \
    $(SRC_DIR)/proc_reader.c \
    $(SRC_DIR)/logger.c \
    $(SRC_DIR)/history.c \
    $(SRC_DIR)/analyzer.c \
    $(SRC_DIR)/risk_engine.c \
    $(SRC_DIR)/scheduler.c

LIB_SRC = \
    $(SRC_DIR)/process_discovery.c \
    $(SRC_DIR)/proc_reader.c \
    $(SRC_DIR)/logger.c \
    $(SRC_DIR)/history.c \
    $(SRC_DIR)/analyzer.c \
    $(SRC_DIR)/risk_engine.c \
    $(SRC_DIR)/scheduler.c

LIB_OBJ = $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(LIB_SRC))

TEST_SRC = $(wildcard $(TEST_DIR)/*.c)
TEST_BIN_SRC = $(patsubst $(TEST_DIR)/%.c,$(TEST_BIN)/%,$(TEST_SRC))

WORKLOAD_SRC = $(wildcard workloads/*.c)
WORKLOAD_BIN = $(patsubst workloads/%.c,workloads/bin/%,$(WORKLOAD_SRC))

.PHONY: all monitor workloads tests test clean

all: monitor

monitor: monitor.o $(LIB_OBJ)
	$(CC) $(CFLAGS) $^ -o $@

monitor.o: $(SRC_DIR)/main.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) $(CFLAGS) -I. -c $< -o $@

workloads: $(WORKLOAD_BIN)

workloads/bin/%: workloads/%.c
	@mkdir -p workloads/bin
	$(CC) $(CFLAGS) $< -o $@

tests: $(TEST_BIN_SRC)

$(TEST_BIN)/%: $(TEST_DIR)/%.c $(LIB_OBJ)
	@mkdir -p $(TEST_BIN)
	$(CC) $(CFLAGS) -I. $< $(LIB_OBJ) -o $@

test: tests
	@for t in $(TEST_BIN_SRC); do \
		echo "=== $$t ==="; \
		$$t || exit 1; \
	done
	@echo "ALL TESTS PASS"

clean:
	rm -rf build tests/bin workloads/bin
	rm -f monitor monitor.o
