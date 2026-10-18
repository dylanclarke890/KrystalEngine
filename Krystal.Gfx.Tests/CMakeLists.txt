add_executable(Krystal.Gfx.Tests)

target_link_libraries(Krystal.Gfx.Tests
  PRIVATE
    Krystal.Core
    Krystal.Gfx
    Catch2::Catch2WithMain
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

catch_discover_tests(Krystal.Gfx.Tests)

target_sources(Krystal.Gfx.Tests
  PRIVATE
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx.Tests/TestDevice.hpp

  PRIVATE
    tests/CommandBuffer.cpp
    tests/Formats.cpp
    tests/Pool.cpp

    # OpenGL
    tests/OpenGL/Device.cpp

    # Render2D
    tests/Render2D/Path2D.cpp
    tests/Render2D/PathRasteriser.cpp
    tests/Render2D/Renderer2D.cpp
    tests/Render2D/Transform2D.cpp
)
