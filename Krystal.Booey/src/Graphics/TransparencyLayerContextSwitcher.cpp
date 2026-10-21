#include "Krystal.Booey/Graphics/TransparencyLayerContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"

namespace krys::boo
{
  TransparencyLayerContextSwitcher::TransparencyLayerContextSwitcher(GraphicsContext &destinationContext,
                                                                     const FloatRect &sourceImageRect,
                                                                     RefPtr<Filter> &&filter) noexcept
      : GraphicsContextSwitcher(krys::move(filter))
  {
    if (_filter)
    {
      _filterStyles = _filter->CreateFilterStyles(destinationContext, sourceImageRect);
    }
  }

  void TransparencyLayerContextSwitcher::BeginClipAndDrawSourceImage(GraphicsContext &destinationContext,
                                                                     const FloatRect &,
                                                                     const FloatRect &clipRect) noexcept
  {
    destinationContext.Save();
    destinationContext.BeginTransparencyLayer(1);

    for (const auto &filterStyle : _filterStyles)
    {
      destinationContext.Save();
      destinationContext.Clip(Intersection(filterStyle.ImageRect, clipRect));
      destinationContext.SetStyle(filterStyle.Style);
      destinationContext.BeginTransparencyLayer(1);
    }
  }

  void TransparencyLayerContextSwitcher::BeginDrawSourceImage(GraphicsContext &destinationContext,
                                                              float opacity) noexcept
  {
    destinationContext.Save();
    destinationContext.BeginTransparencyLayer(opacity);

    for (const auto &filterStyle : _filterStyles)
    {
      destinationContext.Save();
      destinationContext.Clip(filterStyle.ImageRect);
      destinationContext.SetStyle(filterStyle.Style);
      destinationContext.BeginTransparencyLayer(1);
    }
  }

  void TransparencyLayerContextSwitcher::EndDrawSourceImage(GraphicsContext &destinationContext) noexcept
  {
    for (size_t i = 0; i < _filterStyles.size(); ++i)
    {
      destinationContext.EndTransparencyLayer();
      destinationContext.Restore();
    }

    destinationContext.EndTransparencyLayer();
    destinationContext.Restore();
  }
}
