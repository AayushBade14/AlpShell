cmake_minimum_required(VERSION 3.10)
project(alpshell)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Include directory
include_directories(include)

# Source files
set(SOURCES
    main.cpp
    src/command.cpp
    src/executor.cpp
    src/history.cpp
    src/job_control.cpp
    src/lexer.cpp
    src/parser.cpp
    src/shell.cpp
    src/util.cpp
)

# Add executable
add_executable(alpshell ${SOURCES})

# Link libraries
target_link_libraries(alpshell readline)
