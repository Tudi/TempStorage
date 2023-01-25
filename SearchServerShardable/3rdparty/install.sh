#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

# cmocka
echo "Building and installing cmocka..."

cd "${SCRIPT_DIR}/sources/cmocka_1.1.5"
mkdir cmocka_build
cd cmocka_build
cmake -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR/cmocka ..
make
make install

echo "Building and installing cmocka... done."

# json-c
echo "Building and installing json-c..."

cd "${SCRIPT_DIR}/sources/json-c_0.16"
mkdir json-c_build
cd json-c_build
cmake -DCMAKE_INSTALL_PREFIX=$SCRIPT_DIR/json-c ..
make
make install

echo "Building and installing json-c... done."

# Sentry
echo "Building and installing Sentry..."

cd "${SCRIPT_DIR}/sources/sentry-native_0.5.0"
cmake -B build -D SENTRY_BACKEND=breakpad -D CMAKE_INSTALL_PREFIX=$SCRIPT_DIR/sentry-native
cd build && make install

echo "Building and installing Sentry... done."
