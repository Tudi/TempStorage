#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# This next command line silently remove leftovers from previous test executions.
rm company_*.dat 2> /dev/null || true

# valgrind --leak-check=full ./company_cache_engine_tests
./company_cache_engine_tests
COMPANY_TESTS_RESULT=$?

# This next command line silently remove leftovers from previous test executions.
rm profile_*.dat 2> /dev/null || true

# valgrind --leak-check=full ./profile_cache_engine_tests
./profile_cache_engine_tests
PROFILE_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $COMPANY_TESTS_RESULT -ne 0 ] || [ $PROFILE_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT

