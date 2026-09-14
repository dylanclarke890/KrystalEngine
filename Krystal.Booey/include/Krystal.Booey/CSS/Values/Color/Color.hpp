#pragma once

#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::css
{
  class Color
  {
  public:
    friend struct MarkableTraits<Color>;

    struct EmptyToken
    {
      constexpr bool operator==(const EmptyToken &) const = default;
    };

    using ColorKind = Variant<EmptyToken>;

  private:
    ColorKind value;

    explicit constexpr Color(EmptyToken) noexcept;

  public:
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<krys::boo::css::Color>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const krys::boo::css::Color &color) noexcept
    {
      return std::holds_alternative<krys::boo::css::Color::EmptyToken>(color.value);
    }

    KRYS_NODISCARD constexpr static krys::boo::css::Color EmptyValue() noexcept
    {
      return krys::boo::css::Color(krys::boo::css::Color::EmptyToken());
    }
  };
}