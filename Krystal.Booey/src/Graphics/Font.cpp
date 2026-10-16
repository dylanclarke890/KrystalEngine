#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDescription.hpp"
#include "Krystal.Booey/Graphics/SystemFallbackFontCache.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_ADVANCES_H
#include FT_TRUETYPE_TABLES_H

namespace krys::boo
{
  constexpr float SmallCapsFontSizeMultiplier = 0.7f;
  constexpr float EmphasisMarkFontSizeMultiplier = 0.5f;

  Ref<Font> Font::Create(const FontPlatformData &platformData, Origin origin, FontIsInterstitial interstitial,
                         Visibility visibility, IsOrientationFallback orientationFallback) noexcept
  {
    return AdoptRef(*new Font(platformData, origin, interstitial, visibility, orientationFallback));
  }

  Font::Font(const FontPlatformData &platformData, Origin origin, FontIsInterstitial interstitial,
             Visibility visibility, IsOrientationFallback orientationFallback) noexcept
      : _platformData(platformData), _attributes({origin, interstitial, visibility, orientationFallback})
  {
    PlatformInit();
    PlatformGlyphInit();
    PlatformCharWidthInit();
    // FIXME: Vertical metrics for vertical text (OpenTypeVerticalData), once there's vertical text layout.
  }

  Font::Font(IsSystemFallbackFontPlaceholder isSystemFontFallbackPlaceholder) noexcept
      : _isSystemFontFallbackPlaceholder(isSystemFontFallbackPlaceholder
                                         == IsSystemFallbackFontPlaceholder::Yes)
  {
    // This ctor is to be used only for representing a system font fallback placeholder
    // (createSystemFallbackFontPlaceholder)
    krys_debug_assert(isSystemFontFallbackPlaceholder == IsSystemFallbackFontPlaceholder::Yes);
  }

  Font::~Font() noexcept
  {
    SystemFallbackFontCache::Singleton().Remove(this);
  }

  // Estimates of avgCharWidth and maxCharWidth for platforms that don't support accessing these values from
  // the font.
  void Font::InitCharWidths() noexcept
  {
    const GlyphPage *glyphPageZero = GetGlyphPage(GlyphPage::PageNumberForCodePoint(U'0'));

    // Treat the width of a '0' as the avgCharWidth.
    if (_avgCharWidth <= 0.f && glyphPageZero)
    {
      Glyph digitZeroGlyph = glyphPageZero->GlyphDataForCharacter(U'0').Glyph;
      if (digitZeroGlyph)
      {
        _avgCharWidth = WidthForGlyph(digitZeroGlyph);
      }
    }

    // If we can't retrieve the width of a '0', fall back to the x height.
    if (_avgCharWidth <= 0.f)
    {
      _avgCharWidth = _fontMetrics.XHeight().value_or(0);
    }

    if (_maxCharWidth <= 0.f)
    {
      _maxCharWidth = std::max(_avgCharWidth, _fontMetrics.Ascent());
    }
  }

