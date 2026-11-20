add_executable(Krystal.Net.Tests)

# Third-party - RapidJSON (WPT URL test data)
find_package(RapidJSON CONFIG REQUIRED)

target_link_libraries(Krystal.Net.Tests
  PRIVATE
    Krystal.Net
    Catch2::Catch2WithMain
    rapidjson
)

set_target_properties(Krystal.Net.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Net.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Net.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_compile_definitions(Krystal.Net.Tests PRIVATE
    KRYS_NET_TESTS_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/data")

catch_discover_tests(Krystal.Net.Tests)

target_sources(Krystal.Net.Tests
  PRIVATE
    tests/Http/HttpHeaderMap.cpp
    tests/DataUrlDecoder.cpp
    tests/Http/HttpParsers.cpp
    tests/MimeTypeRegistry.cpp
    tests/ResourceResponse.cpp
    tests/Url.cpp
    tests/WPT.cpp
)
