# Do not change the order of these commands
cmake_minimum_required(VERSION 3.16)
set(EXTRA_COMPONENT_DIRS ../components)
set(COMPONENTS main)
set(EXTERN_BUF 1)

include($ENV{IDF_PATH}/tools/cmake/project.cmake)
project(lab04)
# idf_build_set_property(COMPILE_OPTIONS "-Wno-error" APPEND)