  void Font::PlatformGlyphInit() noexcept
  {
    const GlyphPage *glyphPageZeroWidthSpace =
      GetGlyphPage(GlyphPage::PageNumberForCodePoint(text::ZeroWidthSpace));
    char32 zeroWidthSpaceCharacter = text::ZeroWidthSpace;
    if (glyphPageZeroWidthSpace)
    {
      _zeroWidthSpaceGlyph = glyphPageZeroWidthSpace->GlyphDataForCharacter(zeroWidthSpaceCharacter).Glyph;
    }

    if (const GlyphPage *page = GetGlyphPage(GlyphPage::PageNumberForCodePoint(text::Space)))
    {
      _spaceGlyph = page->GlyphDataForCharacter(text::Space).Glyph;
    }

    // Force the glyph for ZERO WIDTH SPACE to have zero width, unless it is shared with SPACE.
    // Helvetica is an example of a non-zero width ZERO WIDTH SPACE glyph.
    // See <http://bugs.webkit.org/show_bug.cgi?id=13178> and Font::isZeroWidthSpaceGlyph()
    if (_zeroWidthSpaceGlyph == _spaceGlyph)
    {
      _zeroWidthSpaceGlyph = 0;
    }

    // widthForGlyph depends on m_zeroWidthSpaceGlyph having the correct value.
    // Therefore all calls to widthForGlyph must happen after this point.

    Glyph zeroGlyph = {0};
    if (const GlyphPage *page = GetGlyphPage(GlyphPage::PageNumberForCodePoint(U'0')))
    {
      zeroGlyph = page->GlyphDataForCharacter(U'0').Glyph;
    }
    if (zeroGlyph)
    {
      _fontMetrics.SetZeroWidth(WidthForGlyph(zeroGlyph));
    }

    // Use the width of the CJK water ideogram (U+6C34) as the
    // approximated width of ideograms in the font, as mentioned in
    // https://www.w3.org/TR/css-values-4/#ic. This is currently only used
    // to support the ic unit. If the width is not available, falls back to
    // 1em as specified.
    if (const GlyphPage *page = GetGlyphPage(GlyphPage::PageNumberForCodePoint(text::CjkWater)))
    {
      auto glyph = page->GlyphDataForCharacter(text::CjkWater).Glyph;
      _fontMetrics.SetIdeogramWidth(WidthForGlyph(glyph));
    }
    else
    {
      _fontMetrics.SetIdeogramWidth(PlatformData().Size());
    }

    // spaceWidth() handles adding in the synthetic bold.
    _spaceWidth = WidthForGlyph(_spaceGlyph, SyntheticBoldInclusion::Exclude);
    auto amountToAdjustLineGap = std::min(_fontMetrics.LineGap(), 0.0f);
    _fontMetrics.SetLineGap(_fontMetrics.LineGap() - amountToAdjustLineGap);
    _fontMetrics.SetLineSpacing(_fontMetrics.LineSpacing() - amountToAdjustLineGap);
    DeterminePitch();
  }

  KRYS_NODISCARD static Maybe<size_t> CodePointSupportIndex(char32 codePoint) noexcept
  {
    // FIXME: Consider reordering these so the most common ones are at the front.
    // Doing this could cause the BitVector to fit inside inline storage and therefore
    // be both a performance and a memory progression.
    if (codePoint < 0x20)
    {
      return codePoint;
    }
    if (codePoint >= 0x7F && codePoint < 0xA0)
    {
      return codePoint - 0x7F + 0x20;
    }
    switch (codePoint)
    {
      case text::SoftHyphen:                 return 0x41;
      case text::NewlineCharacter:           return 0x42;
      case text::TabCharacter:               return 0x43;
      case text::NoBreakSpace:               return 0x44;
      case text::NarrowNoBreakSpace:         return 0x45;
      case text::LeftToRightMark:            return 0x46;
      case text::RightToLeftMark:            return 0x47;
      case text::LeftToRightEmbed:           return 0x48;
      case text::RightToLeftEmbed:           return 0x49;
      case text::LeftToRightOverride:        return 0x4A;
      case text::RightToLeftOverride:        return 0x4B;
      case text::LeftToRightIsolate:         return 0x4C;
      case text::RightToLeftIsolate:         return 0x4D;
      case text::ZeroWidthNonJoiner:         return 0x4E;
      case text::ZeroWidthJoiner:            return 0x4F;
      case text::PopDirectionalFormatting:   return 0x50;
      case text::PopDirectionalIsolate:      return 0x51;
      case text::FirstStrongIsolate:         return 0x52;
      case text::ObjectReplacementCharacter: return 0x53;
      case text::ZeroWidthNoBreakSpace:      return 0x54;
      default:                               return null;
    }
  }

  KRYS_NODISCARD static RefPtr<GlyphPage> CreateAndFillGlyphPage(uint32 pageNumber, const Font &font) noexcept
  {
    uint32 glyphPageSize = GlyphPage::SizeForPageNumber(pageNumber);

    char32 start = GlyphPage::StartingCodePointInPageNumber(pageNumber);
    Array<char32, GlyphPage::Size> codePoints;
    for (uint32 i = 0; i < glyphPageSize; i++)
    {
      codePoints[i] = start + i;
    }

    // Now that we have a buffer full of characters, we want to get back an array
    // of glyph indices. This part involves calling into the platform-specific
    // routine of our glyph map for actually filling in the page with the glyphs.
    // Success is not guaranteed. For example, Times fails to fill page 260, giving glyph data
    // for only 128 out of 256 characters.
    Ref glyphPage = GlyphPage::Create(font);

    bool haveGlyphs = glyphPage->Fill(Span<const char32>(codePoints.data(), glyphPageSize));
    if (!haveGlyphs)
    {
      return nullptr;
    }

    return glyphPage;
  }

