add_library(Krystal.Gfx.D3D11)

target_link_libraries(Krystal.Gfx.D3D11
  PUBLIC 
    Krystal.Core
    Krystal.Gfx
)

set_target_properties(Krystal.Gfx.D3D11
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Gfx.D3D11
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Gfx.D3D11
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.Gfx.D3D11
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx.D3D11/D3D11Context.hpp

  PRIVATE 
    src/D3D11Context.cpp
)
