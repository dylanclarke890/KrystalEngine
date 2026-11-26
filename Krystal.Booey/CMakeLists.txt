add_library(Krystal.Booey STATIC)

target_link_libraries(Krystal.Booey
  PUBLIC
    Krystal.Core
    Krystal.Web
    Krystal.Gfx
    Krystal.PAL
)

set_target_properties(Krystal.Booey
  PROPERTIES
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
    COMPILE_WARNING_AS_ERROR ON
)

target_compile_features(Krystal.Booey
  PUBLIC
    cxx_std_23
)

target_compile_options(Krystal.Booey
  PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /MP /utf-8 /bigobj>
)

target_sources(Krystal.Booey
  PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      include
    FILES
      include/Krystal.Booey/Compositing/BitmapTexture.hpp
      include/Krystal.Booey/Compositing/BitmapTexturePool.hpp
      include/Krystal.Booey/Compositing/EngineTextureLayer.hpp
      include/Krystal.Booey/Compositing/GraphicsLayerTextureMapper.hpp
      include/Krystal.Booey/Compositing/PlatformLayerDisplayDelegate.hpp
      include/Krystal.Booey/Compositing/TextureMapper.hpp
      include/Krystal.Booey/Compositing/TextureMapperAnimation.hpp
      include/Krystal.Booey/Compositing/TextureMapperBackingStore.hpp
      include/Krystal.Booey/Compositing/TextureMapperLayer.hpp
      include/Krystal.Booey/Compositing/TextureMapperPlatformLayer.hpp
      include/Krystal.Booey/Compositing/TextureMapperTile.hpp
      include/Krystal.Booey/Compositing/TextureMapperTiledBackingStore.hpp
      include/Krystal.Booey/EditorClient.hpp
      include/Krystal.Booey/FileLoaderStrategy.hpp
      include/Krystal.Booey/FontFaceCache.hpp
      include/Krystal.Booey/FilterGraph2DBuilder.hpp
      include/Krystal.Booey/GraphicsContextRender2D.hpp
      include/Krystal.Booey/EngineTexture.hpp
      include/Krystal.Booey/ImageBufferRender2DBackend.hpp
      include/Krystal.Booey/TextureImage.hpp
      include/Krystal.Booey/ImageBufferTextures.hpp
      include/Krystal.Booey/Input.hpp
      include/Krystal.Booey/ColorChooserOverlay.hpp
      include/Krystal.Booey/DataListSuggestionsOverlay.hpp
      include/Krystal.Booey/DateTimeChooserOverlay.hpp
      include/Krystal.Booey/Overlay.hpp
      include/Krystal.Booey/PopupMenuOverlay.hpp
      include/Krystal.Booey/PathStrokerStrategy.hpp
      include/Krystal.Booey/Render2DConversions.hpp
      include/Krystal.Booey/SystemPasteboardStrategy.hpp
      include/Krystal.Booey/ValidationMessageOverlay.hpp
      include/Krystal.Booey/View.hpp
      include/Krystal.Booey/ViewAccessibility.hpp

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
    src/FilterGraph2DBuilder.cpp
    src/GraphicsContextRender2D.cpp
    src/EngineTexture.cpp
    src/ImageBufferRender2DBackend.cpp
    src/ImageBufferTextures.cpp
    src/Input.cpp
    src/ColorChooserOverlay.cpp
    src/DataListSuggestionsOverlay.cpp
    src/DateTimeChooserOverlay.cpp
    src/Overlay.cpp
    src/PopupMenuOverlay.cpp
    src/PathStrokerStrategy.cpp
    src/ValidationMessageOverlay.cpp
    src/Render2DConversions.cpp
    src/SystemPasteboardStrategy.cpp
    src/View.cpp
    src/ViewAccessibility.cpp
)
