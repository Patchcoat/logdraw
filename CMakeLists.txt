cmake_minimum_required(VERSION 3.1)
project(logdraw)

set(CMAKE_CXX_STANDARD 11)

#--------------------------------------------------------------------
# Set GLFW variables
#--------------------------------------------------------------------
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

#--------------------------------------------------------------------
# Set build variables
#--------------------------------------------------------------------
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ../bin)
set(CMAKE BUILD TYPE Debug)

#--------------------------------------------------------------------
# Set build variables
#--------------------------------------------------------------------
find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0)

#--------------------------------------------------------------------
# Include directories
#--------------------------------------------------------------------
include_directories(include)
include_directories(src)
include_directories(${GTK3_INCLUDE_DIRS})
link_directories(${GTK3_LIBRARY_DIRS})

#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------
add_subdirectory(include/glfw-3.3)
add_subdirectory(include/glm)
add_subdirectory(src)
