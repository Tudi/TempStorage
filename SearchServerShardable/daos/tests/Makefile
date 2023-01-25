.PHONY: all clean

PROJECT_ROOT:=../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/daos_tests

SRCS:=company_daos_tests.c profile_daos_tests.c daos_V7_tests.c\
  tests_main.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(MODELS_DIR) $(COMPARISON_DIR) $(TEST_DATA_DIR) $(ITEM_FUNCTIONS_DIR) \
  $(TEST_UTILS_DIR) $(UTILS_DIR) $(LOGGER_DIR) $(ASSERT_MT_DIR)\
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib
DEPEND_LIBS:=json-c cmocka pthread

FERRARI_LIBS:=../bin/libdaos.a $(TEST_DATA_DIR)/bin/libtest_data.a $(ITEM_FUNCTIONS_DIR)/bin/libitem_functions.a \
  $(COMPARISON_DIR)/bin/libmodels_comparison.a $(MODELS_DIR)/bin/libmodels.a \
  $(TEST_UTILS_DIR)/bin/libtest_utils.a $(UTILS_DIR)/bin/libutils.a $(LOGGER_DIR)/bin/liblogger.a \
  $(DB_MIGRATION_DIR)/bin/libmigration.a

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_BIN)
	cp ./run_all_tests.sh $(BIN_DIR)
	cp -R ./invalid_data $(BIN_DIR)

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
