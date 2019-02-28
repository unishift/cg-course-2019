cmake_minimum_required(VERSION 3.5)
project(main)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        common.h
        glad.c
        main.cpp
        ShaderProgram.h
        ShaderProgram.cpp)

set(ADDITIONAL_INCLUDE_DIRS
        dependencies/include/GLAD)
set(ADDITIONAL_LIBRARY_DIRS
        dependencies/lib)
set(ADDITIONAL_RUNTIME_LIBRARY_DIRS
        dependencies/bin)

set(CMAKE_CXX_FLAGS_DEBUG  "${CMAKE_CXX_FLAGS_DEBUG}")

set(OpenGL_GL_PREFERENCE GLVND)

if(WIN32)
    set(ADDITIONAL_INCLUDE_DIRS
            ${ADDITIONAL_INCLUDE_DIRS}
            dependencies/include)
    link_directories(${ADDITIONAL_LIBRARY_DIRS})
else()
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules")
    find_package(glfw3 REQUIRED)
endif()

include_directories(${ADDITIONAL_INCLUDE_DIRS})

include(FindOpenGL)
include(FindDevIL)

add_executable(main ${SOURCE_FILES})

target_include_directories(main PRIVATE ${OPENGL_INCLUDE_DIR})
if(DEVELOP_MODE)
    add_custom_command(TARGET main POST_BUILD COMMAND ln -sf "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}/shaders")
else()
    add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/shaders" "${PROJECT_BINARY_DIR}/shaders")
endif()

if(WIN32)
    add_custom_command(TARGET main POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory "${PROJECT_SOURCE_DIR}/dependencies/bin" $<TARGET_FILE_DIR:main>)
    #set(CMAKE_MSVCIDE_RUN_PATH ${ADDITIONAL_RUNTIME_LIBRARY_DIRS})
    target_compile_options(main PRIVATE)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_LIBRARIES} glfw3dll ${IL_LIBRARIES} ${ILUT_LIBRARIES})
else()
    target_compile_options(main PRIVATE -Wnarrowing)
    target_link_libraries(main LINK_PUBLIC ${OPENGL_LIBRARIES} glfw rt dl ${IL_LIBRARIES} ${ILUT_LIBRARIES})
endif()

