cmake_minimum_required(VERSION 3.5)

set(PROJECT_VER 1.0.0.1)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)

add_compile_definitions( LGFX_USE_V1 )  # use V1 instead of V0
add_compile_definitions( PROJECT_NAME_=\"${CMAKE_PROJECT_NAME}\" )

list(APPEND EXTRA_COMPONENT_DIRS components)

project(espidf-iot-base)
