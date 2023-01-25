CC:=gcc

# For debugging
# CFLAGS+=-O0
SANITIZE_FLAGS:=-fsanitize=address -fsanitize=undefined -fno-sanitize=null -fno-sanitize=alignment

# For speed
CFLAGS+=-O3

CFLAGS:=-Werror -Wall
CFLAGS+=$(SANITIZE_FLAGS)
CFLAGS+=$(addprefix -I, $(INCLUDE_DIRS))
CFLAGS+=-D_XOPEN_SOURCE=700
CFLAGS += -g

LDFLAGS+=$(SANITIZE_FLAGS)
LDFLAGS+=$(addprefix -L, $(DEPEND_LIB_DIRS))
LDFLAGS+=$(addprefix -l, $(DEPEND_LIBS))
