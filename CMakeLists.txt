cmake_minimum_required(VERSION 3.10)
project(KiaakEngine VERSION 1.0)

# Specify C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent will be used to acquire minimal dependencies (lua, sol2) when not provided by system
include(FetchContent)

# Try to find system Lua first; if not found, we'll fetch and build Lua 5.4
find_package(Lua 5.4 QUIET)
if(NOT Lua_FOUND)
    message(STATUS "System Lua 5.4 not found — Fetching Lua ${PROJECT_NAME} will build a private copy")
    FetchContent_Declare(
        lua_src
        GIT_REPOSITORY https://github.com/lua/lua.git
        GIT_TAG v5.4.6
    )
    FetchContent_MakeAvailable(lua_src)

    # Build a tiny static lib from the Lua source (Lua places C files at repo root)
    file(GLOB LUA_SRC_FILES "${lua_src_SOURCE_DIR}/*.c")
    add_library(lua_static STATIC ${LUA_SRC_FILES})
    target_include_directories(lua_static PUBLIC "${lua_src_SOURCE_DIR}")
    # Expose target name 'lua' for linking convenience
    add_library(lua ALIAS lua_static)
else()
    message(STATUS "Found system Lua: ${LUA_INCLUDE_DIR} (lib: ${LUA_LIBRARIES})")
endif()

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
target_include_directories(${PROJECT_NAME} PRIVATE external/imgui_color_text_edit)
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

# Fetch sol2 headers (header-only adapter) if not already vendored
FetchContent_Declare(
    sol2
    GIT_REPOSITORY https://github.com/ThePhD/sol2.git
    GIT_TAG v3.3.0
)
FetchContent_MakeAvailable(sol2)
target_include_directories(${PROJECT_NAME} PRIVATE ${sol2_SOURCE_DIR}/include)

# Link Lua (system or fetched) to the engine
target_link_libraries(${PROJECT_NAME} PRIVATE lua)

if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE "-framework Cocoa")
endif()

# Add compiler warnings
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()