  const GlyphPage *Font::GetGlyphPage(uint32 pageNumber) const noexcept
  {
    auto [it, isNewEntry] = _glyphPages.try_emplace(pageNumber, nullptr);
    if (isNewEntry)
    {
      it->second = CreateAndFillGlyphPage(pageNumber, *this);
    }
    return it->second.get();
  }

  Glyph Font::GlyphForCharacter(char32 character) const noexcept
  {
    const GlyphPage *page = GetGlyphPage(GlyphPage::PageNumberForCodePoint(character));
    if (!page)
    {
      return 0;
    }
    return page->GlyphForCharacter(character);
  }

  GlyphData Font::GlyphDataForCharacter(char32 character) const noexcept
  {
    const GlyphPage *page = GetGlyphPage(GlyphPage::PageNumberForCodePoint(character));
    if (!page)
    {
      return GlyphData();
    }
    return page->GlyphDataForCharacter(character);
  }

  auto Font::EnsureDerivedFontData() const noexcept -> DerivedFonts &
  {
    if (!_derivedFontData)
    {
      _derivedFontData = CreateUnique<DerivedFonts>();
    }
    return *_derivedFontData;
  }

  const Font &Font::VerticalRightOrientationFont() const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.VerticalRightOrientationFont)
    {
      auto verticalRightPlatformData =
        FontPlatformData::CloneWithOrientation(_platformData, FontOrientation::Horizontal);
      derivedFontData.VerticalRightOrientationFont =
        Create(verticalRightPlatformData, GetOrigin(), FontIsInterstitial::No, Visibility::Visible,
               IsOrientationFallback::Yes);
    }
    krys_debug_assert(derivedFontData.VerticalRightOrientationFont.get() != this);
    return *derivedFontData.VerticalRightOrientationFont;
  }

  const Font &Font::UprightOrientationFont() const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.UprightOrientationFont)
    {
      derivedFontData.UprightOrientationFont = Create(_platformData, GetOrigin(), FontIsInterstitial::No,
                                                      Visibility::Visible, IsOrientationFallback::Yes);
    }
    krys_debug_assert(derivedFontData.UprightOrientationFont.get() != this);
    return *derivedFontData.UprightOrientationFont;
  }

  const Font &Font::InvisibleFont() const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.InvisibleFont)
    {
      derivedFontData.InvisibleFont =
        Create(_platformData, GetOrigin(), FontIsInterstitial::Yes, Visibility::Invisible);
    }
    krys_debug_assert(derivedFontData.InvisibleFont.get() != this);
    return *derivedFontData.InvisibleFont;
  }

  const Font *Font::SmallCapsFont(const FontDescription &fontDescription) const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.SmallCapsFont)
    {
      derivedFontData.SmallCapsFont = CreateScaledFont(fontDescription, SmallCapsFontSizeMultiplier);
    }
    krys_debug_assert(derivedFontData.SmallCapsFont.get() != this);
    return derivedFontData.SmallCapsFont.get();
  }

  GlyphBufferAdvance Font::ApplyTransforms(GlyphBuffer &, uint32, uint32, bool, bool, const StringAtom &,
                                           utf8_stringview, TextDirection) const noexcept
  {
    return MakeGlyphBufferAdvance();
  }

  RefPtr<Font> Font::HalfWidthFont() const noexcept
  {
    if (IsSystemFontFallbackPlaceholder())
    {
      krys_debug_assert_msg(false, "a placeholder has no half width font");
      return nullptr;
    }

    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.HalfWidthFont)
    {
      derivedFontData.HalfWidthFont = CreateHalfWidthFont();
    }
    krys_debug_assert(derivedFontData.HalfWidthFont.get() != this);
    return derivedFontData.HalfWidthFont;
  }

  const Font &Font::NoSynthesizableFeaturesFont() const noexcept
  {
    return *this;
  }

  const Font *Font::EmphasisMarkFont(const FontDescription &fontDescription) const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.EmphasisMarkFont)
    {
      derivedFontData.EmphasisMarkFont = CreateScaledFont(fontDescription, EmphasisMarkFontSizeMultiplier);
    }
    krys_debug_assert(derivedFontData.EmphasisMarkFont.get() != this);
    return derivedFontData.EmphasisMarkFont.get();
  }

  const Font &Font::BrokenIdeographFont() const noexcept
  {
    DerivedFonts &derivedFontData = EnsureDerivedFontData();
    if (!derivedFontData.BrokenIdeographFont)
    {
      auto font = Create(_platformData, GetOrigin(), FontIsInterstitial::No);
      font->_isBrokenIdeographFallback = true;
      derivedFontData.BrokenIdeographFont = krys::move(font);
    }
    krys_debug_assert(derivedFontData.BrokenIdeographFont.get() != this);
    return *derivedFontData.BrokenIdeographFont;
  }

  bool Font::IsProbablyOnlyUsedToRenderIcons() const noexcept
  {
    // FIXME: Not implemented yet.
    return false;
  }

  utf8_string Font::Description() const noexcept
  {
    if (GetOrigin() == Origin::Remote)
    {
      return u8"[custom font]";
    }

    return PlatformData().Description();
  }

  RefPtr<Font> Font::SystemFallbackFontForCharacterCluster(
    utf8_stringview characterCluster, const FontDescription &description,
    ResolvedEmojiPolicy resolvedEmojiPolicy, boo::IsForPlatformFont isForPlatformFont) const noexcept
  {
    return SystemFallbackFontCache::Singleton().SystemFallbackFontForCharacterCluster(
      this, characterCluster, description, resolvedEmojiPolicy, isForPlatformFont);
  }

  RefPtr<Font> Font::CreateScaledFont(const FontDescription &fontDescription,
                                      float scaleFactor) const noexcept
  {
    return PlatformCreateScaledFont(fontDescription, scaleFactor);
  }

  RefPtr<Font> Font::CreateHalfWidthFont() const noexcept
  {
    return PlatformCreateHalfWidthFont();
  }

  bool Font::SupportsCodePoint(char32 character) const noexcept
  {
    // This is very similar to static_cast<bool>(glyphForCharacter(character))
    // except that glyphForCharacter() maps certain code points to ZWS (because they
    // shouldn't be visible). This function doesn't do that mapping, and instead is
    // as honest as possible about what code points the font supports. This is so
    // that we can accurately determine which characters are supported by this font
    // so we know which boundaries to break strings when we send them to the complex
    // text codepath. The complex text codepath is totally separate from this ZWS
    // replacement logic (because CoreText handles those characters instead of WebKit).
    if (auto index = CodePointSupportIndex(character))
    {
      if (_codePointSupport.size() < 2 * (*index + 1))
      {
        _codePointSupport.resize(2 * (*index + 1));
      }
      bool hasBeenSet = _codePointSupport[2 * *index];
      _codePointSupport[2 * *index] = true;
      if (!hasBeenSet && PlatformSupportsCodePoint(character))
      {
        _codePointSupport[2 * *index + 1] = true;
      }
      return _codePointSupport[2 * *index + 1];
    }
    return GlyphForCharacter(character);
  }

  bool Font::CanRenderCombiningCharacterSequence(utf8_stringview text) const noexcept
  {
    text::CodePointIterator it {text};
    while (!it.AtEnd())
    {
      auto codePoint = *it;
      ++it;

      if (!it.AtEnd() && IsVariationSelector(*it))
      {
        if (!PlatformSupportsCodePoint(codePoint, *it))
        {
          // Try the characters individually.
          if (!SupportsCodePoint(codePoint) || !SupportsCodePoint(*it))
          {
            return false;
          }
        }
        ++it;
        continue;
      }

      if (!SupportsCodePoint(codePoint))
      {
        return false;
      }
    }
    return true;
  }

  ColorGlyphType Font::ColorGlyphType(Glyph) const noexcept
  {
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=259205 Determine if the glyph is a color glyph or not.
    return ColorGlyphType::Outline;
  }

  FloatRect Font::BoundsForGlyph(Glyph glyph) const noexcept
  {
    if (IsZeroWidthSpaceGlyph(glyph))
    {
      return FloatRect();
    }

    FloatRect bounds;
    if (_glyphToBoundsMap)
    {
      bounds = _glyphToBoundsMap->MetricsForGlyph(glyph);
      if (bounds.Width() != GlyphSizeUnknown)
      {
        return bounds;
      }
    }

    bounds = PlatformBoundsForGlyph(glyph);
    if (!_glyphToBoundsMap)
    {
      _glyphToBoundsMap = CreateUnique<GlyphMetricsMap<FloatRect>>();
    }
    _glyphToBoundsMap->SetMetricsForGlyph(glyph, bounds);
    return bounds;
  }

  SmallList<FloatRect, Font::InlineGlyphRunCapacity>
    Font::BoundsForGlyphs(Span<const Glyph> glyphs) const noexcept
  {
    SmallList<FloatRect, InlineGlyphRunCapacity> glyphBounds;
    glyphBounds.reserve(glyphs.size());
    for (auto glyph : glyphs)
    {
      glyphBounds.push_back(BoundsForGlyph(glyph));
    }
    return glyphBounds;
  }

  float Font::WidthForGlyph(Glyph glyph, SyntheticBoldInclusion syntheticBoldInclusion) const noexcept
  {
    // The optimization of returning 0 for the zero-width-space glyph is incorrect for the LastResort font,
    // used in place of the actual font when isLoading() is true on both macOS and iOS.
    // The zero-width-space glyph in that font does not have a width of zero and, further, that glyph is used
    // for many other characters and must not be zero width when used for them.
    if (IsZeroWidthSpaceGlyph(glyph) && !IsInterstitial())
    {
      return 0;
    }

    float width = _glyphToWidthMap.MetricsForGlyph(glyph);
    if (width != GlyphSizeUnknown)
    {
      return width
             + (syntheticBoldInclusion == SyntheticBoldInclusion::Incorporate ? SyntheticBoldOffset() : 0);
    }

    width = PlatformWidthForGlyph(glyph);

    _glyphToWidthMap.SetMetricsForGlyph(glyph, width);
    return width
           + (syntheticBoldInclusion == SyntheticBoldInclusion::Incorporate ? SyntheticBoldOffset() : 0);
  }

  // MARK: - FreeType

  /// @brief Converts lengths in font units to pixels at the font's size.
  KRYS_NODISCARD static float Scale(const FontPlatformData &platformData) noexcept
  {
    auto *face = platformData.Face();
    return face && face->UnitsPerEm() ? platformData.Size() / static_cast<float>(face->UnitsPerEm()) : 0;
  }

  /// @brief The ink bounds of @p glyph in font units, with y pointing up, or null if FreeType can't load it.
  KRYS_NODISCARD static Maybe<FloatRect> GlyphBoundsInFontUnits(FT_Face face, Glyph glyph,
                                                                FontOrientation orientation) noexcept
  {
    FT_Int32 flags = FT_LOAD_NO_SCALE | FT_LOAD_NO_HINTING | FT_LOAD_NO_BITMAP;
    if (orientation == FontOrientation::Vertical)
    {
      flags |= FT_LOAD_VERTICAL_LAYOUT;
    }
    if (FT_Load_Glyph(face, glyph, flags))
    {
      return null;
    }
    auto &metrics = face->glyph->metrics;
    return FloatRect(static_cast<float>(metrics.horiBearingX), static_cast<float>(metrics.horiBearingY),
                     static_cast<float>(metrics.width), static_cast<float>(metrics.height));
  }

  void Font::PlatformInit() noexcept
  {
    if (!_platformData.Size() || !_platformData.Face())
    {
      return;
    }

    auto &freeTypeFace = *_platformData.Face();
    FT_Face face = freeTypeFace.Face();
    float scale = Scale(_platformData);

    // Unhinted metrics, so that the sum of ascent and descent isn't larger than the line height.
    float ascent = static_cast<float>(face->ascender) * scale;
    float descent = -static_cast<float>(face->descender) * scale;
    float lineGap = static_cast<float>(face->height - face->ascender + face->descender) * scale;
    Maybe<float> capHeight;
    Maybe<float> xHeight;
    Maybe<unsigned> unitsPerEm = face->units_per_EM;
    Maybe<float> underlinePosition;
    Maybe<float> underlineThickness;

    // If the USE_TYPO_METRICS flag is set in the OS/2 table then we use typo metrics instead.
    if (auto *os2Table = static_cast<TT_OS2 *>(FT_Get_Sfnt_Table(face, FT_SFNT_OS2));
        os2Table && os2Table->version != 0xFFFF)
    {
      constexpr FT_UShort useTypoMetricsMask = 1 << 7;
      if (os2Table->fsSelection & useTypoMetricsMask)
      {
        ascent = static_cast<float>(os2Table->sTypoAscender) * scale;
        descent = -static_cast<float>(os2Table->sTypoDescender) * scale;
        lineGap = static_cast<float>(os2Table->sTypoLineGap) * scale;
      }
      capHeight = static_cast<float>(os2Table->sCapHeight) * scale;
      xHeight = static_cast<float>(os2Table->sxHeight) * scale;
    }

    // The scale factor of the font matrix is the font's size (a synthetic oblique skew doesn't change it).
    underlinePosition =
      -((static_cast<float>(face->underline_position) + static_cast<float>(face->underline_thickness) / 2.f)
        / static_cast<float>(*unitsPerEm))
      * _platformData.Size();
    underlineThickness = (static_cast<float>(face->underline_thickness) / static_cast<float>(*unitsPerEm))
                         * _platformData.Size();

    // We approximate capHeight and xHeight from the glyphs' extents unless the OS/2 table has them.
    auto heightOfCharacter = [&](char32 character) -> float
    {
      auto bounds = GlyphBoundsInFontUnits(face, static_cast<Glyph>(freeTypeFace.GlyphIndex(character)),
                                           _platformData.Orientation());
      if (!bounds)
      {
        return 0;
      }
      return (_platformData.Orientation() == FontOrientation::Horizontal ? bounds->Height() : bounds->Width())
             * scale;
    };
    if (!capHeight.has_value() || !capHeight.value())
    {
      capHeight = heightOfCharacter(U'T');
    }
    if (!xHeight.has_value() || !xHeight.value())
    {
      xHeight = heightOfCharacter(U'x');
    }

    _fontMetrics.SetAscent(ascent);
    _fontMetrics.SetDescent(descent);
    _fontMetrics.SetCapHeight(capHeight.value());
    _fontMetrics.SetLineSpacing(
      static_cast<float>(std::lroundf(ascent) + std::lroundf(descent) + std::lroundf(lineGap)));
    _fontMetrics.SetLineGap(lineGap);
    _fontMetrics.SetXHeight(xHeight.value());
    if (unitsPerEm)
    {
      _fontMetrics.SetUnitsPerEm(unitsPerEm.value());
    }
    if (underlinePosition)
    {
      _fontMetrics.SetUnderlinePosition(underlinePosition.value());
    }
    if (underlineThickness)
    {
      _fontMetrics.SetUnderlineThickness(underlineThickness.value());
    }

    _spaceWidth = PlatformWidthForGlyph(static_cast<Glyph>(freeTypeFace.GlyphIndex(text::Space)));

    _syntheticBoldOffset = _platformData.SyntheticBold() ? 1.0f : 0.f;

    // Disable antialiasing for the Ahem font because many tests require this.
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(freeTypeFace.FamilyName(), u8"Ahem"))
    {
      _allowsAntialiasing = false;
    }
  }

  void Font::PlatformCharWidthInit() noexcept
  {
    _avgCharWidth = 0.f;
    _maxCharWidth = 0.f;
    InitCharWidths();
  }

  RefPtr<Font> Font::PlatformCreateScaledFont(const FontDescription &fontDescription,
                                              float scaleFactor) const noexcept
  {
    auto *face = _platformData.Face();
    krys_debug_assert(face);
    return Font::Create(FontPlatformData(ShareRef(*face), scaleFactor * _platformData.Size(),
                                         _platformData.SyntheticBold(), _platformData.SyntheticOblique(),
                                         fontDescription.Orientation()),
                        GetOrigin(), FontIsInterstitial::No);
  }

  RefPtr<Font> Font::PlatformCreateHalfWidthFont() const noexcept
  {
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=281333 : implement half width font for this platform.
    return nullptr;
  }

  void Font::DeterminePitch() noexcept
  {
    _treatAsFixedPitch = _platformData.IsFixedPitch();
  }

  bool Font::VariantCapsSupportedForSynthesis(FontVariantCaps fontVariantCaps) const noexcept
  {
    switch (fontVariantCaps)
    {
      case FontVariantCaps::Small:
      case FontVariantCaps::Petite:
      case FontVariantCaps::AllSmall:
      case FontVariantCaps::AllPetite: return false;
      default:
        // Synthesis only supports the variant-caps values listed above.
        return true;
    }
  }

  bool Font::PlatformSupportsCodePoint(char32 character, Maybe<char32> variation) const noexcept
  {
    auto *face = _platformData.Face();
    if (!face)
    {
      return false;
    }
    return variation ? !!face->GlyphIndex(character, *variation) : !!face->GlyphIndex(character);
  }

  FloatRect Font::PlatformBoundsForGlyph(Glyph glyph) const noexcept
  {
    if (!_platformData.Size() || !_platformData.Face())
    {
      return FloatRect();
    }

    auto bounds = GlyphBoundsInFontUnits(_platformData.Face()->Face(), glyph, _platformData.Orientation());
    if (!bounds)
    {
      return FloatRect();
    }

    // Font units have y pointing up; glyph bounds have it pointing down, from the baseline.
    float scale = Scale(_platformData);
    float left = bounds->X() * scale;
    float top = -bounds->Y() * scale;
    float width = bounds->Width() * scale;
    float height = bounds->Height() * scale;

    if (_platformData.SyntheticOblique())
    {
      // A synthetic oblique skews the glyph to the right as it rises.
      float skew = std::tan(FontCascade::SyntheticObliqueAngle() * std::numbers::pi_v<float> / 180.f);
      float shiftAtTop = -top * skew;
      float shiftAtBottom = -(top + height) * skew;
      left += std::min(shiftAtTop, shiftAtBottom);
      width += std::abs(shiftAtTop - shiftAtBottom);
    }

    return FloatRect(left, top, width, height);
  }

  float Font::PlatformWidthForGlyph(Glyph glyph) const noexcept
  {
    if (!_platformData.Size() || !_platformData.Face())
    {
      return 0;
    }

    FT_Int32 flags = FT_LOAD_NO_SCALE;
    if (_platformData.Orientation() == FontOrientation::Vertical)
    {
      flags |= FT_LOAD_VERTICAL_LAYOUT;
    }
    FT_Fixed advance = 0;
    if (FT_Get_Advance(_platformData.Face()->Face(), glyph, flags, &advance))
    {
      return _spaceWidth;
    }
    float width = static_cast<float>(advance) * Scale(_platformData);
    return width ? width : _spaceWidth;
  }

  bool GlyphPage::Fill(Span<const char32> codePoints) noexcept
  {
    const Font &font = GetFont();
    auto *face = font.PlatformData().Face();
    if (!face)
    {
      return false;
    }

    Maybe<Glyph> zeroWidthSpaceGlyphValue;
    auto zeroWidthSpaceGlyph = [&]
    {
      if (!zeroWidthSpaceGlyphValue)
      {
        zeroWidthSpaceGlyphValue = static_cast<Glyph>(face->GlyphIndex(text::ZeroWidthSpace));
      }
      return *zeroWidthSpaceGlyphValue;
    };

    bool haveGlyphs = false;
    for (uint32 i = 0; i < GlyphPage::Size && i < codePoints.size(); i++)
    {
      char32 character = codePoints[i];
      auto glyph =
        static_cast<Glyph>(face->GlyphIndex(FontCascade::TreatAsSpace(character) ? text::Space : character));
      // If the font doesn't support a Default_Ignorable character, replace it with zero with space.
      if (!glyph && (IsDefaultIgnorableCodePoint(character) || IsControlCharacter(character)))
      {
        glyph = zeroWidthSpaceGlyph();
      }

      // FIXME: https://bugs.webkit.org/show_bug.cgi?id=259205 Determine if the glyph is a color glyph or not.
      if (!glyph)
      {
        SetGlyphForIndex(i, 0, ColorGlyphType::Outline);
      }
      else
      {
        SetGlyphForIndex(i, glyph, font.ColorGlyphType(glyph));
        haveGlyphs = true;
      }
    }
    return haveGlyphs;
  }
}
