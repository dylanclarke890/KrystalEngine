#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class GraphicsLayer;
  class GraphicsLayerClient;
  enum class GraphicsLayerType : uint8;

  /// @brief Creates the platform's graphics layers.
  class GraphicsLayerFactory
  {
  public:
    virtual ~GraphicsLayerFactory() noexcept = default;

    KRYS_NODISCARD virtual Ref<GraphicsLayer> CreateGraphicsLayer(GraphicsLayerType,
                                                                  GraphicsLayerClient &) noexcept = 0;
  };
}
