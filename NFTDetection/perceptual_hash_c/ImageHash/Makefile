.PHONY: all clean

PROJECT_ROOT:=.

OBJ_DIR:=obj
BIN_DIR:=bin

TARGET_BIN:=$(BIN_DIR)/imgHash

SRCS:=ImageHash.cpp main.cpp DCTII.cpp BitWriter.cpp ImageHandler.cpp

OBJS:=$(addprefix $(OBJ_DIR)/, $(SRCS:%.cpp=%.o))

# C directories and flags

INCLUDE_DIRS:=. ../FreeImage

DEPEND_LIB_DIRS:=../FreeImage
DEPEND_LIBS:=m stdc++

ImgHash_Libs:=../FreeImage/libfreeimage.a

include compiler_definitions.mk

# Rules

all: $(TARGET_BIN)

$(TARGET_BIN): $(OBJS) $(ImgHash_Libs) | $(BIN_DIR)
	$(CXX) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJS): | $(OBJ_DIR)

$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

$(OBJ_DIR)/%.o: %.cpp $(OBJ_DIR)/%.d
	$(CXX) $(CFLAGS) -o $@ -c $<

$(OBJ_DIR)/%.d: %.cpp | $(OBJ_DIR)
	$(CXX) $(CFLAGS) -MM $< -MT $@ -MF $@

clean:
	rm -fr $(OBJ_DIR) $(BIN_DIR)

ifeq (,$(findstring clean,$(MAKECMDGOALS)))
-include $(OBJS:%.o=%.d)
endif
