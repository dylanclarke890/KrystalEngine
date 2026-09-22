add_library(Krystal.Gfx.OpenGL)

target_link_libraries(Krystal.Gfx.OpenGL
  PUBLIC 
    Krystal.Core
    Krystal.Gfx
)

set_target_properties(Krystal.Gfx.OpenGL
 PROPERTIES
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
  COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Gfx.OpenGL
 PUBLIC
  cxx_std_23
)

target_compile_options(Krystal.Gfx.OpenGL
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8>
)

target_sources(Krystal.Gfx.OpenGL
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Gfx.OpenGL/Context.hpp
      include/Krystal.Gfx.OpenGL/Debug.hpp
      include/Krystal.Gfx.OpenGL/gl.hpp
      include/Krystal.Gfx.OpenGL/Renderer.hpp
      include/Krystal.Gfx.OpenGL/ShaderReflector.hpp
      include/Krystal.Gfx.OpenGL/Utils.hpp

      # Hooks
      include/Krystal.Gfx.OpenGL/Hooks/gl.hpp

      # Mappers
      include/Krystal.Gfx.OpenGL/Mappers/Enums/BufferBitFlags.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/BufferType.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/BufferUsage.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/FilterMode.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/ImageType.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/PixelFormat.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/PrimitiveType.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/VertexAttributeType.hpp
      include/Krystal.Gfx.OpenGL/Mappers/Enums/WrapMode.hpp

      # Official
      include/Krystal.Gfx.OpenGL/Official/glcorearb.h
      include/Krystal.Gfx.OpenGL/Official/glext.h
      include/Krystal.Gfx.OpenGL/Official/khrplatform.h
      include/Krystal.Gfx.OpenGL/Official/wglext.h

      # Registries
      include/Krystal.Gfx.OpenGL/Registries/BufferRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/FontRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/ImageRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/ImageViewRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/MaterialRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/MeshRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/RenderTargetRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/SamplerRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/ShaderRegistry.hpp
      include/Krystal.Gfx.OpenGL/Registries/TextureRegistry.hpp

      # Resources
      include/Krystal.Gfx.OpenGL/Resources/Buffer.hpp
      include/Krystal.Gfx.OpenGL/Resources/Font.hpp
      include/Krystal.Gfx.OpenGL/Resources/Image.hpp
      include/Krystal.Gfx.OpenGL/Resources/ImageView.hpp
      include/Krystal.Gfx.OpenGL/Resources/Material.hpp
      include/Krystal.Gfx.OpenGL/Resources/Mesh.hpp
      include/Krystal.Gfx.OpenGL/Resources/RenderTarget.hpp
      include/Krystal.Gfx.OpenGL/Resources/Sampler.hpp
      include/Krystal.Gfx.OpenGL/Resources/Shader.hpp

  PRIVATE 
    src/Context.cpp
    src/Renderer.cpp
    
    # Hooks
    src/Hooks/gl.cpp

    # Registries
    src/Registries/FontRegistry.cpp
)

if(WIN32)
  target_sources(Krystal.Gfx.OpenGL 
    PRIVATE
      include/Krystal.Gfx.OpenGL/ContextImpl/Win32.hpp
      include/Krystal.Gfx.OpenGL/Hooks/wgl.hpp
      
      src/ContextImpl/Win32.cpp
      src/Hooks/wgl.cpp
  )
endif()