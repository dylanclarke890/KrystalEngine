add_library(Krystal.I18n STATIC)

target_link_libraries(Krystal.I18n
  PUBLIC
    Krystal.Core
    # Scripts and bidi in the public headers
    Krystal.Text
)

set_target_properties(Krystal.I18n
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.I18n
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.I18n
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Third-party - ICU (locale data: date and number formats, month names, the default locale); only the sources
# include it
find_package(ICU REQUIRED COMPONENTS uc in dt)
target_link_libraries(Krystal.I18n PRIVATE ICU::uc ICU::in ICU::dt)

# Third-party - HarfBuzz (a script's writing direction)
find_package(harfbuzz CONFIG REQUIRED)
target_link_libraries(Krystal.I18n PRIVATE harfbuzz::harfbuzz)

target_sources(Krystal.I18n
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.I18n/DateTimeFields.hpp
      include/Krystal.I18n/DateTimeFormat.hpp
      include/Krystal.I18n/DefaultLocale.hpp
      include/Krystal.I18n/Language.hpp
      include/Krystal.I18n/Locale.hpp
      include/Krystal.I18n/LocaleToScriptMapping.hpp

  PRIVATE
    src/DateTimeFormat.cpp
    src/DefaultLocale.cpp
    src/Language.cpp
    src/Locale.cpp
    src/LocaleICU.cpp
    src/LocaleICU.hpp
    src/LocaleToScriptMapping.cpp
)
