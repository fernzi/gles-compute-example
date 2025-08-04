set(TARGET_NAME ${CMAKE_PROJECT_NAME})

set(SHADER_PATH ${CMAKE_CURRENT_SOURCE_DIR}/main.comp.glsl)
configure_file("main.s.in" "main.s" @ONLY)

set_source_files_properties("main.s" OBJECT_DEPENDS ${SHADER_PATH})

add_executable(${TARGET_NAME} "main.cc" "main.s")
target_link_libraries(${TARGET_NAME}
PRIVATE
  epoxy
)
target_compile_options(${TARGET_NAME}
PRIVATE
  $<$<C_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<C_COMPILER_ID:MSVC>>:-Wall -Wpedantic>
)
