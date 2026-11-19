add_library(Krystal.Http STATIC)

target_link_libraries(Krystal.Http
  PUBLIC
    Krystal.Core
)

set_target_properties(Krystal.Http
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Http
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Http
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.Http
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Http/HTTPHeaderMap.hpp
      include/Krystal.Http/HTTPParsers.hpp
      include/Krystal.Http/MIMETypeRegistry.hpp

  PRIVATE
    src/HTTPHeaderMap.cpp
    src/HTTPParsers.cpp
    src/MIMETypeRegistry.cpp
)
