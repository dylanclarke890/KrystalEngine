add_executable(Krystal.UI.Tests)

target_link_libraries(Krystal.UI.Tests
  PRIVATE
    Krystal.UI
    Catch2::Catch2WithMain
    stb
)

set_target_properties(Krystal.UI.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.UI.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.UI.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8 /bigobj>
)

# The GPU tests use Krystal.Gfx.Tests' test device (header-only), and the editor page's tests its custom
# elements (header-only).
target_include_directories(Krystal.UI.Tests
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/Krystal.Gfx.Tests/include
    ${CMAKE_SOURCE_DIR}/Krystal.Editor/include
)

target_compile_definitions(Krystal.UI.Tests
  PRIVATE
    KRYS_UI_TESTS_FONTS_DIR="${CMAKE_SOURCE_DIR}/third-party/liberation-fonts"
    # The page goldens are kept in the source tree, compared with Krystal.Gfx.Tests' golden helpers.
    KRYS_GFX_TESTS_GOLDEN_DIR="${CMAKE_CURRENT_SOURCE_DIR}/golden"
)

catch_discover_tests(Krystal.UI.Tests)

target_sources(Krystal.UI.Tests
  PRIVATE
    ${CMAKE_SOURCE_DIR}/Krystal.Gfx.Tests/src/Golden.cpp

    tests/Benchmark.cpp
    tests/Compositing.cpp
    tests/Gamepads.cpp
    tests/InputMethod.cpp
    tests/GamepadNavigation.cpp
    tests/GamepadProvider.cpp
    tests/Golden.cpp
    tests/GraphicsContextRender2D.cpp
    tests/View.cpp
    tests/Render2DConversions.cpp
    tests/Repaint.cpp
    tests/TextureMapperAnimation.cpp
)
