#pragma once

#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::style
{
  /// @brief A computed `color-scheme`.
  struct ColorScheme
  {
    css::SpaceSeparatedList<css::CustomIdentifier> Schemes;
    Maybe<css::keywords::Only> Only;

    ColorScheme(css::keywords::Normal) noexcept : Schemes {}, Only {null}
    {
    }

    ColorScheme(css::SpaceSeparatedList<css::CustomIdentifier> &&schemes,
                Maybe<css::keywords::Only> only) noexcept
        : Schemes {krys::move(schemes)}, Only {only}
    {
    }

    // As an optimization, if `schemes` is empty, that indicates the
    // entire value should be considered `normal`.
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return Schemes.empty();
    }

    KRYS_NODISCARD EnumSet<boo::ColorScheme> GetColorScheme() const noexcept;

    KRYS_NODISCARD bool operator==(const ColorScheme &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const ColorScheme &colorScheme) noexcept
  {
    if constexpr (!I)
    {
      return colorScheme.Schemes;
    }
    else if constexpr (I == 1)
    {
      return colorScheme.Only;
    }
  }

  DEFINE_TYPE_MAPPING(css::ColorScheme, ColorScheme)

  // MARK: - Conversion

  // `ColorScheme` is special-cased to return a `css::ColorSchemeValue`.
  template <>
  struct CSSValueCreation<ColorScheme>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ColorScheme &) noexcept;
  };

  template <>
  struct CSSValueConversion<ColorScheme>
  {
    KRYS_NODISCARD ColorScheme operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Serialisation

  template <>
  struct Serialiser<ColorScheme>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const ColorScheme &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ColorScheme, 2)
