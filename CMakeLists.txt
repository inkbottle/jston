cmake_minimum_required(VERSION 3.16.0)
project(jston VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(inc)

find_package(nlohmann_json CONFIG REQUIRED)

add_executable(test_basic test/test_basic.cpp)
target_link_libraries(test_basic nlohmann_json::nlohmann_json)

add_executable(test_advanced test/test_advanced.cpp)
target_link_libraries(test_advanced nlohmann_json::nlohmann_json)

