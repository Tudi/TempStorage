#!/bin/bash
./utils_tests
UTILS_TESTS_RESULT=$?

./mt_queue_multiple_threads_tests
MT_QUEUE_MULTITHREADS_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $UTILS_TESTS_RESULT -ne 0 ] || [ $MT_QUEUE_MULTITHREADS_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
fi

exit $TESTS_RESULT
