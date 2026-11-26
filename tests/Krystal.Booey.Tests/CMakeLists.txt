add_executable(Krystal.Booey.Tests)

target_link_libraries(Krystal.Booey.Tests
  PRIVATE
    Krystal.Booey
    Catch2::Catch2WithMain
    stb
)

set_target_properties(Krystal.Booey.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Booey.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Booey.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8 /bigobj>
)

# The GPU tests use Krystal.Gfx.Tests' test device (header-only).
target_include_directories(Krystal.Booey.Tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/tests/Krystal.Gfx.Tests/include
)

target_compile_definitions(Krystal.Booey.Tests
  PRIVATE
    KRYS_BOOEY_TESTS_FONTS_DIR="${CMAKE_SOURCE_DIR}/third-party/liberation-fonts"
    # The page goldens are kept in the source tree, compared with Krystal.Gfx.Tests' golden helpers.
    KRYS_GFX_TESTS_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden"
    # Fresh output (actual images and diffs) goes to the build tree, wherever the tests run from.
    KRYS_GFX_TESTS_GOLDEN_OUTPUT_DIR="${CMAKE_CURRENT_BINARY_DIR}/golden-output"
)

catch_discover_tests(Krystal.Booey.Tests)

target_sources(Krystal.Booey.Tests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/tests/Krystal.Gfx.Tests/src/Golden.cpp

    tests/Accessibility.cpp
    tests/Benchmark.cpp
    tests/BidiEditing.cpp
    tests/Canvas2D.cpp
    tests/ColorSpaces.cpp
    tests/Compositing.cpp
    tests/DragAndDrop.cpp
    tests/InputMethod.cpp
    tests/GamepadNavigation.cpp
    tests/Golden.cpp
    tests/GraphicsContextRender2D.cpp
    tests/View.cpp
    tests/PathStrokerStrategy.cpp
    tests/Render2DConversions.cpp
    tests/Repaint.cpp
    tests/TextureMapperAnimation.cpp
)
