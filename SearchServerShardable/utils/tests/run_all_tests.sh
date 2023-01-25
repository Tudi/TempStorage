#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# valgrind --leak-check=full ./utils_tests
./utils_tests
UTILS_TESTS_RESULT=$?

# valgrind --leak-check=full ./mt_queue_multiple_threads_tests
./mt_queue_multiple_threads_tests
MT_QUEUE_MULTITHREADS_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $UTILS_TESTS_RESULT -ne 0 ] || [ $MT_QUEUE_MULTITHREADS_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT
