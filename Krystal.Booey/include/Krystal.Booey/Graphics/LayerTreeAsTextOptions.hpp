#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  enum class LayerTreeAsTextOptions : uint16
  {
    Debug, // Dump extra debugging info like layer addresses.
    IncludeVisibleRects,
    IncludeTileCaches,
    IncludeRepaintRects,
    IncludePaintingPhases,
    IncludeContentLayers,
    IncludePageOverlayLayers,
    IncludeAcceleratesDrawing,
    IncludeClipping,
    IncludeBackingStoreAttached,
    IncludeRootLayerProperties,
    IncludeEventRegion,
    IncludeExtendedColor,
    IncludeDeviceScale,
    IncludeRootLayers,
  };

  static constexpr EnumSet<LayerTreeAsTextOptions> AllLayerTreeAsTextOptions = {
    LayerTreeAsTextOptions::Debug,
    LayerTreeAsTextOptions::IncludeVisibleRects,
    LayerTreeAsTextOptions::IncludeTileCaches,
    LayerTreeAsTextOptions::IncludeRepaintRects,
    LayerTreeAsTextOptions::IncludePaintingPhases,
    LayerTreeAsTextOptions::IncludeContentLayers,
    LayerTreeAsTextOptions::IncludePageOverlayLayers,
    LayerTreeAsTextOptions::IncludeAcceleratesDrawing,
    LayerTreeAsTextOptions::IncludeClipping,
    LayerTreeAsTextOptions::IncludeBackingStoreAttached,
    LayerTreeAsTextOptions::IncludeRootLayerProperties,
    LayerTreeAsTextOptions::IncludeEventRegion,
    LayerTreeAsTextOptions::IncludeExtendedColor,
  };
}
