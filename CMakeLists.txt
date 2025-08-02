cmake_minimum_required(VERSION 3.25..3.31)

project(gles-compute-example
  VERSION 0.1.0
  LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(TARGET_NAME ${CMAKE_PROJECT_NAME})

add_executable(${TARGET_NAME} "main.cc")
target_compile_options(${TARGET_NAME}
PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
)
