.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/libdaos.a

SRCS:=daos.c files.c file_table.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. $(COMMON_DIR) $(UTILS_DIR) $(LOGGER_DIR) $(MODELS_DIR) \
  $(ITEM_FUNCTIONS_DIR) $(DB_MIGRATION_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_LIB)
	$(MAKE) -C tests all

$(TARGET_LIB): $(OBJS) | $(BIN_DIR)
	$(AR) rvs $@ $^

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/%.d
	$(CC) $(CFLAGS) -g -o $@ -c $<

$(OBJ_DIR)/%.d: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $@ -MF $@

clean:
	rm -fr $(OBJ_DIR) $(BIN_DIR)
	rm -f *.gcov

clean_all: clean
	$(MAKE) -C tests clean

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
