#!/bin/bash

./test_import.sh
IMPORT_TESTS_RESULT=$?

./test_request.sh
REQUEST_TESTS_RESULT=$?

./test_request_fails.sh
REQUEST_FAILS_TESTS_RESULT=$?

./test_import_fails.sh
IMPORT_FAILS_TESTS_RESULT=$?

TESTS_RESULT=0

if [ $IMPORT_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
echo -e "\033[0;31m Import test failed \033[0m"
fi

if [ $REQUEST_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
echo -e "\033[0;31m Request test failed \033[0m"
fi

if [ $REQUEST_FAILS_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
echo -e "\033[0;31m Request failes test failed \033[0m"
fi

if [ $IMPORT_FAILS_TESTS_RESULT -ne 0 ]
then
TESTS_RESULT=1
echo -e "\033[0;31m Import failes test failed \033[0m"
fi

exit $TESTS_RESULT
