#!/bin/bash

SCRIPT_FILE=$(readlink -f "$0")
SCRIPT_DIR=$(dirname "$SCRIPT_FILE")

cd $SCRIPT_DIR/utils/tests/bin
./run_all_tests.sh
UTILS_TESTS_RESULT=$?

cd $SCRIPT_DIR/models/tests/bin
./run_all_tests.sh
MODELS_TESTS_RESULT=$?

cd $SCRIPT_DIR/search/tests/bin
./run_all_tests.sh
SEARCH_TESTS_RESULT=$?

cd $SCRIPT_DIR/daos/tests/bin
./run_all_tests.sh
DAOS_TESTS_RESULT=$?

cd $SCRIPT_DIR/cache_engine/tests/bin
./run_all_tests.sh
CACHE_ENGINE_TESTS_RESULT=$?

cd $SCRIPT_DIR/profile_server/tests/bin
./run_all_tests.sh
PROFILE_SERVER_TESTS_RESULT=$?

cd $SCRIPT_DIR/filters/tests/bin
./run_all_tests.sh
FILTERS_TESTS_RESULT=$?

cd $SCRIPT_DIR/scoring/tests
./run_all_tests.sh
SCORING_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $UTILS_TESTS_RESULT -ne 0 ] || [ $MODELS_TESTS_RESULT -ne 0 ] \
    || [ $SEARCH_TESTS_RESULT -ne 0 ] || [ $DAOS_TESTS_RESULT -ne 0 ] \
    || [ $CACHE_ENGINE_TESTS_RESULT -ne 0 ] || [ $PROFILE_SERVER_TESTS_RESULT -ne 0 ] \
    || [ $FILTERS_TESTS_RESULT -ne 0 ] || [ $SCORING_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT
