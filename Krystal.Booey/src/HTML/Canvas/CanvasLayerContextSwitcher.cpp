#include "Krystal.Booey/HTML/Canvas/CanvasLayerContextSwitcher.hpp"
#include "Krystal.Booey/Graphics/Filters/Filter.hpp"
#include "Krystal.Booey/Graphics/GraphicsContextSwitcher.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext2DBase.hpp"

namespace krys::boo::html
{
  RefPtr<CanvasLayerContextSwitcher> CanvasLayerContextSwitcher::Create(CanvasRenderingContext2DBase &context,
                                                                        const FloatRect &bounds,
                                                                        RefPtr<Filter> &&filter) noexcept
  {
    krys_debug_assert(!bounds.IsEmpty());
    auto *effectiveDrawingContext = context.EffectiveDrawingContext();
    if (!effectiveDrawingContext)
    {
      return nullptr;
    }

    auto targetSwitcher =
      GraphicsContextSwitcher::Create(*effectiveDrawingContext, bounds, krys::move(filter));
    if (!targetSwitcher)
    {
      return nullptr;
    }

    return AdoptRef(*new CanvasLayerContextSwitcher(context, bounds, krys::move(targetSwitcher)));
  }

  CanvasLayerContextSwitcher::CanvasLayerContextSwitcher(
    CanvasRenderingContext2DBase &context, const FloatRect &bounds,
    UniquePtr<GraphicsContextSwitcher> &&targetSwitcher) noexcept
      : _context(context), _effectiveDrawingContext(context.EffectiveDrawingContext()), _bounds(bounds),
        _targetSwitcher(krys::move(targetSwitcher))
  {
    krys_debug_assert(_targetSwitcher);
    krys_debug_assert(_effectiveDrawingContext);
    _targetSwitcher->BeginDrawSourceImage(*_effectiveDrawingContext,
                                          static_cast<float>(context.GlobalAlpha()));
  }

  CanvasLayerContextSwitcher::~CanvasLayerContextSwitcher() noexcept
  {
    _targetSwitcher->EndDrawSourceImage(*_effectiveDrawingContext);
  }

  GraphicsContext *CanvasLayerContextSwitcher::DrawingContext() const noexcept
  {
    if (!_effectiveDrawingContext)
    {
      return nullptr;
    }
    return _targetSwitcher->DrawingContext(*_effectiveDrawingContext);
  }

  FloatBoxExtent CanvasLayerContextSwitcher::Outsets() const noexcept
  {
    return ToFloatBoxExtent(_context.CalculateFilterOutsets(_bounds));
  }
}
