#pragma once

namespace Krys::HTML
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

namespace Krys
{
  template <>
  struct MarkableTraits<Krys::HTML::Color>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const Krys::HTML::Color &color) noexcept
    {
      return std::holds_alternative<Krys::HTML::Color::EmptyToken>(color.value);
    }

    KRYS_NODISCARD constexpr static Krys::HTML::Color EmptyValue() noexcept
    {
      return Krys::HTML::Color(Krys::HTML::Color::EmptyToken());
    }
  };

}