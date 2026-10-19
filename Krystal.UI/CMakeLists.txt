add_library(Krystal.UI STATIC)

target_link_libraries(Krystal.UI
  PUBLIC
    Krystal.Core
    Krystal.Booey
    Krystal.Gfx
)

set_target_properties(Krystal.UI
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.UI
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.UI
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8 /bigobj>
)

target_sources(Krystal.UI
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.UI/FontFaceCache.hpp
      include/Krystal.UI/GraphicsContextRender2D.hpp
      include/Krystal.UI/Page.hpp
      include/Krystal.UI/Render2DConversions.hpp

  PRIVATE
    src/FontFaceCache.cpp
    src/GraphicsContextRender2D.cpp
    src/Page.cpp
    src/Render2DConversions.cpp
)
