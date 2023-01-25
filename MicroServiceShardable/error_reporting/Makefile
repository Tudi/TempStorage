.PHONY: all clean

PROJECT_ROOT:=..

include $(PROJECT_ROOT)/directories.mk

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_LIB:=$(BIN_DIR)/liberror_reporting.a

SRCS:=error_reporting.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. .. $(LOGGER_DIR) $(3RDPARTY_DIR)/sentry-native/include
DEPEND_LIB_DIRS:=$(3RDPARTY_DIR)/sentry-native/lib
DEPEND_LIBS:=sentry

include $(PROJECT_ROOT)/compiler_definitions.mk

all: $(TARGET_LIB)

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

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif

upload-sentry:
	sentry-cli upload-dif --log-level=debug --include-sources -o recruiter-com -p similarity-service ../ --wait
