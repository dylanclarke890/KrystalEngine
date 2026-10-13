#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class FontLoadRequest;
  class SVGFontFaceElement;
  class ScriptExecutionContext;

  /// @brief A `local(<family-name>)` source in an `@font-face` rule's `src` descriptor.
  class FontFaceSrcLocalValue final : public Value
  {
  private:
    CSSOMStringAtom _fontFaceName;

    explicit FontFaceSrcLocalValue(CSSOMStringAtom &&fontFaceName) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontFaceSrcLocalValue> Create(CSSOMStringAtom fontFaceName) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _fontFaceName.View().empty();
    }

    KRYS_NODISCARD const CSSOMStringAtom &FontFaceName() const noexcept
    {
      return _fontFaceName;
    }

    // FIXME: not defined until SVG fonts are ported.
    KRYS_NODISCARD SVGFontFaceElement *GetSVGFontFaceElement() const noexcept;
    // FIXME: not defined until SVG fonts are ported.
    void SetSVGFontFaceElement(SVGFontFaceElement &element) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FontFaceSrcLocalValue &other) const noexcept;
  };

  enum class FontTechnology : uint8
  {
    ColorColrv0,
    ColorColrv1,
    ColorCbdt,
    ColorSbix,
    ColorSvg,
    FeaturesAat,
    FeaturesGraphite,
    FeaturesOpentype,
    Incremental,
    Palettes,
    Variations,
    // Reserved for invalid conversion result.
    Invalid
  };

  KRYS_NODISCARD constexpr CSSOMStringView CssTextFromFontTech(FontTechnology tech) noexcept
  {
    switch (tech)
    {
      case FontTechnology::ColorColrv0:      return u8"color-colrv0";
      case FontTechnology::ColorColrv1:      return u8"color-colrv1";
      case FontTechnology::ColorCbdt:        return u8"color-cbdt";
      case FontTechnology::ColorSbix:        return u8"color-sbix";
      case FontTechnology::ColorSvg:         return u8"color-svg";
      case FontTechnology::FeaturesAat:      return u8"features-aat";
      case FontTechnology::FeaturesGraphite: return u8"features-graphite";
      case FontTechnology::FeaturesOpentype: return u8"features-opentype";
      case FontTechnology::Incremental:      return u8"incremental";
      case FontTechnology::Palettes:         return u8"palettes";
      case FontTechnology::Variations:       return u8"variations";
      default:                               return u8"";
    }
  }

  /// @brief A `url()` source, with optional `format()` and `tech()`, in an `@font-face` rule's `src`
  /// descriptor.
  class FontFaceSrcResourceValue final : public Value
  {
  private:
    CSSURL _location;
    CSSOMString _format;
    List<FontTechnology> _technologies;
    // FIXME: add the cached font (CachedResourceHandle<CachedFont>) once the resource loader is ported.

    FontFaceSrcResourceValue(CSSURL &&location, CSSOMString &&format,
                             List<FontTechnology> &&technologies) noexcept;

  public:
    KRYS_NODISCARD static Ref<FontFaceSrcResourceValue> Create(CSSURL location, CSSOMString format,
                                                               List<FontTechnology> &&technologies) noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _location.Specified.empty();
    }

    // FIXME: not defined until the resource loader is ported.
    KRYS_NODISCARD RefPtr<FontLoadRequest>
      GetFontLoadRequest(ScriptExecutionContext &context,
                         bool isInitiatingElementInUserAgentShadowTree) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool CustomTraverseSubresources(
      KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept;
    KRYS_NODISCARD bool CustomMayDependOnBaseUrl() const noexcept;
    KRYS_NODISCARD bool Equals(const FontFaceSrcResourceValue &other) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceSrcLocalValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontFaceSrcLocalValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FontFaceSrcResourceValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFontFaceSrcResourceValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
