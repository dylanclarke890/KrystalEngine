#pragma once

#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  class VariableReferenceValue : public Value
  {
  public:
    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const VariableReferenceValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::VariableReferenceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsVariableReferenceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()