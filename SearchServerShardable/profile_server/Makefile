.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/libprofile_server.a

SRCS:=file_lock_list.c profile_server.c request_processing.c search_engine.c system_info_data.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(LOGGER_DIR) $(UTILS_DIR) $(COMMON_DIR) $(MODELS_DIR) $(SEARCH_DIR) $(ITEM_FUNCTIONS_DIR) \
  $(DAOS_DIR) $(CACHE_ENGINE_DIR) $(FILTERS_DIR) $(SCORING_DIR) $(PROFILING_DIR) \
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
	$(CC) $(CFLAGS) -o $@ -c $<

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
