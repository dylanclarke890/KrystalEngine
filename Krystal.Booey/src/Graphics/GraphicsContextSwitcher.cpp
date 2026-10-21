#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/TransparencyLayerContextSwitcher.hpp"

namespace krys::boo
{
  UniquePtr<GraphicsContextSwitcher> GraphicsContextSwitcher::Create(GraphicsContext &destinationContext,
                                                                     const FloatRect &sourceImageRect,
                                                                     RefPtr<Filter> &&filter) noexcept
  {
    if (filter && filter->FilterRenderingModes().Contains(FilterRenderingMode::GraphicsContext))
    {
      return CreateUnique<TransparencyLayerContextSwitcher>(destinationContext, sourceImageRect,
                                                            krys::move(filter));
    }

    // FIXME: Drawing into an image buffer and applying the filter there (ImageBufferContextSwitcher), once
    // filters can be applied in software.
    return nullptr;
  }

  GraphicsContextSwitcher::GraphicsContextSwitcher(RefPtr<Filter> &&filter) noexcept
      : _filter(krys::move(filter))
  {
  }
}
