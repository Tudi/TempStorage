.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/libmodels.a

SRCS:=company.c company_cached.c company_industry.c education.c id_value.c position_cached.c \
  position_persistent.c profile_cached.c profile_email.c profile_persistent.c \
  profile_phone_number.c profile_social_url.c profile_tag.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(UTILS_DIR) $(LOGGER_DIR) $(DB_MIGRATION_DIR) $(DAOS_DIR) \
  $(ITEM_FUNCTIONS_DIR) $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_LIB)
	$(MAKE) -C comparison all
	$(MAKE) -C test_data all
	$(MAKE) -C tests all

$(TARGET_LIB): $(OBJS) | $(BIN_DIR)
	$(AR) rvs $@ $^

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

clean_all: clean
	$(MAKE) -C comparison clean
	$(MAKE) -C test_data clean
	$(MAKE) -C tests clean

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
