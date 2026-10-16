#pragma once

#include "Krystal.Booey/Graphics/FontTaggedSettings.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <feature-tag-value> = <opentype-tag> [ <integer [0,∞]> | on | off ]?
  // <'font-feature-settings'> = normal | <feature-tag-value>#
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-feature-settings
  struct FontFeatureSettings
  {
    using Value = Integer<css::NonNegative>;

    FontFeatureSettings(css::keywords::Normal) noexcept : _platform {}
    {
    }
    FontFeatureSettings(boo::FontFeatureSettings &&platform) noexcept : _platform {krys::move(platform)}
    {
    }
    FontFeatureSettings(const boo::FontFeatureSettings &platform) noexcept : _platform {platform}
    {
    }

    KRYS_NODISCARD const boo::FontFeatureSettings &Platform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD boo::FontFeatureSettings TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool operator==(const FontFeatureSettings &) const noexcept = default;

  private:
    boo::FontFeatureSettings _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontFeatureSettings>
  {
    KRYS_NODISCARD FontFeatureSettings operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontFeatureSettings>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontFeatureSettings &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontFeatureSettings>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const FontFeatureSettings &) noexcept;
  };
}
