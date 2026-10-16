#include "Krystal.Booey/Graphics/FontCascadeFonts.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/CSS/WebKitFontFamilyNames.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Core/Assert.hpp"
#include <unicode/utf8.h>

namespace krys::boo
{
  /// @brief A glyph page whose glyphs come from different fonts: a page of the primary font with the glyphs
  /// fallback found for the characters it lacks.
  class MixedFontGlyphPage
  {
  private:
    Array<Glyph, GlyphPage::Size> _glyphs = {};
    Array<WeakPtr<const Font>, GlyphPage::Size> _fonts = {};

  public:
    explicit MixedFontGlyphPage(const GlyphPage *initialPage) noexcept
    {
      if (initialPage)
      {
        for (uint32 i = 0; i < GlyphPage::Size; ++i)
        {
          SetGlyphDataForIndex(i, initialPage->GlyphDataForIndex(i));
        }
      }
    }

    KRYS_NODISCARD GlyphData GlyphDataForCharacter(char32 c) const noexcept
    {
      uint32 index = GlyphPage::IndexForCodePoint(c);
      return {_glyphs[index], _fonts[index].get()};
    }

    void SetGlyphDataForCharacter(char32 c, GlyphData glyphData) noexcept
    {
      SetGlyphDataForIndex(GlyphPage::IndexForCodePoint(c), glyphData);
    }

  private:
    void SetGlyphDataForIndex(uint32 index, const GlyphData &glyphData) noexcept
    {
      _glyphs[index] = glyphData.Glyph;
      _fonts[index] = glyphData.Font;
    }
  };

  FontCascadeFonts::GlyphPageCacheEntry::GlyphPageCacheEntry() noexcept = default;

  FontCascadeFonts::GlyphPageCacheEntry::GlyphPageCacheEntry(RefPtr<GlyphPage> &&singleFont) noexcept
      : _singleFont(krys::move(singleFont))
  {
  }

  FontCascadeFonts::GlyphPageCacheEntry::GlyphPageCacheEntry(GlyphPageCacheEntry &&) noexcept = default;
  auto FontCascadeFonts::GlyphPageCacheEntry::operator=(GlyphPageCacheEntry &&) noexcept
    -> GlyphPageCacheEntry & = default;
  FontCascadeFonts::GlyphPageCacheEntry::~GlyphPageCacheEntry() noexcept = default;

  GlyphData FontCascadeFonts::GlyphPageCacheEntry::GlyphDataForCharacter(char32 character) const noexcept
  {
    krys_debug_assert(!(_singleFont && _mixedFont));
    if (_singleFont)
    {
      return _singleFont->GlyphDataForCharacter(character);
    }
    if (_mixedFont)
    {
      return _mixedFont->GlyphDataForCharacter(character);
    }
    return GlyphData();
  }

  void FontCascadeFonts::GlyphPageCacheEntry::SetGlyphDataForCharacter(char32 character,
                                                                       GlyphData glyphData) noexcept
  {
    krys_debug_assert(!GlyphDataForCharacter(character).IsValid());
    if (!_mixedFont)
    {
      _mixedFont = CreateUnique<MixedFontGlyphPage>(_singleFont.get());
      _singleFont = nullptr;
    }
    _mixedFont->SetGlyphDataForCharacter(character, glyphData);
  }

  void FontCascadeFonts::GlyphPageCacheEntry::SetSingleFontPage(RefPtr<GlyphPage> &&page) noexcept
  {
    krys_debug_assert(IsNull());
    _singleFont = krys::move(page);
  }

  FontCascadeFonts::FontCascadeFonts() noexcept : _generation(FontCache::Singleton().Generation())
  {
  }

  FontCascadeFonts::FontCascadeFonts(const FontPlatformData &platformData) noexcept
      : _generation(FontCache::Singleton().Generation()), _isForPlatformFont(true)
  {
    _realizedFallbackRanges.push_back(FontRanges(FontCache::Singleton().FontForPlatformData(platformData)));
  }

  FontCascadeFonts::~FontCascadeFonts() noexcept = default;

