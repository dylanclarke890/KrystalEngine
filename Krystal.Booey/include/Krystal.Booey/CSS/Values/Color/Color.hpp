#pragma once

#include "Krystal.Booey/CSS/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  // The following color kinds are forward declared and stored in UniqueRefs to
  // avoid unnecessarily growing the size of Color for the uncommon cases.
  struct ColorLayers;
  struct ColorMix;
  struct ContrastColor;
  struct LightDarkColor;

  template <typename Descriptor>
  struct AbsoluteColor;

  template <typename Descriptor>
  struct RelativeColor;

  class Color
  {
  public:
    friend struct MarkableTraits<Color>;

    struct EmptyToken
    {
      constexpr bool operator==(const EmptyToken &) const = default;
    };

    using ColorKind = Variant<EmptyToken, ResolvedColor, KeywordColor, HexColor>;

  private:
    ColorKind value;

    explicit constexpr Color(EmptyToken) noexcept;

  public:
    explicit Color(ResolvedColor color) noexcept;

    explicit Color(KeywordColor color) noexcept;

    explicit Color(HexColor color) noexcept;

    /// @brief Return an absolute color if possible, otherwise an invalid color.
    /// @see https://drafts.csswg.org/css-color-5/#absolute-color
    KRYS_NODISCARD boo::Color AbsoluteColor() const noexcept;

    KRYS_NODISCARD bool operator==(const Color &other) const noexcept;
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