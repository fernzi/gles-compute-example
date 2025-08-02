cmake_minimum_required(VERSION 3.25..3.31)

project(gles-compute-example
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

find_package(epoxy 1.5 REQUIRED)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})

add_executable(${TARGET_NAME} "main.cc")
target_link_libraries(${TARGET_NAME}
PRIVATE
  epoxy
)
target_compile_options(${TARGET_NAME}
PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
)
