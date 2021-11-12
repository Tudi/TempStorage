.PHONY: all clean

CC=gcc
OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/strstrProfile

SRCS:=strstrBothLen_v2.c main.c strstrSearchedLen.c strstrReference.c strstrMischasanEdited.c strstrBothLen.c \
	strstrNOPString.c InputGenerator.c ProfileTimer.c strstrSumCmp.c strstr5bit.c

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

INCLUDE_DIRS:=. 

CFLAGS:=$(CFLAGS) -Werror -Wall -Ofast -g
CFLAGS:=$(CFLAGS) $(addprefix -I, $(INCLUDE_DIRS))

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS) | $(BIN_DIR)
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
