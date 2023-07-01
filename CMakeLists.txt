cmake_minimum_required(VERSION 3.14)

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 20)

set(PATCH_VERSION "1" CACHE INTERNAL "Patch version")
set(PROJECT_VESRION 0.0.${PATCH_VERSION})

project(prodcon VERSION ${PROJECT_VESRION})

find_package(Threads)

add_executable(${PROJECT_NAME} main.cpp) # target source ...
target_link_libraries(${PROJECT_NAME} 
    Threads::Threads)

set_target_properties(${PROJECT_NAME} PROPERTIES    # target PROPERTIES prop1 value1  ...
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
target_include_directories(${PROJECT_NAME}
    PRIVATE "${CMAKE_BINARY_DIR}"
)

# ++setup google test localy
include(FetchContent)   # run ..\CMake\share\cmake-X.YY\Modules\FetchContent.cmake
set(GTESTS_URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip)
FetchContent_Declare(
    googletest
    URL ${GTESTS_URL}
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_GetProperties(googletest)
if(NOT googletest_POPULATED)
    FetchContent_Populate(googletest)
    add_subdirectory(${googletest_SOURCE_DIR} ${googletest_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark.git
    GIT_TAG v1.8.0) # latest for 06-2023
set(BENCHMARK_ENABLE_TESTING off)
FetchContent_GetProperties(googlebenchmark)
if(NOT googlebenchmark_POPULATED)
    FetchContent_Populate(googlebenchmark)
    add_subdirectory(${googlebenchmark_SOURCE_DIR} ${googlebenchmark_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

enable_testing()    # Enables testing for this directory and below
# --setup google test localy

# Make Release by default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

if (MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W4
    )
else ()
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -pedantic -Werror
    )
endif()

add_executable(test_${PROJECT_NAME} test.cpp)
target_link_libraries(test_${PROJECT_NAME} 
    GTest::gtest_main 
    Threads::Threads)

include(GoogleTest)               # run ..\CMake\share\cmake-X.YY\Modules\GoogleTest.cmake
gtest_discover_tests(test_${PROJECT_NAME})  # discover tests by asking the compiled test executable to enumerate its tests

add_executable(bench_${PROJECT_NAME} bench.cpp)
target_include_directories(bench_${PROJECT_NAME} PUBLIC benchmarks/include)

#find_package(benchmark REQUIRED)
target_link_libraries(bench_${PROJECT_NAME} 
            benchmark::benchmark_main                        
            benchmark::benchmark
            Threads::Threads)

install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} test_${PROJECT_NAME} RUNTIME DESTINATION bin)
install(TARGETS ${PROJECT_NAME} bench_${PROJECT_NAME} RUNTIME DESTINATION bin)


set(CPACK_GENERATOR DEB)
set(CPACK_PACKAGE_VERSION_MAJOR "${PROJECT_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${PROJECT_VERSION_MINOR}")
set(CPACK_PACKAGE_VERSION_PATCH "${PROJECT_VERSION_PATCH}")

set(CPACK_PACKAGE_CONTACT vovchek@inbox.ru)

include(CPack)
