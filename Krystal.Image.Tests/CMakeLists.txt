add_executable(Krystal.Image.Tests)

target_link_libraries(Krystal.Image.Tests
  PRIVATE
    Krystal.Image
    Catch2::Catch2WithMain
)

set_target_properties(Krystal.Image.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Image.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Image.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

catch_discover_tests(Krystal.Image.Tests)

# The test images, which Krystal.Booey.Tests shares.
target_sources(Krystal.Image.Tests
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Image.Tests/AnimatedImages.hpp
      include/Krystal.Image.Tests/EXIFImages.hpp
      include/Krystal.Image.Tests/ICCImages.hpp
      include/Krystal.Image.Tests/TestImages.hpp

  PRIVATE
    tests/ImageDecoder.cpp
    tests/ImageEncoder.cpp
)
