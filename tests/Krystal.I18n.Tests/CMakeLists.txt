add_executable(Krystal.I18n.Tests)

target_link_libraries(Krystal.I18n.Tests
  PRIVATE
    Krystal.I18n
    Catch2::Catch2WithMain
)

set_target_properties(Krystal.I18n.Tests
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.I18n.Tests
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.I18n.Tests
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

catch_discover_tests(Krystal.I18n.Tests)

target_sources(Krystal.I18n.Tests
  PRIVATE
    tests/DateTimeFormat.cpp
    tests/DefaultLocale.cpp
    tests/Language.cpp
    tests/LocaleICU.cpp
    tests/LocaleToScriptMapping.cpp
)