  bool FontCascadeFonts::IsFixedPitch(const FontCascadeDescription &description,
                                      FontSelector *fontSelector) noexcept
  {
    if (_pitch == UnknownPitch)
    {
      DeterminePitch(description, fontSelector);
    }
    return _pitch == FixedPitch;
  }

  bool FontCascadeFonts::CanTakeFixedPitchFastContentMeasuring(const FontCascadeDescription &description,
                                                               FontSelector *fontSelector) noexcept
  {
    if (!_canTakeFixedPitchFastContentMeasuring)
    {
      DetermineCanTakeFixedPitchFastContentMeasuring(description, fontSelector);
    }
    return *_canTakeFixedPitchFastContentMeasuring;
  }

  const Font &FontCascadeFonts::PrimaryFont(const FontCascadeDescription &description,
                                            FontSelector *fontSelector) noexcept
  {
    if (!_cachedPrimaryFont)
    {
      auto &primaryRanges = RealizeFallbackRangesAt(description, fontSelector, 0);
      _cachedPrimaryFont =
        primaryRanges.GlyphDataForCharacter(U' ', ExternalResourceDownloadPolicy::Allow).Font;
      if (!_cachedPrimaryFont)
      {
        _cachedPrimaryFont = WeakPtr<const Font>::WithRef(
          primaryRanges.RangeAt(0).GetFont(ExternalResourceDownloadPolicy::Allow));
      }
      else if (_cachedPrimaryFont->IsInterstitial())
      {
        for (uint32 index = 1;; ++index)
        {
          auto &localRanges = RealizeFallbackRangesAt(description, fontSelector, index);
          if (localRanges.IsNull())
          {
            break;
          }
          auto font = localRanges.GlyphDataForCharacter(U' ', ExternalResourceDownloadPolicy::Forbid).Font;
          if (font && !font->IsInterstitial())
          {
            _cachedPrimaryFont = krys::move(font);
            break;
          }
        }
      }
    }
    krys_debug_assert(_cachedPrimaryFont);
    return *_cachedPrimaryFont;
  }

  void FontCascadeFonts::DeterminePitch(const FontCascadeDescription &description,
                                        FontSelector *fontSelector) noexcept
  {
    auto &primaryRanges = RealizeFallbackRangesAt(description, fontSelector, 0);
    uint32 numRanges = primaryRanges.Size();
    if (numRanges == 1)
    {
      _pitch = primaryRanges.FontForFirstRange().GetPitch();
    }
    else
    {
      _pitch = VariablePitch;
    }
  }

  void FontCascadeFonts::DetermineCanTakeFixedPitchFastContentMeasuring(const FontCascadeDescription &,
                                                                        FontSelector *) noexcept
  {
    _canTakeFixedPitchFastContentMeasuring = false;
  }

  bool FontCascadeFonts::IsLoadingCustomFonts() const noexcept
  {
    for (auto &fontRanges : _realizedFallbackRanges)
    {
      if (fontRanges.IsLoading())
      {
        return true;
      }
    }
    return false;
  }

  KRYS_NODISCARD static FontRanges RealizeNextFallback(const FontCascadeDescription &description,
                                                       uint32 &index, FontSelector *fontSelector) noexcept
  {
    krys_debug_assert(index < description.EffectiveFamilyCount());

    auto &fontCache = FontCache::Singleton();
    while (index < description.EffectiveFamilyCount())
    {
      const auto &family = description.EffectiveFamilyAt(index++);
      if (family == StringAtom::Null())
      {
        continue;
      }
      if (fontSelector)
      {
        auto ranges = fontSelector->FontRangesForFamily(description, family);
        if (!ranges.IsNull())
        {
          return ranges;
        }
      }
      if (auto font = fontCache.FontForFamily(description, family))
      {
        return FontRanges(krys::move(font));
      }
    }
    // We didn't find a font. Try to find a similar font using our own specific knowledge about our platform.
    // For example on macOS, we know to map any families containing the words Arabic, Pashto, or Urdu to the
    // Geeza Pro font.
    for (auto &family : description.Families())
    {
      if (auto font = fontCache.SimilarFont(description, family))
      {
        return FontRanges(krys::move(font));
      }
    }
    return {};
  }

