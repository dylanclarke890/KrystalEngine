add_executable(Krystal.Text.Tests)

target_link_libraries(Krystal.Text.Tests
  PRIVATE
    Krystal.Text
    Catch2::Catch2WithMain
)

set_target_properties(Krystal.Text.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Text.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Text.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# The tests read Liberation Sans, as bundled with Booey.
target_compile_definitions(Krystal.Text.Tests
  PRIVATE
    KRYS_TEXT_TESTS_FONTS_DIR="${CMAKE_SOURCE_DIR}/third-party/liberation-fonts"
)

# FreeType and HarfBuzz are read directly to check the faces made from them.
find_package(Freetype REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
target_link_libraries(Krystal.Text.Tests PRIVATE Freetype::Freetype harfbuzz::harfbuzz)

catch_discover_tests(Krystal.Text.Tests)

target_sources(Krystal.Text.Tests
  PRIVATE
    tests/FreeTypeFace.cpp
)
