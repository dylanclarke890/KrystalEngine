#pragma once

#include "Krystal.Booey/Graphics/Animation/TimingFunction.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo
{
  namespace css
  {
    struct EasingFunction;
  }

  namespace style
  {
    // <easing-function> = linear | <cubic-bezier-easing-function> | <step-easing-function>
    // https://www.w3.org/TR/css-easing-1/#typedef-easing-function
    struct EasingFunction
    {
      Ref<TimingFunction> Value;

      KRYS_NODISCARD bool operator==(const EasingFunction &other) const noexcept
      {
        return Value.get() == other.Value.get() || *Value == *other.Value;
      }
    };

    // MARK: - Deprecated Conversions

    KRYS_NODISCARD Ref<TimingFunction> CreateTimingFunctionDeprecated(const css::EasingFunction &) noexcept;
    KRYS_NODISCARD RefPtr<TimingFunction> CreateTimingFunctionDeprecated(const css::Value &) noexcept;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<EasingFunction>
    {
      KRYS_NODISCARD EasingFunction operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<EasingFunction>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const EasingFunction &) noexcept;
    };

    // MARK: - Serialization

    template <>
    struct Serialiser<EasingFunction>
    {
      void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                      const EasingFunction &) noexcept;
    };
  }
}