  const FontRanges &FontCascadeFonts::RealizeFallbackRangesAt(const FontCascadeDescription &description,
                                                              FontSelector *fontSelector,
                                                              uint32 index) noexcept
  {
    if (index < _realizedFallbackRanges.size())
    {
      return _realizedFallbackRanges[index];
    }

    krys_debug_assert(index == _realizedFallbackRanges.size());
    krys_debug_assert(FontCache::Singleton().Generation() == _generation);

    _realizedFallbackRanges.push_back(FontRanges());
    auto &fontRanges = _realizedFallbackRanges.back();

    if (!index)
    {
      fontRanges = RealizeNextFallback(description, _lastRealizedFallbackIndex, fontSelector);
      if (fontRanges.IsNull() && fontSelector)
      {
        fontRanges =
          fontSelector->FontRangesForFamily(description, css::WebKitFontFamilyNames::StandardFamily());
      }
      if (fontRanges.IsNull())
      {
        fontRanges = FontRanges(FontCache::Singleton().LastResortFallbackFont(description));
      }
      return fontRanges;
    }

    if (_lastRealizedFallbackIndex < description.EffectiveFamilyCount())
    {
      fontRanges = RealizeNextFallback(description, _lastRealizedFallbackIndex, fontSelector);
    }

    if (fontRanges.IsNull() && fontSelector)
    {
      krys_debug_assert(_lastRealizedFallbackIndex >= description.EffectiveFamilyCount());
      uint32 fontSelectorFallbackIndex = _lastRealizedFallbackIndex - description.EffectiveFamilyCount();
      if (fontSelectorFallbackIndex == fontSelector->FallbackFontCount())
      {
        return fontRanges;
      }
      ++_lastRealizedFallbackIndex;
      fontRanges = FontRanges(fontSelector->FallbackFontAt(description, fontSelectorFallbackIndex));
    }

    return fontRanges;
  }

  KRYS_NODISCARD constexpr static bool IsInRange(char32 character, char32 lowerBound,
                                                 char32 upperBound) noexcept
  {
    return character >= lowerBound && character <= upperBound;
  }

