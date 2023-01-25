.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/ss_client_app

SRCS:=ss_client_app.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

# C directories and flags

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(LOGGER_DIR) $(UTILS_DIR) $(SS_CLIENT_DIR) $(SCORE_MANAGER_DIR)

DEPEND_LIB_DIRS:=
DEPEND_LIBS:=pthread

FERRARI_LIBS:=$(SS_CLIENT_DIR)/bin/libss_client.a $(UTILS_DIR)/bin/libutils.a \
  $(LOGGER_DIR)/bin/liblogger.a

include $(PROJECT_ROOT)/compiler_definitions.mk

# Rules

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS) $(FERRARI_LIBS) | $(BIN_DIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: %.c $(OBJ_DIR)/%.d
	$(CC) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d: %.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -MM $< -MT $@ -MF $@

clean:
	rm -fr $(OBJ_DIR) $(BIN_DIR)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
