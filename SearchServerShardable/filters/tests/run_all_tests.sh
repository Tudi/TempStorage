#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# valgrind --leak-check=full ./filters_tests
./filters_tests
