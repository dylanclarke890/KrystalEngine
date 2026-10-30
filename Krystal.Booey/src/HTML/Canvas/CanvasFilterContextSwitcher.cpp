#include "Krystal.Booey/HTML/Canvas/CanvasFilterContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasLayerContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  UniquePtr<CanvasFilterContextSwitcher>
    CanvasFilterContextSwitcher::Create(CanvasRenderingContext2DBase &context,
                                        const FloatRect &bounds) noexcept
  {
    if (context.GetState().Filter.IsNone())
    {
      return nullptr;
    }

    auto filter = context.CreateFilter(bounds);
    if (!filter)
    {
      return nullptr;
    }

    auto filterSwitcher = CreateUnique<CanvasFilterContextSwitcher>(context);

    auto targetSwitcher = CanvasLayerContextSwitcher::Create(context, bounds, krys::move(filter));
    if (!targetSwitcher)
    {
      return nullptr;
    }

    context.ModifiableState().TargetSwitcher = krys::move(targetSwitcher);
    return filterSwitcher;
  }

  CanvasFilterContextSwitcher::CanvasFilterContextSwitcher(CanvasRenderingContext2DBase &context) noexcept
      : _context(context)
  {
    context.Save();
    context.RealizeSaves();
  }

  CanvasFilterContextSwitcher::~CanvasFilterContextSwitcher() noexcept
  {
    _context.Restore();
  }

  FloatRect CanvasFilterContextSwitcher::ExpandedBounds() const noexcept
  {
    return _context.GetState().TargetSwitcher->ExpandedBounds();
  }
}
