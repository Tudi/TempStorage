.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/ss_server_app

SRCS:=ss_server_app.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

# C directories and flags

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(LOGGER_DIR) $(UTILS_DIR) $(ERROR_REPORTING_DIR) \
  $(SS_SERVER_DIR) $(3RDPARTY_DIR)/sentry-native/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/sentry-native/lib
DEPEND_LIBS:=pthread m sentry

FERRARI_LIBS:=$(SS_SERVER_DIR)/bin/libss_server.a \
  $(UTILS_DIR)/bin/libutils.a \
  $(SCORE_MANAGER_DIR)/bin/score_manager.a \
  $(PROFILING_DIR)/bin/profiling.a $(LOGGER_DIR)/bin/liblogger.a \
  $(ERROR_REPORTING_DIR)/bin/liberror_reporting.a

include $(PROJECT_ROOT)/compiler_definitions.mk

# Rules

all: $(TARGET_BIN)

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

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
