cmake_minimum_required(VERSION 3.10)
project(KiaakEngine VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Use pkg-config to find GLFW
find_package(PkgConfig REQUIRED)
pkg_check_modules(GLFW REQUIRED glfw3)

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# Add editor source files
file(GLOB_RECURSE EDITOR_SOURCES "editor/src/*.cpp")
file(GLOB_RECURSE EDITOR_HEADERS "editor/include/*.hpp")

# Add GLAD source
set(GLAD_SOURCES external/glad/src/glad.c)

# Add ImGui sources
set(IMGUI_SOURCES
    external/imgui/imgui.cpp
    external/imgui/imgui_demo.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_opengl3.cpp
)

# Create the executable
set(PLATFORM_SOURCES)
if(APPLE)
    list(APPEND PLATFORM_SOURCES external/platform/osx_file_dialog.mm)
endif()

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} ${EDITOR_SOURCES} ${EDITOR_HEADERS} ${GLAD_SOURCES} ${IMGUI_SOURCES} ${PLATFORM_SOURCES})

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE include)
target_include_directories(${PROJECT_NAME} PRIVATE editor/include)
target_include_directories(${PROJECT_NAME} PRIVATE external/glad/include)
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui)
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui/backends)
target_include_directories(${PROJECT_NAME} PRIVATE external)
target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/glfw/3.4/include)
target_include_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/glm/1.0.1/include)

# Link directories
target_link_directories(${PROJECT_NAME} PRIVATE /opt/homebrew/Cellar/glfw/3.4/lib)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    glfw
    OpenGL::GL
)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
endif()

# Add compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
