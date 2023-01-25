SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

export TZ=UTC
export FERRARI_C_TEST_LIB_PATH=$SCRIPT_DIR/3rdparty/json-c/lib:$SCRIPT_DIR/3rdparty/cmocka/lib
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH
