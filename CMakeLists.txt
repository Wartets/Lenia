cmake_minimum_required(VERSION 3.16)
project(Lenia)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform-specific settings
if(MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find dependencies
find_package(glfw3 3.3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(OpenGL REQUIRED)

# Add source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
)

file(GLOB_RECURSE HEADERS 
    "src/*.hpp"
    "src/*.h"
)

# Create executable
add_executable(Lenia ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(Lenia PRIVATE
    glfw
    glm::glm
    imgui::imgui
    OpenGL::GL
)

# Include directories
target_include_directories(Lenia PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/libs
)

# Platform-specific configurations
if(WIN32)
    set_target_properties(Lenia PROPERTIES
        WIN32_EXECUTABLE ON
    )
    if(MSVC)
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Lenia)
    endif()
endif()

# Copy assets to build directory
add_custom_command(TARGET Lenia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets
    $<TARGET_FILE_DIR:Lenia>/assets
    COMMENT "Copying assets..."
)

# Copy initial colormap data
add_custom_command(TARGET Lenia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/bin/colormap
    $<TARGET_FILE_DIR:Lenia>/colormap
    COMMENT "Copying colormaps..."
)
