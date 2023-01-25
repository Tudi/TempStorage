#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# valgrind --leak-check=full ./models_tests
./models_tests
