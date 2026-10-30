#pragma once

#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  /// @brief Platform specific interface for attaching contents to GraphicsLayer.
  /// Responsible for creating compositor resources to show the particular contents in the platform specific
  /// GraphicsLayer.
  /// @note The host's compositor subclasses it with what its layers show, and its `GraphicsLayer`s take the
  /// subclasses it made.
  class GraphicsLayerContentsDisplayDelegate : public RefCounted<GraphicsLayerContentsDisplayDelegate>
  {
    KRYS_NON_COPY_MOVABLE(GraphicsLayerContentsDisplayDelegate);

  public:
    virtual ~GraphicsLayerContentsDisplayDelegate() noexcept = default;

  protected:
    GraphicsLayerContentsDisplayDelegate() noexcept = default;
  };
}
