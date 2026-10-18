add_library(Krystal.Gfx)

target_link_libraries(Krystal.Gfx 
  PUBLIC
    Krystal.Core

  PRIVATE
    stb
    msdf-atlas-gen::msdf-atlas-gen
    msdfgen::msdfgen
    msdfgen::msdfgen-ext
    Freetype::Freetype
)

set_target_properties(Krystal.Gfx
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Gfx
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Gfx
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

find_package(Freetype REQUIRED)
find_package(msdfgen CONFIG REQUIRED)

set(MSDF_ATLAS_USE_VCPKG        OFF CACHE BOOL "" FORCE)  # we already have msdfgen/freetype from vcpkg
set(MSDF_ATLAS_USE_SKIA         OFF CACHE BOOL "" FORCE)  # no skia
set(MSDF_ATLAS_MSDFGEN_EXTERNAL ON  CACHE BOOL "" FORCE)  # use find_package(msdfgen)
set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "" FORCE)  # no CLI
set(MSDF_ATLAS_INSTALL          OFF CACHE BOOL "" FORCE)  # source consumption
set(MSDF_ATLAS_NO_ARTERY_FONT   ON  CACHE BOOL "" FORCE)  # drops artery-font-format submodule
set(MSDF_ATLAS_DYNAMIC_RUNTIME  ON  CACHE BOOL "" FORCE)  # use dynamic runtime for msdf-atlas-gen
add_subdirectory(third-party/msdf-atlas-gen)

target_sources(Krystal.Gfx
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx/Buffer.hpp
      include/Krystal.Gfx/CommandBuffer.hpp
      include/Krystal.Gfx/Commands.hpp
      include/Krystal.Gfx/CompareOp.hpp
      include/Krystal.Gfx/Device.hpp
      include/Krystal.Gfx/Formats.hpp
      include/Krystal.Gfx/Handles.hpp
      include/Krystal.Gfx/Pipeline.hpp
      include/Krystal.Gfx/Pool.hpp
      include/Krystal.Gfx/RenderPass.hpp
      include/Krystal.Gfx/Sampler.hpp
      include/Krystal.Gfx/Shader.hpp
      include/Krystal.Gfx/Swapchain.hpp
      include/Krystal.Gfx/Texture.hpp

      # OpenGL
      include/Krystal.Gfx/OpenGL/gl.hpp
      include/Krystal.Gfx/OpenGL/GLContext.hpp
      include/Krystal.Gfx/OpenGL/GLDevice.hpp
      include/Krystal.Gfx/OpenGL/GLMappings.hpp
      include/Krystal.Gfx/OpenGL/ShaderReflector.hpp
      include/Krystal.Gfx/OpenGL/Hooks/gl.hpp
      include/Krystal.Gfx/OpenGL/Official/glcorearb.h
      include/Krystal.Gfx/OpenGL/Official/glext.h
      include/Krystal.Gfx/OpenGL/Official/khrplatform.h

      # Render2D
      include/Krystal.Gfx/Render2D/Dashing.hpp
      include/Krystal.Gfx/Render2D/Flattening.hpp
      include/Krystal.Gfx/Render2D/Gradient2D.hpp
      include/Krystal.Gfx/Render2D/MaskAtlas.hpp
      include/Krystal.Gfx/Render2D/Paint2D.hpp
      include/Krystal.Gfx/Render2D/Path2D.hpp
      include/Krystal.Gfx/Render2D/PathRasteriser.hpp
      include/Krystal.Gfx/Render2D/PathStroker.hpp
      include/Krystal.Gfx/Render2D/Rect2D.hpp
      include/Krystal.Gfx/Render2D/Renderer2D.hpp
      include/Krystal.Gfx/Render2D/RoundedRect2D.hpp
      include/Krystal.Gfx/Render2D/StrokeStyle.hpp
      include/Krystal.Gfx/Render2D/Transform2D.hpp

      # Text
      include/Krystal.Gfx/Text/FontAtlas.hpp
      include/Krystal.Gfx/Text/FontAtlasLoader.hpp

  PRIVATE
    src/CommandBuffer.cpp
    src/Device.cpp

    # OpenGL
    src/OpenGL/GLDevice.cpp
    src/OpenGL/Hooks/gl.cpp

    # Render2D
    src/Render2D/Gradient2D.cpp
    src/Render2D/MaskAtlas.cpp
    src/Render2D/Path2D.cpp
    src/Render2D/PathRasteriser.cpp
    src/Render2D/PathStroker.cpp
    src/Render2D/Renderer2D.cpp

    # Text
    src/Text/FontAtlasLoader.cpp
)

if(WIN32)
  target_sources(Krystal.Gfx
    PRIVATE
      include/Krystal.Gfx/OpenGL/Hooks/wgl.hpp
      include/Krystal.Gfx/OpenGL/Official/wglext.h

      src/OpenGL/GLContextWin32.cpp
      src/OpenGL/Hooks/wgl.cpp
  )
endif()
