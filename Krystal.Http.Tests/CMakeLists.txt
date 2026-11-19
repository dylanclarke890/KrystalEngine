add_executable(Krystal.Http.Tests)

target_link_libraries(Krystal.Http.Tests
  PRIVATE
    Krystal.Http
    Catch2::Catch2WithMain
)

set_target_properties(Krystal.Http.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Http.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Http.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

catch_discover_tests(Krystal.Http.Tests)

target_sources(Krystal.Http.Tests
  PRIVATE
    tests/HTTPHeaderMap.cpp
    tests/HTTPParsers.cpp
    tests/MIMETypeRegistry.cpp
)
