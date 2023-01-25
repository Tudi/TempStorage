#!/bin/bash

export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# valgrind --leak-check=full --track-origins=yes ./basic/bin/scoring_tests
./basic/bin/scoring_tests
SCORING_TESTS_RESULT=$?

cd ./testJSONLoad/bin
# valgrind --leak-check=full --track-origins=yes ./scoring_file_profiles
./scoring_file_profiles
SCORING_FILE_PROFILES_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $SCORING_TESTS_RESULT -ne 0 ] || [ $SCORING_FILE_PROFILES_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT
