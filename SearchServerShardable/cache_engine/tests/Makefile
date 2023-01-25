.PHONY: all clean

PROJECT_ROOT:=../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

# Company cache tests

COMPANY_CACHE_TEST_BIN:=$(BIN_DIR)/company_cache_engine_tests

COMPANY_CACHE_TEST_SRCS:=company_cache_engine_tests.c

COMPANY_CACHE_TEST_OBJS:=$(addprefix $(OBJ_DIR)/, $(COMPANY_CACHE_TEST_SRCS:%.c=%.o))

# Profile cache tests

PROFILE_CACHE_TEST_BIN:=$(BIN_DIR)/profile_cache_engine_tests

PROFILE_CACHE_TEST_SRCS:=profile_cache_engine_tests.c

PROFILE_CACHE_TEST_OBJS:=$(addprefix $(OBJ_DIR)/, $(PROFILE_CACHE_TEST_SRCS:%.c=%.o))

# C directories and flags

INCLUDE_DIRS:=. .. $(UTILS_DIR) $(LOGGER_DIR) $(COMMON_DIR) $(MODELS_DIR) $(DAOS_DIR) \
  $(ASSERT_MT_DIR) $(TEST_UTILS_DIR) $(COMPARISON_DIR) $(TEST_DATA_DIR) $(ITEM_FUNCTIONS_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib
DEPEND_LIBS:=json-c cmocka pthread m

FERRARI_LIBS:=../bin/libcache_engine.a $(DAOS_DIR)/bin/libdaos.a $(COMPARISON_DIR)/bin/libmodels_comparison.a \
  $(TEST_DATA_DIR)/bin/libtest_data.a $(ITEM_FUNCTIONS_DIR)/bin/libitem_functions.a \
  $(MODELS_DIR)/bin/libmodels.a $(DB_MIGRATION_DIR)/bin/libmigration.a \
  $(TEST_UTILS_DIR)/bin/libtest_utils.a $(UTILS_DIR)/bin/libutils.a \
  $(ASSERT_MT_DIR)/bin/libassert_mt.a $(LOGGER_DIR)/bin/liblogger.a 

include $(PROJECT_ROOT)/compiler_definitions.mk

#
# Rules
#

all: $(COMPANY_CACHE_TEST_BIN) $(PROFILE_CACHE_TEST_BIN)
	cp ./run_all_tests.sh $(BIN_DIR)

# Company cache tests

$(COMPANY_CACHE_TEST_BIN): $(COMPANY_CACHE_TEST_OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(COMPANY_CACHE_TEST_OBJS): | $(OBJ_DIR)

# Profile cache tests

$(PROFILE_CACHE_TEST_BIN): $(PROFILE_CACHE_TEST_OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROFILE_CACHE_TEST_OBJS): | $(OBJ_DIR)

# Common

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
-include $(COMPANY_CACHE_TEST_OBJS:%.o=%.d)
-include $(PROFILE_CACHE_TEST_OBJS:%.o=%.d)
endif