  KRYS_NODISCARD static bool ShouldIgnoreRotation(char32 character) noexcept
  {
    if (character == 0x000A7 || character == 0x000A9 || character == 0x000AE)
    {
      return true;
    }

    if (character == 0x000B6 || character == 0x000BC || character == 0x000BD || character == 0x000BE)
    {
      return true;
    }

    if (IsInRange(character, 0x002E5, 0x002EB))
    {
      return true;
    }

    if (IsInRange(character, 0x01100, 0x011FF) || IsInRange(character, 0x01401, 0x0167F)
        || IsInRange(character, 0x018B0, 0x018FF))
    {
      return true;
    }

    if (character == 0x02016 || character == 0x02020 || character == 0x02021 || character == 0x2030
        || character == 0x02031)
    {
      return true;
    }

    if (IsInRange(character, 0x0203B, 0x0203D) || character == 0x02042 || character == 0x02044
        || character == 0x02047 || character == 0x02048 || character == 0x02049 || character == 0x2051)
    {
      return true;
    }

    if (IsInRange(character, 0x02065, 0x02069) || IsInRange(character, 0x020DD, 0x020E0)
        || IsInRange(character, 0x020E2, 0x020E4) || IsInRange(character, 0x02100, 0x02117)
        || IsInRange(character, 0x02119, 0x02131) || IsInRange(character, 0x02133, 0x0213F))
    {
      return true;
    }

    if (IsInRange(character, 0x02145, 0x0214A) || character == 0x0214C || character == 0x0214D
        || IsInRange(character, 0x0214F, 0x0218F))
    {
      return true;
    }

    if (IsInRange(character, 0x02300, 0x02307) || IsInRange(character, 0x0230C, 0x0231F)
        || IsInRange(character, 0x02322, 0x0232B) || IsInRange(character, 0x0237D, 0x0239A)
        || IsInRange(character, 0x023B4, 0x023B6) || IsInRange(character, 0x023BA, 0x023CF)
        || IsInRange(character, 0x023D1, 0x023DB) || IsInRange(character, 0x023E2, 0x024FF))
    {
      return true;
    }

    if (IsInRange(character, 0x025A0, 0x02619) || IsInRange(character, 0x02620, 0x02767)
        || IsInRange(character, 0x02776, 0x02793) || IsInRange(character, 0x02B12, 0x02B2F)
        || IsInRange(character, 0x02B4D, 0x02BFF) || IsInRange(character, 0x02E80, 0x03007))
    {
      return true;
    }

    if (character == 0x03012 || character == 0x03013 || IsInRange(character, 0x03020, 0x0302F)
        || IsInRange(character, 0x03031, 0x0309F) || IsInRange(character, 0x030A1, 0x030FB)
        || IsInRange(character, 0x030FD, 0x0A4CF))
    {
      return true;
    }

    if (IsInRange(character, 0x0A960, 0x0A97F) || IsInRange(character, 0x0AC00, 0x0D7FF)
        || IsInRange(character, 0x0E000, 0x0FAFF))
    {
      return true;
    }

    if (IsInRange(character, 0x0FE10, 0x0FE1F) || IsInRange(character, 0x0FE30, 0x0FE48)
        || IsInRange(character, 0x0FE50, 0x0FE57) || IsInRange(character, 0x0FE5F, 0x0FE62)
        || IsInRange(character, 0x0FE67, 0x0FE6F))
    {
      return true;
    }

    if (IsInRange(character, 0x0FF01, 0x0FF07) || IsInRange(character, 0x0FF0A, 0x0FF0C)
        || IsInRange(character, 0x0FF0E, 0x0FF19) || character == 0x0FF1B
        || IsInRange(character, 0x0FF1F, 0x0FF3A))
    {
      return true;
    }

    if (character == 0x0FF3C || character == 0x0FF3E)
    {
      return true;
    }

    if (IsInRange(character, 0x0FF40, 0x0FF5A) || IsInRange(character, 0x0FFE0, 0x0FFE2)
        || IsInRange(character, 0x0FFE4, 0x0FFE7) || IsInRange(character, 0x0FFF0, 0x0FFF8)
        || character == 0x0FFFD)
    {
      return true;
    }

    if (IsInRange(character, 0x13000, 0x1342F) || IsInRange(character, 0x1B000, 0x1B0FF)
        || IsInRange(character, 0x1D000, 0x1D1FF) || IsInRange(character, 0x1D300, 0x1D37F)
        || IsInRange(character, 0x1F000, 0x1F64F) || IsInRange(character, 0x1F680, 0x1F77F))
    {
      return true;
    }

    if (IsInRange(character, 0x20000, 0x2FFFD) || IsInRange(character, 0x30000, 0x3FFFD))
    {
      return true;
    }

    return false;
  }

  KRYS_NODISCARD static GlyphData
    GlyphDataForNonCJKCharacterWithGlyphOrientation(char32 character, NonCJKGlyphOrientation orientation,
                                                    const GlyphData &data) noexcept
  {
    bool syntheticOblique = data.Font->PlatformData().SyntheticOblique();
    if (orientation == NonCJKGlyphOrientation::Upright || ShouldIgnoreRotation(character))
    {
      GlyphData uprightData = data.Font->UprightOrientationFont().GlyphDataForCharacter(character);
      // If the glyphs are the same, then we know we can just use the horizontal glyph rotated vertically
      // to be upright. For synthetic oblique, however, we will always return the uprightData to ensure
      // that non-CJK and CJK runs are broken up. This guarantees that vertical
      // fonts without isTextOrientationFallback() set contain CJK characters only and thus we can get
      // the oblique slant correct.
      if (data.Glyph == uprightData.Glyph && !syntheticOblique)
      {
        return data;
      }
      // The glyphs are distinct, meaning that the font has a vertical-right glyph baked into it. We can't use
      // that glyph, so we fall back to the upright data and use the horizontal glyph.
      if (uprightData.Font)
      {
        return uprightData;
      }
    }
    else if (orientation == NonCJKGlyphOrientation::Mixed)
    {
      GlyphData verticalRightData =
        data.Font->VerticalRightOrientationFont().GlyphDataForCharacter(character);

      // If there is a baked-in rotated glyph, we will use it unless syntheticOblique is set. If
      // synthetic oblique is set, we fall back to the horizontal glyph. This guarantees that vertical
      // fonts without isTextOrientationFallback() set contain CJK characters only and thus we can get
      // the oblique slant correct.
      if (data.Glyph != verticalRightData.Glyph && !syntheticOblique)
      {
        return data;
      }

      // The glyphs are identical, meaning that we should just use the horizontal glyph.
      if (verticalRightData.Font)
      {
        return verticalRightData;
      }
    }
    return data;
  }

