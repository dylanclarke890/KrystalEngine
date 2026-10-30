#pragma once

#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-width'> = normal | <percentage [0,∞]> | ultra-condensed | extra-condensed | condensed |
  //   semi-condensed | semi-expanded | expanded | extra-expanded | ultra-expanded
  // NOTE: Computed value is always resolved to a <percentage [0,∞]>.
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-width
  struct FontWidth
  {
    using Percentage = style::Percentage<css::NonNegative>;

    FontWidth(css::keywords::Normal) noexcept : _platform {NormalWidthValue()}
    {
    }
    FontWidth(css::keywords::UltraCondensed) noexcept : _platform {UltraCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::ExtraCondensed) noexcept : _platform {ExtraCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::Condensed) noexcept : _platform {CondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::SemiCondensed) noexcept : _platform {SemiCondensedWidthValue()}
    {
    }
    FontWidth(css::keywords::SemiExpanded) noexcept : _platform {SemiExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::Expanded) noexcept : _platform {ExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::ExtraExpanded) noexcept : _platform {ExtraExpandedWidthValue()}
    {
    }
    FontWidth(css::keywords::UltraExpanded) noexcept : _platform {UltraExpandedWidthValue()}
    {
    }
    FontWidth(Percentage percentage) noexcept
        : _platform {FontSelectionValue::ClampFloat(static_cast<float>(percentage.Value))}
    {
    }

    FontWidth(FontSelectionValue platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD Percentage GetPercentage() const noexcept
    {
      return Percentage {static_cast<float>(_platform)};
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      return visitor(GetPercentage());
    }

    KRYS_NODISCARD FontSelectionValue Platform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD bool operator==(const FontWidth &) const noexcept = default;

  private:
    FontSelectionValue _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontWidth>
  {
    KRYS_NODISCARD FontWidth operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontWidth>
  {
    KRYS_NODISCARD FontWidth Blend(const FontWidth &, const FontWidth &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontWidth)
