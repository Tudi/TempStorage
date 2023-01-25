#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

# cmocka
rm -fr "${SCRIPT_DIR}/sources/cmocka_1.1.5/cmocka_build"
rm -fr "${SCRIPT_DIR}/cmocka"

# json-c
rm -fr "${SCRIPT_DIR}/sources/json-c_0.16/json-c_build"
rm -fr "${SCRIPT_DIR}/json-c"

# sentry
rm -fr "${SCRIPT_DIR}/sources/sentry-native_0.5.0/build"
