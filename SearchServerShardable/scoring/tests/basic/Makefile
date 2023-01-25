.PHONY: all clean

PROJECT_ROOT:=../../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/scoring_tests

SRCS:=scoring_tests.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(PROJECT_ROOT) $(COMPARISON_DIR) $(LOGGER_DIR) $(UTILS_DIR) $(COMMON_DIR) \
  $(TEST_UTILS_DIR) $(MODELS_DIR) $(FILTERS_DIR) $(SCORING_DIR) $(PROFILING_DIR) $(SEARCH_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib $(UTILS_DIR)/bin
DEPEND_LIBS:=m json-c cmocka pthread

FERRARI_LIBS:=$(SCORING_DIR)/bin/scoring.a $(FILTERS_DIR)/bin/filters.a \
  $(MODELS_DIR)/bin/libmodels.a $(TEST_UTILS_DIR)/bin/libtest_utils.a \
  $(SEARCH_DIR)/bin/libsearch.a $(LOGGER_DIR)/bin/liblogger.a \
  $(UTILS_DIR)/bin/libutils.a $(PROFILING_DIR)/bin/profiling.a

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS) $(FERRARI_LIBS)| $(BIN_DIR)
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
