cmake_minimum_required(VERSION 3.20)
project(VirtualLogiGate C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL3
find_package(SDL3 REQUIRED CONFIG)

# Collect all source files
file(GLOB SOURCES "src/*.c")

# Create executable
add_executable(VirtualLogiGate ${SOURCES})

# Link SDL3
target_link_libraries(VirtualLogiGate PRIVATE SDL3::SDL3)
