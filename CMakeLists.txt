cmake_minimum_required(VERSION 3.20)
project(VirtualLogiGate C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Find SDL3
find_package(SDL3 REQUIRED CONFIG)
find_package(SDL3_ttf REQUIRED CONFIG)

# Collect all source files
file(GLOB SOURCES "src/*.c")

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link SDL3
target_link_libraries(${PROJECT_NAME} 
    PRIVATE 
    SDL3::SDL3 
    SDL3_ttf::SDL3_ttf
)

set(ASSET_DIR "${CMAKE_BINARY_DIR}/assets")
file(MAKE_DIRECTORY ${ASSET_DIR})

set(FONT_SOURCE "${CMAKE_SOURCE_DIR}/assets/fonts/Arial.ttf")
set(FONT_DESTINATION "${ASSET_DIR}/fonts/Arial.ttf")

if(EXISTS ${FONT_SOURCE})
    configure_file(${FONT_SOURCE} ${FONT_DESTINATION} COPYONLY)
else()
    message(FATAL_ERROR "Font file not found: ${FONT_SOURCE}")
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
    FONT_PATH="${ASSET_DIR}/fonts/Arial.ttf"
)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD

    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to output directory"
)
