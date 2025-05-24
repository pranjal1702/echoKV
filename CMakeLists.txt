cmake_minimum_required(VERSION 3.10)
project(echoKV LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/lib/catch2)

# Source files
add_library(hash_table src/linear_probing_hash_table.cpp lib/catch2/catch_amalgamated.cpp)
target_link_libraries(hash_table PRIVATE xxhash)

# Test executable
add_executable(test_hash_table tests/test_hash_table.cpp)
target_link_libraries(test_hash_table PRIVATE hash_table)

# Enable testing
enable_testing()
add_test(NAME HashTableTests COMMAND test_hash_table)