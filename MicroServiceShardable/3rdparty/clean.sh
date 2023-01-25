#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

# sentry
rm -fr "${SCRIPT_DIR}/sources/sentry-native_0.5.0/build"
