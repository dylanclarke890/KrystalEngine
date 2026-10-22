add_library(Krystal.UI STATIC)

target_link_libraries(Krystal.UI
  PUBLIC
    Krystal.Core
    Krystal.Booey
    Krystal.Gfx
    Krystal.PAL
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
      include/Krystal.UI/Compositing/BitmapTexture.hpp
      include/Krystal.UI/Compositing/BitmapTexturePool.hpp
      include/Krystal.UI/Compositing/GraphicsLayerTextureMapper.hpp
      include/Krystal.UI/Compositing/TextureMapper.hpp
      include/Krystal.UI/Compositing/TextureMapperAnimation.hpp
      include/Krystal.UI/Compositing/TextureMapperBackingStore.hpp
      include/Krystal.UI/Compositing/TextureMapperLayer.hpp
      include/Krystal.UI/Compositing/TextureMapperPlatformLayer.hpp
      include/Krystal.UI/Compositing/TextureMapperTile.hpp
      include/Krystal.UI/Compositing/TextureMapperTiledBackingStore.hpp
      include/Krystal.UI/EditorClient.hpp
      include/Krystal.UI/FileLoaderStrategy.hpp
      include/Krystal.UI/FontFaceCache.hpp
      include/Krystal.UI/GraphicsContextRender2D.hpp
      include/Krystal.UI/ImageBufferTextures.hpp
      include/Krystal.UI/Input.hpp
      include/Krystal.UI/Render2DConversions.hpp
      include/Krystal.UI/SystemPasteboardStrategy.hpp
      include/Krystal.UI/View.hpp

  PRIVATE
    src/Compositing/BitmapTexture.cpp
    src/Compositing/BitmapTexturePool.cpp
    src/Compositing/GraphicsLayerTextureMapper.cpp
    src/Compositing/TextureMapper.cpp
    src/Compositing/TextureMapperAnimation.cpp
    src/Compositing/TextureMapperBackingStore.cpp
    src/Compositing/TextureMapperLayer.cpp
    src/Compositing/TextureMapperTile.cpp
    src/Compositing/TextureMapperTiledBackingStore.cpp
    src/EditorClient.cpp
    src/FileLoaderStrategy.cpp
    src/FontFaceCache.cpp
    src/GraphicsContextRender2D.cpp
    src/ImageBufferTextures.cpp
    src/Input.cpp
    src/Render2DConversions.cpp
    src/SystemPasteboardStrategy.cpp
    src/View.cpp
)
