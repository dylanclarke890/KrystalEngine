add_library(Krystal.Gfx)

target_link_libraries(Krystal.Gfx 
  PUBLIC
    Krystal.Core
    Krystal.PAL

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
      include/Krystal.Gfx/Cameras/FirstPersonCamera.hpp
      include/Krystal.Gfx/Enums/BufferBitFlags.hpp
      include/Krystal.Gfx/Enums/BufferType.hpp
      include/Krystal.Gfx/Enums/BufferUsage.hpp
      include/Krystal.Gfx/Enums/FilterMode.hpp
      include/Krystal.Gfx/Enums/ImageType.hpp
      include/Krystal.Gfx/Enums/MeshType.hpp
      include/Krystal.Gfx/Enums/PixelFormat.hpp
      include/Krystal.Gfx/Enums/PrimitiveType.hpp
      include/Krystal.Gfx/Enums/VertexAttributeType.hpp
      include/Krystal.Gfx/Enums/VertexInputRate.hpp
      include/Krystal.Gfx/Enums/WrapMode.hpp
      include/Krystal.Gfx/Registries/IBufferRegistry.hpp
      include/Krystal.Gfx/Registries/IFontRegistry.hpp
      include/Krystal.Gfx/Registries/IImageRegistry.hpp
      include/Krystal.Gfx/Registries/IImageViewRegistry.hpp
      include/Krystal.Gfx/Registries/IMaterialRegistry.hpp
      include/Krystal.Gfx/Registries/IMeshRegistry.hpp
      include/Krystal.Gfx/Registries/IRenderTargetRegistry.hpp
      include/Krystal.Gfx/Registries/ISamplerRegistry.hpp
      include/Krystal.Gfx/Registries/IShaderRegistry.hpp
      include/Krystal.Gfx/Registries/ITextureRegistry.hpp
      include/Krystal.Gfx/Resources/Buffer.hpp
      include/Krystal.Gfx/Resources/Font.hpp
      include/Krystal.Gfx/Resources/Image.hpp
      include/Krystal.Gfx/Resources/ImageView.hpp
      include/Krystal.Gfx/Resources/Material.hpp
      include/Krystal.Gfx/Resources/Mesh.hpp
      include/Krystal.Gfx/Resources/RenderTarget.hpp
      include/Krystal.Gfx/Resources/Sampler.hpp
      include/Krystal.Gfx/Resources/Shader.hpp
      include/Krystal.Gfx/Resources/Texture.hpp
      include/Krystal.Gfx/Utils/MeshDataUtils.hpp
      include/Krystal.Gfx/Colour.hpp
      include/Krystal.Gfx/ColourFormatter.hpp
      include/Krystal.Gfx/ColourHasher.hpp
      include/Krystal.Gfx/Commands.hpp
      include/Krystal.Gfx/FontAtlasLoader.hpp
      include/Krystal.Gfx/Handle.hpp
      include/Krystal.Gfx/ICamera.hpp
      include/Krystal.Gfx/IContext.hpp
      include/Krystal.Gfx/InstanceData.hpp
      include/Krystal.Gfx/IRenderer.hpp
      include/Krystal.Gfx/Light.hpp
      include/Krystal.Gfx/ResourceHandleCache.hpp
      include/Krystal.Gfx/ResourceManager.hpp
      include/Krystal.Gfx/Vertex.hpp

  PRIVATE 
    src/Cameras/FirstPersonCamera.cpp
    src/FontAtlasLoader.cpp
)