  KRYS_NODISCARD static RefPtr<const Font> FindBestFallbackFont(FontCascadeFonts &fontCascadeFonts,
                                                                const FontCascadeDescription &description,
                                                                FontSelector *fontSelector,
                                                                char32 character) noexcept
  {
    for (uint32 fallbackIndex = 0;; ++fallbackIndex)
    {
      auto &fontRanges = fontCascadeFonts.RealizeFallbackRangesAt(description, fontSelector, fallbackIndex);
      if (fontRanges.IsNull())
      {
        break;
      }
      const Font *currentFont =
        fontRanges.GlyphDataForCharacter(character, ExternalResourceDownloadPolicy::Forbid).Font.get();
      if (!currentFont)
      {
        currentFont = &fontRanges.FontForFirstRange();
      }

      if (!currentFont->IsInterstitial())
      {
        return ShareRefPtr(currentFont);
      }
    }

    return nullptr;
  }

  GlyphData FontCascadeFonts::GlyphDataForSystemFallback(char32 character,
                                                         const FontCascadeDescription &description,
                                                         FontSelector *fontSelector, FontVariant variant,
                                                         ResolvedEmojiPolicy resolvedEmojiPolicy,
                                                         bool systemFallbackShouldBeInvisible) noexcept
  {
    RefPtr<const Font> font = FindBestFallbackFont(*this, description, fontSelector, character);

    if (!font)
    {
      font = ShareRefPtr(&RealizeFallbackRangesAt(description, fontSelector, 0).FontForFirstRange());
    }

    char8 cluster[U8_MAX_LENGTH];
    int32_t length = 0;
    U8_APPEND_UNSAFE(cluster, length, character);
    auto systemFallbackFont = font->SystemFallbackFontForCharacterCluster(
      utf8_stringview {cluster, static_cast<size_t>(length)}, description, resolvedEmojiPolicy,
      _isForPlatformFont ? IsForPlatformFont::Yes : IsForPlatformFont::No);
    if (!systemFallbackFont)
    {
      return GlyphData();
    }

    if (systemFallbackShouldBeInvisible)
    {
      systemFallbackFont = ShareRefPtr(const_cast<Font *>(&systemFallbackFont->InvisibleFont()));
    }

    if (systemFallbackFont->PlatformData().Orientation() == FontOrientation::Vertical
        && !systemFallbackFont->HasVerticalGlyphs() && FontCascade::IsCJKIdeographOrSymbol(character))
    {
      variant = BrokenIdeographVariant;
    }

    GlyphData fallbackGlyphData;
    if (variant == NormalVariant)
    {
      fallbackGlyphData = systemFallbackFont->GlyphDataForCharacter(character);
    }
    else
    {
      fallbackGlyphData =
        systemFallbackFont->VariantFont(description, variant)->GlyphDataForCharacter(character);
    }

    if (fallbackGlyphData.Font
        && fallbackGlyphData.Font->PlatformData().Orientation() == FontOrientation::Vertical
        && !fallbackGlyphData.Font->IsTextOrientationFallback())
    {
      if (variant == NormalVariant && !FontCascade::IsCJKIdeographOrSymbol(character))
      {
        fallbackGlyphData = GlyphDataForNonCJKCharacterWithGlyphOrientation(
          character, description.GetNonCJKGlyphOrientation(), fallbackGlyphData);
      }
    }

    // Keep the system fallback fonts we use alive.
    if (fallbackGlyphData.IsValid())
    {
      _systemFallbackFontSet.insert(krys::move(systemFallbackFont));
    }

    return fallbackGlyphData;
  }

  enum class FallbackVisibility
  {
    Immaterial,
    Visible,
    Invisible
  };

