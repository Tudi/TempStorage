CC:=gcc

ifdef DEBUG
  CFLAGS+=-O0
  SANITIZE_FLAGS:=-fsanitize=address -fsanitize=undefined -fno-sanitize=null -fno-sanitize=alignment
else
  CFLAGS+=-O3
  SANITIZE_FLAGS:=
endif

ifdef COVERAGE
  COVERAGE_FLAGS+=--coverage
endif

CFLAGS+=-Werror -Wall
CFLAGS+=$(SANITIZE_FLAGS)
CFLAGS+=$(addprefix -I, $(INCLUDE_DIRS))
CFLAGS+=-D_XOPEN_SOURCE=700
CFLAGS+=-g
CFLAGS+=$(COVERAGE_FLAGS)

LDFLAGS+=$(SANITIZE_FLAGS)
LDFLAGS+=$(addprefix -L, $(DEPEND_LIB_DIRS))
LDFLAGS+=$(addprefix -l, $(DEPEND_LIBS))
LDFLAGS+=$(COVERAGE_FLAGS)
