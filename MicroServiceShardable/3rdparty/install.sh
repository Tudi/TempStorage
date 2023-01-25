#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

# Sentry
echo "Building and installing Sentry..."

cd "${SCRIPT_DIR}/sources/sentry-native_0.5.0"
cmake -B build -D SENTRY_BACKEND=breakpad -D CMAKE_INSTALL_PREFIX="${SCRIPT_DIR}/sentry-native"
cd build && make install

echo "Building and installing Sentry... done."
