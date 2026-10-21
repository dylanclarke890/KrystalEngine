#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class Filter;
  class GraphicsContext;

  /// @brief Where a filtered source is drawn, and how its result reaches the destination context.
  // FIXME: Drawing the source into an image buffer and applying the filter to it
  // (ImageBufferContextSwitcher), once filters can be applied in software.
  class GraphicsContextSwitcher
  {
    KRYS_NON_COPY_MOVABLE(GraphicsContextSwitcher);

  protected:
    RefPtr<Filter> _filter;

  public:
    /// @brief A switcher that applies @p filter as it's drawn: through the graphics context when the filter
    /// supports it; otherwise null.
    KRYS_NODISCARD static UniquePtr<GraphicsContextSwitcher>
      Create(GraphicsContext &destinationContext, const FloatRect &sourceImageRect,
             RefPtr<Filter> &&filter = nullptr) noexcept;

    virtual ~GraphicsContextSwitcher() noexcept = default;

    KRYS_NODISCARD virtual GraphicsContext *DrawingContext(GraphicsContext &destinationContext) const noexcept
    {
      return &destinationContext;
    }

    KRYS_NODISCARD virtual bool HasSourceImage() const noexcept
    {
      return false;
    }

    virtual void BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                             const FloatRect &repaintRect,
                                             const FloatRect &clipRect) noexcept = 0;
    virtual void EndClipAndDrawSourceImage(GraphicsContext &destinationContext) noexcept = 0;

    virtual void BeginDrawSourceImage(GraphicsContext &destinationContext, float opacity = 1.f) noexcept = 0;
    virtual void EndDrawSourceImage(GraphicsContext &destinationContext) noexcept = 0;

  protected:
    explicit GraphicsContextSwitcher(RefPtr<Filter> &&filter) noexcept;
  };
}
