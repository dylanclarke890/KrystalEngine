#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css
{
  class TransformListValue final : public ValueContainingList
  {
  public:
    KRYS_NODISCARD static Ref<TransformListValue> Create(ValueListBuilder values) noexcept;

    KRYS_NODISCARD static Ref<TransformListValue> Create(Ref<Value> value) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const TransformListValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::TransformListValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsTransformListValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()