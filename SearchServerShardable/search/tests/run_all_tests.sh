#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# valgrind --leak-check=full ./search_tests
./search_tests
