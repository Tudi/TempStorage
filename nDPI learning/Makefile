# Project settings
BIN_DIR := bin
TARGETS := $(BIN_DIR)/flow_inspector $(BIN_DIR)/db_gen
SRC_DIR := src
OBJ_DIR := build

# Compiler and flags
CC := gcc
CFLAGS := -Wall -Wextra -Werror -Wno-stringop-truncation -Wno-unused-parameter -Wno-error=stringop-truncation -Wno-error=unused-parameter -Wno-error=unused-variable -O2

DEBUG_CFLAGS := -g -O0 -D_USE_BOUNDS_CHECKING -D_DEBUG -DDEBUG  -fno-inline -fno-omit-frame-pointer -fno-optimize-sibling-calls -rdynamic -fvisibility=default -Wl,--export-dynamic
DEBUG_LDFLAGS := -fsanitize=address,undefined

# Normalize lowercase 'debug' to uppercase 'DEBUG'
ifeq ($(origin debug), command line)
  DEBUG := $(debug)
endif
ifdef DEBUG
CFLAGS += $(DEBUG_CFLAGS)
LDFLAGS += $(DEBUG_LDFLAGS)
endif
# release specific flags
ifndef ENABLE_ALL_NDPI_FEATURES
CFLAGS += -D_RELEASE_BUILD
endif
ifdef ENABLE_ALL_NDPI_FEATURES
CFLAGS += -D_FLOW_SERIALIZER
CFLAGS += -DENABLE_HUMAN_READABLE_STRING_EXTRACTION
CFLAGS += -DMAINTAIN_FLOW_PAYLOAD_COPY
CFLAGS += -DENABLE_FLOW_DATA_ANALYSIS
CFLAGS += -DTCP_FLAG_STATISTICS
CFLAGS += -DENABLE_FLOW_STATS_GENERATION
CFLAGS += -DENABLE_FLOW_DISCOVER_CALLBACK
CFLAGS += -DCOPY_FLOW_INFO_TO_FLOW
CFLAGS += -DENABLE_ALL_NDPI_FEATURES
CFLAGS += -DTLS_BROWSER_HEURISTICS
CFLAGS += -DTLS_NOT_USED_FIELDS
CFLAGS += -DSTORE_CONFIDENCE_SCORE
CFLAGS += -DCALCULATE_ENTROPY_SCORE
CFLAGS += -DNDI_USE_BINS
CFLAGS += -DSTORE_HTTP_INFO
CFLAGS += -DSTORE_STUN_INFO
endif

ifdef debug_stat_allocs
CFLAGS += $(DEBUG_CFLAGS) -D_STAT_MEM_ALLOCS
LDFLAGS += $(DEBUG_LDFLAGS)
endif

# Include directories
INCLUDES := -I./dependencies/nDPI/src/include -I./dependencies/nDPI/src/lib -I./dependencies/uthash

# Libraries to link 
LIBS := -lpcap -lssl -lcrypto -lndpi -lm -lmysqlclient
LIB_DIRS := -L./dependencies/nDPI/src/lib -Wl,-rpath=./dependencies/nDPI/src/lib

# Source and object files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
FLOW_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/flow_%.o,$(SRCS))
DBGEN_OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/dbgen_%.o,$(SRCS))

# Default rule
all: $(TARGETS)

# Link object files into binary
$(BIN_DIR)/flow_inspector: $(FLOW_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIB_DIRS) $(FLOW_OBJS) -o $@ $(LIBS)
$(BIN_DIR)/db_gen: $(DBGEN_OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(LIB_DIRS) $(DBGEN_OBJS) -o $@ $(LIBS)

# Compile each source file into object file
$(OBJ_DIR)/flow_%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/dbgen_%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -D_APP_MODE_BUILD_DB $(INCLUDES) -c $< -o $@

# Clean build outputs
clean:
	rm -rf $(OBJ_DIR) $(TARGETS)

.PHONY: all clean


# --------------------------
# Test build configuration
# --------------------------

TEST_CFLAGS = -g -O0 -Wall -Wextra -Werror -D_USE_BOUNDS_CHECKING -DDEBUG			  
TEST_LDFLAGS = -fsanitize=address,undefined		  
TEST_DIR := tests
TEST_SRC := $(wildcard $(TEST_DIR)/*.c)
TEST_OBJ := $(patsubst $(TEST_DIR)/%.c,$(OBJ_DIR)/%.test.o,$(TEST_SRC))
TEST_BIN := bin/test_runner

TEST_LIBS := -lcmocka -lpcap -lssl -lcrypto -lndpi
TEST_LIB_DIRS := -L./dependencies/nDPI/src/lib -Wl,-rpath=./dependencies/nDPI/src/lib
TEST_INCLUDES := -I$(TEST_DIR) -I./include -I./dependencies/nDPI/src/include

.PHONY: tests
tests: $(TEST_BIN)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TEST_BIN)

$(TEST_BIN): $(TEST_OBJ) $(filter-out build/flow_inspector.o,$(OBJS))
	@mkdir -p $(dir $@)
	$(CC) $(TEST_CFLAGS) $(TEST_OBJ) $(filter-out build/flow_inspector.o,$(OBJS)) -o $@ $(TEST_LIB_DIRS) $(TEST_LIBS)

$(OBJ_DIR)/%.test.o: $(TEST_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(TEST_CFLAGS) $(TEST_INCLUDES) -c $< -o $@
