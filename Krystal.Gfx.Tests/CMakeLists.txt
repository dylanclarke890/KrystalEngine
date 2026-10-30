add_executable(Krystal.Gfx.Tests)

target_link_libraries(Krystal.Gfx.Tests
  PRIVATE
    Krystal.Core
    Krystal.Gfx
    Catch2::Catch2WithMain
    stb
    Krystal.Text
)

set_target_properties(Krystal.Gfx.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Gfx.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Gfx.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Render2D's goldens are kept in the source tree.
target_compile_definitions(Krystal.Gfx.Tests
  PRIVATE
    KRYS_GFX_TESTS_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden"
    KRYS_GFX_TESTS_FONTS_DIR="${CMAKE_SOURCE_DIR}/third-party/liberation-fonts"
)

# FreeType is read directly, to compare glyphs with its own rendering.
find_package(Freetype REQUIRED)
target_link_libraries(Krystal.Gfx.Tests PRIVATE Freetype::Freetype)

catch_discover_tests(Krystal.Gfx.Tests)

target_sources(Krystal.Gfx.Tests
  PRIVATE
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx.Tests/Golden.hpp
      include/Krystal.Gfx.Tests/TestDevice.hpp

  PRIVATE
    src/Golden.cpp

    tests/CommandBuffer.cpp
    tests/Formats.cpp
    tests/Pool.cpp

    # OpenGL
    tests/OpenGL/Device.cpp

    # Render2D
    tests/Render2D/CoverageBlur.cpp
    tests/Render2D/FilterRenderer2D.cpp
    tests/Render2D/FontFace2D.cpp
    tests/Render2D/GlyphAtlas.cpp
    tests/Render2D/Golden.cpp
    tests/Render2D/Gradient2D.cpp
    tests/Render2D/Path2D.cpp
    tests/Render2D/PathRasteriser.cpp
    tests/Render2D/PathStroker.cpp
    tests/Render2D/Renderer2D.cpp
    tests/Render2D/Text.cpp
    tests/Render2D/Transform2D.cpp
)
