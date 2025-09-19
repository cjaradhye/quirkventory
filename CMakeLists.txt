cmake_minimum_required(VERSION 3.10)
project(Quirkventory VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set compiler flags
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
    else()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    endif()
endif()

# Include directories
include_directories(include)

# Find required packages
find_package(Threads REQUIRED)

# Google Test configuration
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        v1.14.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Source files
set(SOURCES
    src/Product.cpp
    src/Inventory.cpp
    src/Order.cpp
    src/User.cpp
    src/NotificationSystem.cpp
    src/CLI.cpp
    src/HTTPServer.cpp
)

# Header files
set(HEADERS
    include/Product.hpp
    include/Inventory.hpp
    include/Order.hpp
    include/User.hpp
    include/NotificationSystem.hpp
    include/CLI.hpp
    include/HTTPServer.hpp
)

# Create library for reusable components
add_library(quirkventory_lib STATIC ${SOURCES} ${HEADERS})
target_link_libraries(quirkventory_lib ${CMAKE_THREAD_LIBS_INIT})
target_include_directories(quirkventory_lib PUBLIC include)

# Main executable
add_executable(quirkventory src/main.cpp)
target_link_libraries(quirkventory quirkventory_lib)

# Test executable (optional)
add_executable(quirkventory_test 
    tests/test_main.cpp
)
target_link_libraries(quirkventory_test quirkventory_lib)
target_include_directories(quirkventory_test PRIVATE tests)

# Google Test based unit tests
add_executable(quirkventory_gtest
    tests/gtest/test_product_gtest.cpp
    tests/gtest/test_inventory_gtest.cpp
    tests/gtest/test_order_gtest.cpp
    tests/gtest/test_user_gtest.cpp
    tests/gtest/test_notification_gtest.cpp
    tests/gtest/test_integration.cpp
)
target_link_libraries(quirkventory_gtest 
    quirkventory_lib 
    gtest_main 
    gmock_main
)
target_include_directories(quirkventory_gtest PRIVATE include tests)

# Enable testing
enable_testing()
include(GoogleTest)
gtest_discover_tests(quirkventory_gtest)

# Installation
install(TARGETS quirkventory DESTINATION bin)
install(TARGETS quirkventory_lib DESTINATION lib)
install(FILES ${HEADERS} DESTINATION include/quirkventory)

# Package configuration
set(CPACK_PACKAGE_NAME "Quirkventory")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "C++ Inventory Management System with OOP Demonstration")
set(CPACK_PACKAGE_VENDOR "Quirkventory Systems")
include(CPack)

# Custom targets for convenience
add_custom_target(run
    COMMAND quirkventory
    DEPENDS quirkventory
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running Quirkventory application"
)

add_custom_target(run_tests
    COMMAND quirkventory_test
    DEPENDS quirkventory_test
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running Quirkventory tests"
)

add_custom_target(run_gtest
    COMMAND quirkventory_gtest
    DEPENDS quirkventory_gtest
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running Google Test suite"
)

add_custom_target(test_all
    COMMAND quirkventory_test && quirkventory_gtest
    DEPENDS quirkventory_test quirkventory_gtest
    WORKING_DIRECTORY ${CMAKE_PROJECT_DIR}
    COMMENT "Running all tests"
)

# Documentation target (requires Doxygen)
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(doc_doxygen ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Print build information
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")