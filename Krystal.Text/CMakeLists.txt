add_library(Krystal.Text STATIC)

target_link_libraries(Krystal.Text
  PUBLIC
    Krystal.Core
)

set_target_properties(Krystal.Text
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Text
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Text
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Third-party - FreeType (font files and faces)
find_package(Freetype REQUIRED)
target_link_libraries(Krystal.Text PRIVATE Freetype::Freetype)

# Third-party - HarfBuzz (each face's HarfBuzz face)
find_package(harfbuzz CONFIG REQUIRED)
target_link_libraries(Krystal.Text PRIVATE harfbuzz::harfbuzz)

target_sources(Krystal.Text
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Text/FreeTypeFace.hpp
      include/Krystal.Text/HarfBuzz.hpp

  PRIVATE
    src/FreeTypeFace.cpp
    src/HarfBuzz.cpp
)