  static void OpportunisticallyStartFontDataURLLoading(const FontCascadeDescription &description,
                                                       FontSelector *fontSelector) noexcept
  {
    // It is a somewhat common practice for a font foundry to break up a single font into two fonts, each
    // having a random half of the alphabet, and then encoding the two fonts as data: urls (with different
    // font-family names). Therefore, if these two fonts don't get loaded at (nearly) the same time, there
    // will be a flash of unintelligible text where only a random half of the letters are visible. This code
    // attempts to pre-warm these data urls to make them load at closer to the same time. However, font
    // loading is asynchronous, and this code doesn't actually fix the race - it just makes it more likely for
    // the two fonts to tie in the race.
    if (!fontSelector)
    {
      return;
    }
    for (uint32 i = 0; i < description.FamilyCount(); ++i)
    {
      fontSelector->OpportunisticallyStartFontDataURLLoading(description, description.FamilyAt(i));
    }
  }

  GlyphData FontCascadeFonts::GlyphDataForVariant(char32 character, const FontCascadeDescription &description,
                                                  FontSelector *fontSelector, FontVariant variant,
                                                  ResolvedEmojiPolicy resolvedEmojiPolicy,
                                                  uint32 fallbackIndex) noexcept
  {
    FallbackVisibility fallbackVisibility = FallbackVisibility::Immaterial;
    ExternalResourceDownloadPolicy policy = ExternalResourceDownloadPolicy::Allow;
    GlyphData loadingResult;
    OpportunisticallyStartFontDataURLLoading(description, fontSelector);
    for (;; ++fallbackIndex)
    {
      auto &fontRanges = RealizeFallbackRangesAt(description, fontSelector, fallbackIndex);
      if (fontRanges.IsNull())
      {
        break;
      }

      GlyphData data = fontRanges.GlyphDataForCharacter(character, policy);
      if (!data.IsValid())
      {
        continue;
      }

      if (data.Font->IsInterstitial())
      {
        policy = ExternalResourceDownloadPolicy::Forbid;
        if (fallbackVisibility == FallbackVisibility::Immaterial)
        {
          fallbackVisibility = data.Font->GetVisibility() == FontVisibility::Visible
                                 ? FallbackVisibility::Visible
                                 : FallbackVisibility::Invisible;
        }
        if (!loadingResult.IsValid() && data.Glyph)
        {
          loadingResult = data;
        }
        continue;
      }

      if (fallbackVisibility == FallbackVisibility::Invisible
          && data.Font->GetVisibility() == FontVisibility::Visible)
      {
        data.Font = WeakPtr<const Font>::WithRef(&data.Font->InvisibleFont());
      }

      if (variant == NormalVariant)
      {
        if (data.Font->PlatformData().Orientation() == FontOrientation::Vertical
            && !data.Font->IsTextOrientationFallback())
        {
          if (!FontCascade::IsCJKIdeographOrSymbol(character))
          {
            return GlyphDataForNonCJKCharacterWithGlyphOrientation(
              character, description.GetNonCJKGlyphOrientation(), data);
          }

          if (!data.Font->HasVerticalGlyphs())
          {
            // Use the broken ideograph font data. The broken ideograph font will use the horizontal width of
            // glyphs to make sure you get a square (even for broken glyphs like symbols used for
            // punctuation).
            return GlyphDataForVariant(character, description, fontSelector, BrokenIdeographVariant,
                                       resolvedEmojiPolicy, fallbackIndex);
          }
        }
      }
      else
      {
        // The variantFont function should not normally return 0.
        // But if it does, we will just render the capital letter big.
        if (const Font *variantFont = data.Font->VariantFont(description, variant))
        {
          return variantFont->GlyphDataForCharacter(character);
        }
      }

      return data;
    }

    if (loadingResult.IsValid())
    {
      return loadingResult;
    }

    // https://drafts.csswg.org/css-fonts-4/#char-handling-issues
    // "If a given character is a Private-Use Area Unicode codepoint, user agents must only match font
    // families named in the font-family list that are not generic families. If none of the families named in
    // the font-family list contain a glyph for that codepoint, user agents must display some form of missing
    // glyph symbol for that character rather than attempting installed font fallback for that codepoint."
    if (IsPrivateUseAreaCharacter(character))
    {
      return {0, &PrimaryFont(description, fontSelector)}; // 0 is the font's reserved .notdef glyph
    }

    return GlyphDataForSystemFallback(character, description, fontSelector, variant, resolvedEmojiPolicy,
                                      fallbackVisibility == FallbackVisibility::Invisible);
  }

