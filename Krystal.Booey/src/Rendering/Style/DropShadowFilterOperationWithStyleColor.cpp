#include "Krystal.Booey/Rendering/Style/DropShadowFilterOperationWithStyleColor.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  Ref<DropShadowFilterOperation> DropShadowFilterOperationWithStyleColor::CreateEquivalentWithResolvedColor(
    const RenderStyle &style) noexcept
  {
    auto resolvedColor = _styleColor.ResolveColor(style.GetColor());
    return DropShadowFilterOperation::Create(Location(), StdDeviation(), resolvedColor);
  }

  bool DropShadowFilterOperationWithStyleColor::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }
    const auto &other = Downcast<DropShadowFilterOperationWithStyleColor>(operation);
    return NonColorEqual(other) && _styleColor == other._styleColor;
  }

  RefPtr<FilterOperation> DropShadowFilterOperationWithStyleColor::Blend(const FilterOperation *from,
                                                                         const BlendingContext &context,
                                                                         bool blendToPassthrough) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    auto toColor = _styleColor.ResolveColor(context.ToCurrentColor);

    if (blendToPassthrough)
    {
      return DropShadowFilterOperationWithStyleColor::Create(
        boo::Blend(_location, IntPoint(), context), boo::Blend(_stdDeviation, 0, context),
        boo::Blend(toColor, boo::Color::transparentBlack, context));
    }

    const auto *fromOperation = Downcast<DropShadowFilterOperationWithStyleColor>(from);
    IntPoint fromLocation = fromOperation ? fromOperation->Location() : IntPoint();
    int fromStdDeviation = fromOperation ? fromOperation->StdDeviation() : 0;

    auto fromColor = fromOperation ? fromOperation->StyleColor().ResolveColor(context.FromCurrentColor)
                                   : boo::Color::transparentBlack;

    return DropShadowFilterOperationWithStyleColor::Create(
      boo::Blend(fromLocation, _location, context),
      std::max(boo::Blend(fromStdDeviation, _stdDeviation, context), 0),
      boo::Blend(fromColor, toColor, context));
  }
}
