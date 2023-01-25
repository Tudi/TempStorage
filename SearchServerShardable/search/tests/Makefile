.PHONY: all clean

PROJECT_ROOT:=../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/search_tests

SRCS:=composite_score_comparison.c composite_score_tests.c search_criteria_comparison.c \
  search_criteria_tests.c search_filter_comparison.c search_filter_tests.c \
  similarityscore_tests.c \
  tests_main.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(COMPARISON_DIR) $(UTILS_DIR) $(TEST_UTILS_DIR) $(LOGGER_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib
DEPEND_LIBS:=json-c cmocka pthread

FERRARI_LIBS:=../bin/libsearch.a $(TEST_UTILS_DIR)/bin/libtest_utils.a \
  $(UTILS_DIR)/bin/libutils.a $(LOGGER_DIR)/bin/liblogger.a

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_BIN)
	cp ./run_all_tests.sh $(BIN_DIR)

$(TARGET_BIN): $(OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): | $(OBJ_DIR)

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
-include $(OBJS:%.o=%.d)
endif
