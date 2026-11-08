add_executable(Krystal.Threading.Tests)

target_link_libraries(Krystal.Threading.Tests
  PRIVATE
    Krystal.Threading
    Catch2::Catch2WithMain
)

set_target_properties(Krystal.Threading.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Threading.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Threading.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

catch_discover_tests(Krystal.Threading.Tests)

target_sources(Krystal.Threading.Tests
  PRIVATE
    tests/CancellationToken.cpp
    tests/JobSystem.cpp
    tests/MainThread.cpp
    tests/TaskTarget.cpp
    tests/Thread.cpp
)
