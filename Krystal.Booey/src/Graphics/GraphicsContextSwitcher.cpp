#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/ImageBufferContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/TransparencyLayerContextSwitcher.hpp"

namespace krys::boo
{
  UniquePtr<GraphicsContextSwitcher> GraphicsContextSwitcher::Create(GraphicsContext &destinationContext,
                                                                     const FloatRect &sourceImageRect,
                                                                     RefPtr<Filter> &&filter,
                                                                     FilterResults *results) noexcept
  {
    if (filter && filter->FilterRenderingModes().Contains(FilterRenderingMode::GraphicsContext))
    {
      return CreateUnique<TransparencyLayerContextSwitcher>(destinationContext, sourceImageRect,
                                                            krys::move(filter));
    }

    return CreateUnique<ImageBufferContextSwitcher>(destinationContext, sourceImageRect, krys::move(filter),
                                                    results);
  }

  GraphicsContextSwitcher::GraphicsContextSwitcher(RefPtr<Filter> &&filter) noexcept
      : _filter(krys::move(filter))
  {
  }
}
