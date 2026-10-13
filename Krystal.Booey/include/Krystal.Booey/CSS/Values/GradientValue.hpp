#pragma once

#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleImage;
}

namespace krys::boo::style
{
  class BuilderState;
}

namespace krys::boo::css
{
  /// @brief The value of a gradient function (`linear-gradient()`, `radial-gradient()`, `conic-gradient()`,
  /// their repeating and prefixed forms, and `-webkit-gradient()`).
  class GradientValue final : public Value
  {
  private:
    boo::css::Gradient _gradient;
    // FIXME: add the cached style image (mutable RefPtr<StyleImage>) once StyleImage is ported.

    explicit GradientValue(boo::css::Gradient &&gradient) noexcept
        : Value(ValueType::Gradient), _gradient(krys::move(gradient))
    {
    }

  public:
    KRYS_NODISCARD static Ref<GradientValue> Create(boo::css::Gradient &&gradient) noexcept
    {
      return AdoptRef(*new GradientValue(krys::move(gradient)));
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const GradientValue &other) const noexcept;

    // FIXME: not defined until style::BuilderState and StyleImage are ported.
    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::GradientValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsGradientValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
