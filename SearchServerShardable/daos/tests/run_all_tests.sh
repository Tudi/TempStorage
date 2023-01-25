#!/bin/bash
export LD_LIBRARY_PATH=$FERRARI_C_TEST_LIB_PATH

# These next two command lines silently remove leftovers from previous test executions.
rm company_*.dat 2> /dev/null || true
rm profile_*.dat 2> /dev/null || true

# valgrind --leak-check=full ./daos_tests
./daos_tests
