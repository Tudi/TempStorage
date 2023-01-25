#!/bin/bash
set -e

if [ $# -ne 1 ]; then
  echo "Error: incorrect number of arguments ($#)."
  echo "Usage: $0 [ generate | clean_last_run | clean_all ]"
  exit 1
fi

run_generate()
{
  full_lcov_cmd="lcov --output-file coverage.info"
  for directory in ${directories_list[@]}; do
    input_file=coverage/${directory//\//_}.info
    full_lcov_cmd+=" --add-tracefile $input_file"
  done

  mkdir -p coverage

  original_directory=$(pwd)

  for directory in ${directories_list[@]}; do
    cd "$directory"
    gcov --object-directory ./obj ./*.c
    cd "$original_directory"
  done

  for directory in ${directories_list[@]}; do
    out_file=coverage/${directory//\//_}.info
    lcov --directory "$directory/obj" --base-directory "$directory" --capture --output-file "$out_file" --no-recursion
  done

  eval "$full_lcov_cmd"
}

partial_clean()
{
  for directory in ${directories_list[@]}; do
    rm -fv "$directory"/obj/*.gcda
    rm -fv "$directory"/*.gcov
  done

  for directory in ${extra_directories_list[@]}; do
    rm -fv "$directory"/obj/*.gcda
  done

  rm -fr coverage
  rm -fv coverage.info
}

run_clean_last_run()
{
  partial_clean
}

run_clean_all()
{
  for directory in ${directories_list[@]}; do
    rm -fv "$directory"/obj/*.gcno
  done

  for directory in ${extra_directories_list[@]}; do
    rm -fv "$directory"/obj/*.gcno
  done

  partial_clean
}

extra_directories_list=(assert_mt cache_engine/tests daos/tests error_reporting filters/tests
  models/comparison models/test_data models/tests
  profile_client profile_client_app profile_server/tests profile_server_app profiling search/tests
  scoring/tests/basic scoring/tests/testJSONLoad stress_tests stress_tests/stress_insert test_utils utils/tests)

directories_list=(logger utils tools/db_migration models item_functions search daos cache_engine filters
  scoring profile_server)

if [ "$1" = "generate" ]; then
  run_generate
elif [ "$1" = "clean_last_run" ]; then
  run_clean_last_run
elif [ "$1" = "clean_all" ]; then
  run_clean_all
else
  echo "Error: invalid argument."
  echo "Usage: $0 [ generate | clean_last_run | clean_all ]"
  exit 2
fi

