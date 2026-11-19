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
      include/Krystal.Net/Http/HTTPHeaderMap.hpp
      include/Krystal.Net/Http/HTTPParsers.hpp
      include/Krystal.Net/Http/MIMETypeRegistry.hpp
      include/Krystal.Net/URL.hpp
      include/Krystal.Net/URLHash.hpp
      include/Krystal.Net/URLParser.hpp

  PRIVATE
    src/Http/HTTPHeaderMap.cpp
    src/Http/HTTPParsers.cpp
    src/Http/MIMETypeRegistry.cpp
    src/URL.cpp
    src/URLParser.cpp
)
