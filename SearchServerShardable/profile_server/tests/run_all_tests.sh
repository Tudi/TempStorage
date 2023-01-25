#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# This next command line silently removes leftovers from previous test executions.
rm profile_*.dat 2> /dev/null || true

# valgrind --leak-check=full ./cmocka_profile_server_tests
./cmocka_profile_server_tests
CMOCKA_TESTS_RESULT=$?

# valgrind --leak-check=full ./profile_server_tests
./profile_server_tests
PROFILE_SERVER_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $CMOCKA_TESTS_RESULT -ne 0 ] || [ $PROFILE_SERVER_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT
