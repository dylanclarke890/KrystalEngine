#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"

namespace krys::boo::style
{
  class DropShadowFilterOperationWithStyleColor : public DropShadowFilterOperationBase
  {
  public:
    KRYS_NODISCARD static Ref<DropShadowFilterOperationWithStyleColor>
      Create(const IntPoint &location, int stdDeviation, const style::Color &color) noexcept
    {
      return AdoptRef(*new DropShadowFilterOperationWithStyleColor(location, stdDeviation, color));
    }

    KRYS_NODISCARD Ref<DropShadowFilterOperation>
      CreateEquivalentWithResolvedColor(const RenderStyle &) noexcept;

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new DropShadowFilterOperationWithStyleColor(Location(), StdDeviation(), _styleColor));
    }

    KRYS_NODISCARD const style::Color &StyleColor() const noexcept
    {
      return _styleColor;
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;
    KRYS_NODISCARD bool RequiresRepaintForCurrentColorChange() const noexcept override
    {
      return _styleColor.ContainsCurrentColor();
    }

    DropShadowFilterOperationWithStyleColor(const IntPoint &location, int stdDeviation,
                                            const style::Color &color) noexcept
        : DropShadowFilterOperationBase(Type::DropShadowWithStyleColor, location, stdDeviation),
          _styleColor(color)
    {
    }

    style::Color _styleColor;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::style::DropShadowFilterOperationWithStyleColor,
                                             GetType()
                                               == krys::boo::FilterOperation::Type::DropShadowWithStyleColor)
