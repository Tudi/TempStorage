#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

cd "${SCRIPT_DIR}/utils/tests/bin"
./run_all_tests.sh
UTILS_TESTS_RESULT=$?

cd "${SCRIPT_DIR}/tests"
./run_all_tests.sh
SS_SERVER_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $UTILS_TESTS_RESULT -ne 0 ] || [ $SS_SERVER_TESTS_RESULT -ne 0 ]
then
	echo -e "\033[0;31m Some tests have failed ! \033[0m"
TESTS_RESULT=1
fi

exit $TESTS_RESULT
