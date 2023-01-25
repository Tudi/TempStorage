.PHONY: all clean

PROJECT_ROOT:=../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/models_tests

SRCS:=binary_utils_tests.c company_cached_tests.c company_tests.c company_industry_tests.c \
  education_tests.c id_value_tests.c position_cached_tests.c position_persistent_tests.c \
  profile_cached_tests.c profile_email_tests.c profile_persistent_tests.c \
  profile_phone_number_tests.c profile_social_url_tests.c profile_tag_tests.c \
  tests_main.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(COMPARISON_DIR) $(UTILS_DIR) $(TEST_UTILS_DIR) $(COMMON_DIR) \
  $(TEST_DATA_DIR) $(LOGGER_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/cmocka/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/cmocka/lib
DEPEND_LIBS:=m json-c cmocka pthread

FERRARI_LIBS:=$(COMPARISON_DIR)/bin/libmodels_comparison.a \
   ../bin/libmodels.a \
   $(DB_MIGRATION_DIR)/bin/libmigration.a \
   $(TEST_DATA_DIR)/bin/libtest_data.a $(TEST_UTILS_DIR)/bin/libtest_utils.a \
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
