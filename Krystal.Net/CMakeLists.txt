add_library(Krystal.Net STATIC)

target_link_libraries(Krystal.Net
  PUBLIC
    Krystal.Core
  PRIVATE
    # IDNA for URL hosts
    Krystal.Text
)

set_target_properties(Krystal.Net
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Net
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Net
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.Net
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Net/Http/HttpHeaderMap.hpp
      include/Krystal.Net/Http/HttpParsers.hpp
      include/Krystal.Net/Http/MimeTypeRegistry.hpp
      include/Krystal.Net/Url.hpp
      include/Krystal.Net/UrlHash.hpp
      include/Krystal.Net/UrlParser.hpp

  PRIVATE
    src/Http/HttpHeaderMap.cpp
    src/Http/HttpParsers.cpp
    src/Http/MimeTypeRegistry.cpp
    src/Url.cpp
    src/UrlParser.cpp
)
