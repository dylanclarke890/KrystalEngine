#pragma once

#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-variation-settings'> = normal | [ <opentype-tag> <number> ]#
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variation-settings
  struct FontVariationSettings
  {
    using Value = style::Number<css::All, float>;

    FontVariationSettings(css::keywords::Normal) noexcept : _platform {}
    {
    }
    FontVariationSettings(boo::FontVariationSettings &&platform) noexcept : _platform {krys::move(platform)}
    {
    }
    FontVariationSettings(const boo::FontVariationSettings &platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD const boo::FontVariationSettings &Platform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD boo::FontVariationSettings TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool operator==(const FontVariationSettings &) const noexcept = default;

  private:
    boo::FontVariationSettings _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariationSettings>
  {
    KRYS_NODISCARD FontVariationSettings operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontVariationSettings>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontVariationSettings &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontVariationSettings>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const FontVariationSettings &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontVariationSettings>
  {
    KRYS_NODISCARD bool CanBlend(const FontVariationSettings &, const FontVariationSettings &) noexcept;
    KRYS_NODISCARD FontVariationSettings Blend(const FontVariationSettings &, const FontVariationSettings &,
                                               const BlendingContext &) noexcept;
  };
}
