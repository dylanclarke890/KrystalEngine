add_library(Krystal.Text STATIC)

target_link_libraries(Krystal.Text
  PUBLIC
    Krystal.Core
)

set_target_properties(Krystal.Text
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Text
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Text
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

# Third-party - ICU (break iteration, character properties, bidi and scripts; its headers are in the public ones)
find_package(ICU REQUIRED COMPONENTS uc dt)
target_link_libraries(Krystal.Text PUBLIC ICU::uc PRIVATE ICU::dt)

# Third-party - FreeType (font files and faces)
find_package(Freetype REQUIRED)
target_link_libraries(Krystal.Text PRIVATE Freetype::Freetype)

# Third-party - HarfBuzz (each face's HarfBuzz face)
find_package(harfbuzz CONFIG REQUIRED)
target_link_libraries(Krystal.Text PRIVATE harfbuzz::harfbuzz)

target_sources(Krystal.Text
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Text/Base64.hpp
      include/Krystal.Text/Bidi.hpp
      include/Krystal.Text/BidiContext.hpp
      include/Krystal.Text/BidiResolver.hpp
      include/Krystal.Text/BidiRunList.hpp
      include/Krystal.Text/CharacterProperties.hpp
      include/Krystal.Text/EncodingRegistry.hpp
      include/Krystal.Text/Encodings/All.hpp
      include/Krystal.Text/Encodings/ASCII.hpp
      include/Krystal.Text/Encodings/Atari.hpp
      include/Krystal.Text/Encodings/Base.hpp
      include/Krystal.Text/Encodings/Big5-HKSCS.hpp
      include/Krystal.Text/Encodings/Decode.hpp
      include/Krystal.Text/Encodings/Encode.hpp
      include/Krystal.Text/Encodings/EncodingId.hpp
      include/Krystal.Text/Encodings/EUC-KR-UHC.hpp
      include/Krystal.Text/Encodings/GB18030.hpp
      include/Krystal.Text/Encodings/IBM.hpp
      include/Krystal.Text/Encodings/ISO-8859.hpp
      include/Krystal.Text/Encodings/Kamenicky.hpp
      include/Krystal.Text/Encodings/KOI8.hpp
      include/Krystal.Text/Encodings/KZ-1048.hpp
      include/Krystal.Text/Encodings/Macintosh.hpp
      include/Krystal.Text/Encodings/PETSCII.hpp
      include/Krystal.Text/Encodings/ShiftJIS.hpp
      include/Krystal.Text/Encodings/Tatar.hpp
      include/Krystal.Text/Encodings/UTF.hpp
      include/Krystal.Text/Encodings/Windows.hpp
      include/Krystal.Text/FreeTypeFace.hpp
      include/Krystal.Text/HarfBuzz.hpp
      include/Krystal.Text/Hyphenation.hpp
      include/Krystal.Text/ICodec.hpp
      include/Krystal.Text/LocaleToScriptMapping.hpp
      include/Krystal.Text/Script.hpp
      include/Krystal.Text/TextBoundaries.hpp
      include/Krystal.Text/TextBreakIterator.hpp
      include/Krystal.Text/TextDirection.hpp
      include/Krystal.Text/UnicodeHelpers.hpp

  PRIVATE
    src/Base64.cpp
    src/Bidi.cpp
    src/BidiContext.cpp
    src/CharacterProperties.cpp
    src/FreeTypeFace.cpp
    src/HarfBuzz.cpp
    src/Hyphenation.cpp
    src/LocaleToScriptMapping.cpp
    src/Script.cpp
    src/TextBoundaries.cpp
    src/TextBreakIterator.cpp
    src/UnicodeHelpers.cpp
)
