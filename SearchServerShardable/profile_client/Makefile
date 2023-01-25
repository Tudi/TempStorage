.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/libprofile_client.a

SRCS:=profile_client.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(LOGGER_DIR) $(UTILS_DIR) $(MODELS_DIR) $(SEARCH_DIR) \
  $(DAOS_DIR) $(CACHE_ENGINE_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_LIB)

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

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
