#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-variant-east-asian'> = normal | [ <east-asian-variant-values> || <east-asian-width-values> ||
  //   ruby ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variant-east-asian
  struct FontVariantEastAsian
  {
    using Platform = boo::FontVariantEastAsianValues;

    constexpr FontVariantEastAsian(css::keywords::Normal) noexcept : _platform {}
    {
    }

    constexpr FontVariantEastAsian(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD constexpr Platform GetPlatform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD constexpr bool IsNormal() const noexcept
    {
      return _platform.Variant == FontVariantEastAsianVariant::Normal
             && _platform.Width == FontVariantEastAsianWidth::Normal
             && _platform.Ruby == FontVariantEastAsianRuby::Normal;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNormal())
      {
        return visitor(css::keywords::Normal {});
      }

      auto variantValue = [](auto value)
        -> Maybe<Variant<css::keywords::Jis78, css::keywords::Jis83, css::keywords::Jis90,
                         css::keywords::Jis04, css::keywords::Simplified, css::keywords::Traditional>>
      {
        switch (value)
        {
          case FontVariantEastAsianVariant::Normal:      return std::nullopt;
          case FontVariantEastAsianVariant::Jis78:       return css::keywords::Jis78 {};
          case FontVariantEastAsianVariant::Jis83:       return css::keywords::Jis83 {};
          case FontVariantEastAsianVariant::Jis90:       return css::keywords::Jis90 {};
          case FontVariantEastAsianVariant::Jis04:       return css::keywords::Jis04 {};
          case FontVariantEastAsianVariant::Simplified:  return css::keywords::Simplified {};
          case FontVariantEastAsianVariant::Traditional: return css::keywords::Traditional {};
        }
        krys_unreachable();
      };
      auto widthValue =
        [](auto value) -> Maybe<Variant<css::keywords::FullWidth, css::keywords::ProportionalWidth>>
      {
        switch (value)
        {
          case FontVariantEastAsianWidth::Normal:       return std::nullopt;
          case FontVariantEastAsianWidth::Full:         return css::keywords::FullWidth {};
          case FontVariantEastAsianWidth::Proportional: return css::keywords::ProportionalWidth {};
        }
        krys_unreachable();
      };
      auto rubyValue = [](auto value) -> Maybe<css::keywords::Ruby>
      {
        switch (value)
        {
          case FontVariantEastAsianRuby::Normal: return std::nullopt;
          case FontVariantEastAsianRuby::Yes:    return css::keywords::Ruby {};
        }
        krys_unreachable();
      };

      return visitor(css::SpaceSeparatedTuple {
        variantValue(_platform.Variant),
        widthValue(_platform.Width),
        rubyValue(_platform.Ruby),
      });
    }

    KRYS_NODISCARD constexpr bool operator==(const FontVariantEastAsian &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantEastAsian>
  {
    KRYS_NODISCARD FontVariantEastAsian operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontVariantEastAsian)
