add_library(stb STATIC)

target_compile_options(stb
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(stb
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/..
    FILES
      stb_image.h
      stb_rect_pack.h
  PRIVATE
    stb_image.cpp
    stb_rect_pack.cpp
)