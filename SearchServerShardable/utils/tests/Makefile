.PHONY: all clean

PROJECT_ROOT:=../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

# CMocka tests

CMOCKA_TEST_BIN:=$(BIN_DIR)/utils_tests

CMOCKA_TEST_SRCS:=date_time_tests.c int32_binary_heap_tests.c int32_list_tests.c \
  mt_queue_single_thread_tests.c bitfield_tests.c \
  boolean_parser_tests.c tests_main.c

CMOCKA_TEST_OBJS:=$(addprefix $(OBJ_DIR)/, $(CMOCKA_TEST_SRCS:%.c=%.o))

# MtQueue multiple threads tests

TEST_MTQUEUE_MULTITHREAD_BIN:=$(BIN_DIR)/mt_queue_multiple_threads_tests

TEST_MTQUEUE_MULTITHREAD_SRCS:=mt_queue_multiple_threads_tests.c

TEST_MTQUEUE_MULTITHREAD_OBJS:=$(addprefix $(OBJ_DIR)/, $(TEST_MTQUEUE_MULTITHREAD_SRCS:%.c=%.o))

# C directories and flags

INCLUDE_DIRS:=. .. $(TEST_UTILS_DIR) $(ASSERT_MT_DIR) $(LOGGER_DIR) \
  $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib
DEPEND_LIBS:=json-c cmocka pthread

FERRARI_LIBS:=../bin/libutils.a $(ASSERT_MT_DIR)/bin/libassert_mt.a $(LOGGER_DIR)/bin/liblogger.a

include $(PROJECT_ROOT)/compiler_definitions.mk

# Rules

all: $(CMOCKA_TEST_BIN) $(TEST_MTQUEUE_MULTITHREAD_BIN) $(TEST_PROFILESERVER_BIN)
	cp ./run_all_tests.sh $(BIN_DIR)

$(CMOCKA_TEST_BIN): $(CMOCKA_TEST_OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TEST_MTQUEUE_MULTITHREAD_BIN): $(TEST_MTQUEUE_MULTITHREAD_OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(CMOCKA_TEST_OBJS) $(TEST_MTQUEUE_MULTITHREAD_OBJS): | $(OBJ_DIR)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/%.d
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $@ -MF $@

clean:
	rm -fr $(OBJ_DIR) $(BIN_DIR)
	rm -f *.gcov

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(CMOCKA_TEST_OBJS:%.o=%.d)
-include $(TEST_MTQUEUE_MULTITHREAD_OBJS:%.o=%.d)
endif
