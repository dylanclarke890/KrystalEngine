#pragma once

#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  // TODO: this shouldn't be forward declared but we haven't implemented Gradient yet.
  struct Gradient;

  class GradientValue final : public Value
  {
  public:
    KRYS_NODISCARD static Ref<GradientValue> Create(Gradient gradient) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const GradientValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(const Func<IterationStatus(css::Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GradientValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGradientValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()