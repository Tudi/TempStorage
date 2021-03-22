# The minimum version of CMake that will work.
cmake_minimum_required(VERSION 2.8)

# The project name.
project(portsinkSQS)

# Requires that you build with:
#   -Daws-sdk-cpp_DIR=../sdk_build
find_package(aws-sdk-cpp)

# Link to the AWS SDK for C++ shared libraries.
add_definitions(-DUSE_IMPORT_EXPORT)

# The executable name and its source files.
add_executable(portsinkSQS portsinkSQS.cpp)

# The libraries used by your executable.
target_link_libraries(portsinkSQS aws-cpp-sdk-sqs uuid)
