#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class GridIntegerRepeatValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<GridIntegerRepeatValue> Create(Ref<PrimitiveValue> repetitions,
                                                             ValueListBuilder builder) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const GridIntegerRepeatValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GridIntegerRepeatValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGridIntegerRepeatValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()