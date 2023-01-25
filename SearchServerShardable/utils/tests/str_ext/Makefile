.PHONY: all clean

PROJECT_ROOT:=../../..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/util_tests

SRCS:=strings_ext_tests.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(UTILS_DIR) $(3RDPARTY_DIR)/klib

FERRARI_LIBS:=$(UTILS_DIR)/bin/libutils.a

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^

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
