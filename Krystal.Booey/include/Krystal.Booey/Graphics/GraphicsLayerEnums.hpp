#pragma once

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo
{
  enum class GraphicsLayerType : uint8
  {
    Normal,
    Structural, // Supports position and transform only, and doesn't flatten (i.e. behaves like preserves3D is
                // true).
    PageTiledBacking,
    TiledBacking,
    ScrollContainer,
    ScrolledContents,
    Shape
  };

  enum class GraphicsLayerMode : uint8
  {
    PlatformLayer,
    LayerHostingContextId
  };

  enum class GraphicsLayerContentsLayerPurpose : uint8
  {
    None = 0,
    Image,
    Media,
    Canvas,
    BackgroundColor,
    Plugin,
    Model,
    HostedModel,
    Host,
  };

  enum class GraphicsLayerShouldSetNeedsDisplay : bool
  {
    DoNotSet,
    Set
  };

  enum class GraphicsLayerShouldClipToLayer : bool
  {
    DoNotClip,
    Clip
  };

  enum class GraphicsLayerCompositingCoordinatesOrientation : uint8
  {
    TopDown,
    BottomUp
  };

  enum class GraphicsLayerScalingFilter : uint8
  {
    Linear,
    Nearest,
    Trilinear
  };

  enum class GraphicsLayerCustomAppearance : bool
  {
    None,
    ScrollingShadow
  };
}
