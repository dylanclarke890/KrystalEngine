#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class Filter;
  class GraphicsContext;
  class GraphicsContextSwitcher;
}

namespace krys::boo::html
{
  class CanvasRenderingContext2DBase;

  /// @brief Redirects a 2D context's drawing into a layer that's filtered as it's drawn back into the canvas,
  /// when the switcher goes.
  class CanvasLayerContextSwitcher : public RefCounted<CanvasLayerContextSwitcher>
  {
  private:
    CanvasRenderingContext2DBase &_context;
    GraphicsContext *_effectiveDrawingContext;
    FloatRect _bounds;
    UniquePtr<GraphicsContextSwitcher> _targetSwitcher;

  public:
    /// @brief A switcher for drawing within @p bounds through @p filter; null if the context has nowhere to
    /// draw.
    KRYS_NODISCARD static RefPtr<CanvasLayerContextSwitcher>
      Create(CanvasRenderingContext2DBase &, const FloatRect &bounds, RefPtr<Filter> &&) noexcept;
    ~CanvasLayerContextSwitcher() noexcept;

    /// @brief Where the context draws while the switcher is in place.
    KRYS_NODISCARD GraphicsContext *DrawingContext() const noexcept;
    /// @brief The bounds, with the filter's outsets.
    KRYS_NODISCARD FloatRect ExpandedBounds() const noexcept
    {
      return _bounds + Outsets();
    }

  private:
    CanvasLayerContextSwitcher(CanvasRenderingContext2DBase &, const FloatRect &bounds,
                               UniquePtr<GraphicsContextSwitcher> &&) noexcept;

    KRYS_NODISCARD FloatBoxExtent Outsets() const noexcept;
  };
}
