cmake_minimum_required(VERSION 3.15)

project(ToolKit)

set(CMAKE_CXX_STANDARD  17)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

add_executable(ToolKit WinMain.cpp WinMain.h ToolKitUI.cpp ToolKitUI.h)

# Set the Windows SDK version
set(CMAKE_SYSTEM_VERSION 10.0 CACHE STRING "" FORCE)

# Link against user32.lib, gdi32.lib, d3d9.lib and lib::imgui
target_link_libraries(ToolKit PRIVATE user32.lib gdi32.lib d3d9.lib lib::imgui)

# Specify the Windows subsystem
set_target_properties(ToolKit PROPERTIES WIN32_EXECUTABLE YES)

add_subdirectory("vendor")