.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/libutils.a

SRCS:=mt_queue.c assert_mt.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. $(LOGGER_DIR)

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

clean_all: clean
	$(MAKE) -C tests clean

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
