#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-variant-alternates'> = normal | [ stylistic(<feature-value-name>) || historical-forms ||
  // styleset(<feature-value-name>#) || character-variant(<feature-value-name>#) ||
  // swash(<feature-value-name>) || ornaments(<feature-value-name>) || annotation(<feature-value-name>) ]
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-variant-alternates
  struct FontVariantAlternates
  {
    using Platform = boo::FontVariantAlternates;

    FontVariantAlternates(css::keywords::Normal) noexcept : _platform {Platform::Normal()}
    {
    }

    FontVariantAlternates(css::keywords::HistoricalForms) noexcept : _platform {Platform::Normal()}
    {
      _platform.ValuesRef().HistoricalForms = true;
    }

    FontVariantAlternates(Platform value) noexcept : _platform {value}
    {
    }

    KRYS_NODISCARD const Platform &GetPlatform() const noexcept
    {
      return _platform;
    }
    KRYS_NODISCARD Platform TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return _platform.IsNormal();
    }

    KRYS_NODISCARD bool operator==(const FontVariantAlternates &) const noexcept = default;

  private:
    Platform _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontVariantAlternates>
  {
    KRYS_NODISCARD FontVariantAlternates operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<FontVariantAlternates>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontVariantAlternates &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<FontVariantAlternates>
  {
    void operator()(css::CSSOMString &, const css::SerialisationContext &, const RenderStyle &,
                    const FontVariantAlternates &) noexcept;
  };
}
