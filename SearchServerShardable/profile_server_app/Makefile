.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/profile_server_app

SRCS:=profile_server_app.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

# C directories and flags

INCLUDE_DIRS:=. .. $(COMMON_DIR) $(LOGGER_DIR) $(UTILS_DIR) $(MODELS_DIR) $(SEARCH_DIR) $(ERROR_REPORTING_DIR) \
  $(DAOS_DIR) $(PROFILE_SERVER_DIR) $(CACHE_ENGINE_DIR) $(FILTERS_DIR) $(SCORING_DIR) $(ITEM_FUNCTIONS_DIR) \
  $(3RDPARTY_DIR)/klib $(3RDPARTY_DIR)/json-c/include/json-c $(3RDPARTY_DIR)/sentry-native/include

DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/json-c/lib $(3RDPARTY_DIR)/sentry-native/lib
DEPEND_LIBS:=json-c pthread m sentry

FERRARI_LIBS:=$(PROFILE_SERVER_DIR)/bin/libprofile_server.a \
  $(CACHE_ENGINE_DIR)/bin/libcache_engine.a $(DAOS_DIR)/bin/libdaos.a \
  $(SCORING_DIR)/bin/scoring.a $(FILTERS_DIR)/bin/filters.a $(ITEM_FUNCTIONS_DIR)/bin/libitem_functions.a \
  $(SEARCH_DIR)/bin/libsearch.a $(MODELS_DIR)/bin/libmodels.a \
  $(DB_MIGRATION_DIR)/bin/libmigration.a \
  $(UTILS_DIR)/bin/libutils.a \
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
	rm -f *.gcov

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
