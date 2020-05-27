add_executable(${CMAKE_PROJECT_NAME} logdraw.cpp draw.cpp gtkUI.cpp)
add_library(glad glad.c)

set(OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)

if (OPENGL_FOUND)
    target_include_directories(${CMAKE_PROJECT_NAME} PUBLIC ${OPENGL_INCLUDE_DIR})
    target_link_libraries(${CMAKE_PROJECT_NAME} ${OPENGL_gl_LIBRARY})
endif()

add_definitions (${GTK3_CFLAGS_OTHER})
target_link_libraries(${CMAKE_PROJECT_NAME} glad)
target_link_libraries(${CMAKE_PROJECT_NAME} glm)
target_link_libraries(${CMAKE_PROJECT_NAME} glfw)
target_link_libraries(logdraw ${GTK3_LIBRARIES})
