add_library(Krystal.Gfx)

target_link_libraries(Krystal.Gfx 
  PUBLIC
    Krystal.Core
    Krystal.Text

  PRIVATE
    stb
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

# Third-party - FreeType (glyph rasterisation, on Krystal.Text's faces)
find_package(Freetype REQUIRED)

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
      include/Krystal.Gfx/Render2D/AtlasPages.hpp
      include/Krystal.Gfx/Render2D/CoverageBlur.hpp
      include/Krystal.Gfx/Render2D/Dashing.hpp
      include/Krystal.Gfx/Render2D/FilterGraph2D.hpp
      include/Krystal.Gfx/Render2D/FilterRenderer2D.hpp
      include/Krystal.Gfx/Render2D/Flattening.hpp
      include/Krystal.Gfx/Render2D/FontFace2D.hpp
      include/Krystal.Gfx/Render2D/GlyphAtlas.hpp
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

  PRIVATE
    src/CommandBuffer.cpp
    src/Device.cpp

    # OpenGL
    src/OpenGL/GLDevice.cpp
    src/OpenGL/Hooks/gl.cpp

    # Render2D
    src/Render2D/AtlasPages.cpp
    src/Render2D/CoverageBlur.cpp
    src/Render2D/FilterRenderer2D.cpp
    src/Render2D/FontFace2D.cpp
    src/Render2D/GlyphAtlas.cpp
    src/Render2D/Gradient2D.cpp
    src/Render2D/MaskAtlas.cpp
    src/Render2D/Path2D.cpp
    src/Render2D/PathRasteriser.cpp
    src/Render2D/PathStroker.cpp
    src/Render2D/Renderer2D.cpp
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