  KRYS_NODISCARD static RefPtr<GlyphPage> GlyphPageFromFontRanges(uint32 pageNumber,
                                                                  const FontRanges &fontRanges) noexcept
  {
    const Font *font = nullptr;
    char32 pageRangeFrom = pageNumber * GlyphPage::Size;
    char32 pageRangeTo = pageRangeFrom + GlyphPage::Size - 1;
    auto policy = ExternalResourceDownloadPolicy::Allow;
    FallbackVisibility desiredVisibility = FallbackVisibility::Immaterial;
    for (uint32 i = 0; i < fontRanges.Size(); ++i)
    {
      auto &range = fontRanges.RangeAt(i);
      if (range.From() <= pageRangeFrom && pageRangeTo <= range.To())
      {
        font = range.GetFont(policy);
        if (!font)
        {
          continue;
        }
        if (font->IsInterstitial())
        {
          if (desiredVisibility == FallbackVisibility::Immaterial)
          {
            auto fontVisibility = font->GetVisibility();
            if (fontVisibility == FontVisibility::Visible)
            {
              desiredVisibility = FallbackVisibility::Visible;
            }
            else
            {
              krys_debug_assert(fontVisibility == FontVisibility::Invisible);
              desiredVisibility = FallbackVisibility::Invisible;
            }
          }
          font = nullptr;
          policy = ExternalResourceDownloadPolicy::Forbid;
          continue;
        }
      }
      break;
    }
    if (!font || font->PlatformData().Orientation() == FontOrientation::Vertical)
    {
      return nullptr;
    }

    if (desiredVisibility == FallbackVisibility::Invisible
        && font->GetVisibility() == FontVisibility::Visible)
    {
      return ShareRefPtr(const_cast<GlyphPage *>(font->InvisibleFont().GetGlyphPage(pageNumber)));
    }
    return ShareRefPtr(const_cast<GlyphPage *>(font->GetGlyphPage(pageNumber)));
  }

  GlyphData FontCascadeFonts::GlyphDataForCharacter(char32 c, const FontCascadeDescription &description,
                                                    FontSelector *fontSelector, FontVariant variant,
                                                    ResolvedEmojiPolicy resolvedEmojiPolicy) noexcept
  {
    krys_debug_assert(variant != AutoVariant);

    if (variant != NormalVariant)
    {
      return GlyphDataForVariant(c, description, fontSelector, variant, resolvedEmojiPolicy);
    }

    const uint32 pageNumber = GlyphPage::PageNumberForCodePoint(c);

    auto &pages = _cachedPages[ToUnderlying(resolvedEmojiPolicy)];
    auto it = pages.find(pageNumber);
    if (it == pages.end())
    {
      // Initialize cache with a full page of glyph mappings from a single font.
      it = pages
             .emplace(pageNumber, GlyphPageCacheEntry {GlyphPageFromFontRanges(
                                    pageNumber, RealizeFallbackRangesAt(description, fontSelector, 0))})
             .first;
    }
    auto &cacheEntry = it->second;

    GlyphData glyphData = cacheEntry.GlyphDataForCharacter(c);
    if (!glyphData.IsValid())
    {
      // No glyph, resolve per-character.
      krys_debug_assert(variant == NormalVariant);
      glyphData = GlyphDataForVariant(c, description, fontSelector, variant, resolvedEmojiPolicy);
      // Cache the results.
      cacheEntry.SetGlyphDataForCharacter(c, glyphData);
    }

    return glyphData;
  }

  void FontCascadeFonts::PruneSystemFallbacks() noexcept
  {
    if (_systemFallbackFontSet.empty())
    {
      return;
    }
    // Mutable glyph pages may reference fallback fonts.
    for (auto &cachedPages : _cachedPages)
    {
      std::erase_if(cachedPages, [](auto &keyAndValue) { return keyAndValue.second.IsMixedFont(); });
    }
    _systemFallbackFontSet.clear();
  }
}
