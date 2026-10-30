#pragma once

#include "Krystal.Booey/Style/ScopedName.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <single-animation-name> = none | <keyframes-name>
  // https://www.w3.org/TR/css-animations-1/#propdef-animation-name
  struct SingleAnimationName
  {
    SingleAnimationName(css::keywords::None) noexcept : _value {ScopedName {css::CSSOMStringAtom::Null()}}
    {
    }

    SingleAnimationName(ScopedName &&keyframesName) noexcept : _value {krys::move(keyframesName)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _value.Name == css::CSSOMStringAtom::Null();
    }
    KRYS_NODISCARD bool IsKeyframesName() const noexcept
    {
      return _value.Name != css::CSSOMStringAtom::Null();
    }
    KRYS_NODISCARD Maybe<ScopedName> TryKeyframesName() const noexcept
    {
      return IsKeyframesName() ? Maybe<ScopedName> {_value} : std::nullopt;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(_value);
    }

    KRYS_NODISCARD bool operator==(const SingleAnimationName &) const noexcept = default;

  private:
    ScopedName _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SingleAnimationName>
  {
    KRYS_NODISCARD SingleAnimationName operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SingleAnimationName)
