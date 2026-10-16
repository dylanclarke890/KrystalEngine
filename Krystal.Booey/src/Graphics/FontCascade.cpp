#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Booey/Graphics/WidthCache.hpp"
#include "Krystal.Booey/Graphics/WidthIterator.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <ranges>
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    uint32 LastFontCascadeGeneration = 0;

    /// @brief The last code point of @p text (UTF-8), which must not be empty.
    KRYS_NODISCARD char32 LastCodePoint(utf8_stringview text) noexcept
    {
      krys_debug_assert(!text.empty());
      auto start = text.length() - 1;
      while (start && text.length() - start < 4 && (static_cast<char8>(text[start]) & 0xC0) == 0x80)
      {
        --start;
      }
      auto decoded = text::DecodeUTF8(Span<const char8> {text}.subspan(start));
      if (start + decoded.Length != text.length())
      {
        return text::UnicodeReplacementChar;
      }
      return decoded.CodePoint;
    }
  }

  FontCascade::CodePath FontCascade::s_codePath = CodePath::Auto;

  FontCascade::FontCascade() noexcept = default;

  FontCascade::FontCascade(FontCascadeDescription &&description) noexcept
      : _fontDescription(krys::move(description)), _generation(++LastFontCascadeGeneration),
        _useBackslashAsYenSymbol(ComputeUseBackslashAsYenSymbol()), _enableKerning(ComputeEnableKerning()),
        _requiresShaping(ComputeRequiresShaping())
  {
    _fontDescription.SetShouldDisableLigaturesForSpacing(false);
  }

  FontCascade::FontCascade(FontCascadeDescription &&description, const FontCascade &other) noexcept
      : _fontDescription(krys::move(description)), _spacing(other._spacing),
        _generation(++LastFontCascadeGeneration), _useBackslashAsYenSymbol(ComputeUseBackslashAsYenSymbol()),
        _enableKerning(ComputeEnableKerning()), _requiresShaping(ComputeRequiresShaping())
  {
  }

  FontCascade::FontCascade(const FontPlatformData &platformData, FontSmoothingMode fontSmoothingMode) noexcept
      : _fonts(FontCascadeFonts::CreateForPlatformFont(platformData)), _enableKerning(ComputeEnableKerning()),
        _requiresShaping(ComputeRequiresShaping())
  {
    _fontDescription.SetFontSmoothing(fontSmoothingMode);
    _fontDescription.SetSpecifiedSize(platformData.Size());
    _fontDescription.SetComputedSize(platformData.Size());
    if (auto *face = platformData.Face())
    {
      auto &capabilities = face->Capabilities();
      _fontDescription.SetIsItalic(capabilities.Slope.Maximum != NormalItalicValue());
      _fontDescription.SetWeight(IsFontWeightBold(capabilities.Weight.Maximum) ? BoldWeightValue()
                                                                               : NormalWeightValue());
    }
  }

  FontCascade::FontCascade(const FontCascade &other) noexcept
      : _fontDescription(other._fontDescription), _spacing(other._spacing), _fonts(other._fonts),
        _fontSelector(other._fontSelector), _generation(other._generation),
        _useBackslashAsYenSymbol(other._useBackslashAsYenSymbol), _enableKerning(ComputeEnableKerning()),
        _requiresShaping(ComputeRequiresShaping())
  {
  }

  FontCascade &FontCascade::operator=(const FontCascade &other) noexcept
  {
    _fontDescription = other._fontDescription;
    _fonts = other._fonts;
    _spacing = other._spacing;
    _generation = other._generation;
    _useBackslashAsYenSymbol = other._useBackslashAsYenSymbol;
    _enableKerning = other._enableKerning;
    _requiresShaping = other._requiresShaping;
    _fontSelector = other._fontSelector;
    _canUseSimplifiedTextMeasuringForAutoVariantCache.reset();
    return *this;
  }

  FontCascade::~FontCascade() noexcept = default;

  bool FontCascade::operator==(const FontCascade &other) const noexcept
  {
    if (_fontDescription != other._fontDescription || _spacing != other._spacing)
    {
      return false;
    }
    if (_fonts != other._fonts)
    {
      return false;
    }
    if (!_fonts || !other._fonts)
    {
      return false;
    }
    if (GetFontSelector() != other.GetFontSelector())
    {
      return false;
    }
    // Can these cases actually somehow occur? All fonts should get wiped out by full style recalc.
    if (FontSelectorVersion() != other.FontSelectorVersion())
    {
      return false;
    }
    if (_fonts->Generation() != other._fonts->Generation())
    {
      return false;
    }
    return true;
  }

  bool FontCascade::IsCurrent(const FontSelector &fontSelector) const noexcept
  {
    if (!_fonts)
    {
      return false;
    }
    if (_fonts->Generation() != FontCache::Singleton().Generation())
    {
      return false;
    }
    if (FontSelectorVersion() != fontSelector.Version())
    {
      return false;
    }

    return true;
  }

  void FontCascade::UpdateFonts(Ref<FontCascadeFonts> &&fonts) const noexcept
  {
    // FIXME: Ideally we'd only update m_generation if the fonts changed.
    _fonts = krys::move(fonts);
    _generation = ++LastFontCascadeGeneration;
    _canUseSimplifiedTextMeasuringForAutoVariantCache.reset();
  }

  void FontCascade::Update(RefPtr<FontSelector> &&fontSelector) const noexcept
  {
    _fontSelector = krys::move(fontSelector);
    FontCache::Singleton().UpdateFontCascade(*this);
  }

  uint32 FontCascade::FontSelectorVersion() const noexcept
  {
    return _fontSelector ? _fontSelector->Version() : 0;
  }

  FontCascadeFonts &FontCascade::EnsureFonts() const noexcept
  {
    // FIXME: Every cascade should be updated (given its document's font selector) before it's used; some
    // aren't yet, so they get fonts without a font selector.
    if (!_fonts)
    {
      Update(krys::move(_fontSelector));
    }
    return *_fonts;
  }

  Ref<const Font> FontCascade::PrimaryFont() const noexcept
  {
    auto &font = EnsureFonts().PrimaryFont(_fontDescription, _fontSelector.get());
    return ShareRef(font);
  }

  const FontRanges &FontCascade::FallbackRangesAt(uint32 index) const noexcept
  {
    return EnsureFonts().RealizeFallbackRangesAt(_fontDescription, _fontSelector.get(), index);
  }

  bool FontCascade::IsFixedPitch() const noexcept
  {
    return EnsureFonts().IsFixedPitch(_fontDescription, _fontSelector.get());
  }

  float FontCascade::ZeroWidth() const noexcept
  {
    // This represents the advance measure of the glyph 0 (zero, the Unicode character U+0030)
    // in the element's font. In cases where it is impossible or impractical to determine the measure of the 0
    // glyph, it must be assumed to be 0.5em
    auto defaultZeroWidthValue = GetFontDescription().ComputedSize() / 2;
    if (!MetricsOfPrimaryFont().ZeroWidth())
    {
      return defaultZeroWidthValue;
    }

    auto glyphData = GlyphDataForCharacter(U'0', false);
    if (!glyphData.IsValid())
    {
      return defaultZeroWidthValue;
    }

    return glyphData.Font->GetFontMetrics().ZeroWidth().value_or(defaultZeroWidthValue);
  }

  GlyphData FontCascade::GlyphDataForCharacter(char32 c, bool mirror, FontVariant variant,
                                               Maybe<ResolvedEmojiPolicy> resolvedEmojiPolicy) const noexcept
  {
    if (variant == AutoVariant)
    {
      if (_fontDescription.VariantCaps() == FontVariantCaps::Small)
      {
        auto upperC = static_cast<char32>(u_toupper(static_cast<UChar32>(c)));
        if (upperC != c)
        {
          c = upperC;
          variant = SmallCapsVariant;
        }
        else
        {
          variant = NormalVariant;
        }
      }
      else
      {
        variant = NormalVariant;
      }
    }

    if (mirror)
    {
      c = static_cast<char32>(u_charMirror(static_cast<UChar32>(c)));
    }

    auto emojiPolicy = resolvedEmojiPolicy.value_or(ResolveEmojiPolicy(_fontDescription.VariantEmoji(), c));

    return EnsureFonts().GlyphDataForCharacter(c, _fontDescription, _fontSelector.get(), variant,
                                               emojiPolicy);
  }

  RefPtr<const Font> FontCascade::FontForCombiningCharacterSequence(utf8_stringview text) const noexcept
  {
    constexpr char32 TextVariationSelector = 0xFE0E;
    constexpr char32 EmojiVariationSelector = 0xFE0F;
    constexpr char32 EmojiCat = 0x1F408;

    krys_debug_assert(!text.empty());
    text::CodePointIterator codePointsIterator {text};
    char32 baseCharacter = *codePointsIterator;
    ++codePointsIterator;
    bool isOnlySingleCodePoint = codePointsIterator.AtEnd();

    auto [emojiPolicy, shouldForceEmojiFont] = [&]() -> std::pair<ResolvedEmojiPolicy, bool>
    {
      if (!isOnlySingleCodePoint)
      {
        if (*codePointsIterator == EmojiVariationSelector)
        {
          return {ResolvedEmojiPolicy::RequireEmoji, true};
        }

        if (*codePointsIterator == TextVariationSelector)
        {
          return {ResolvedEmojiPolicy::RequireText, false};
        }
      }

      auto emojiPolicy = ResolveEmojiPolicy(_fontDescription.VariantEmoji(), baseCharacter);
      return {emojiPolicy, emojiPolicy == ResolvedEmojiPolicy::RequireEmoji
                             && _fontDescription.VariantEmoji() == FontVariantEmoji::Emoji};
    }();

    char32 baseCharacterForBaseFont = baseCharacter;
    if (shouldForceEmojiFont)
    {
      // System fallback doesn't support character sequences, so here we override
      // the base character with the cat emoji to try to force an emoji font.
      baseCharacterForBaseFont = EmojiCat;
    }
    GlyphData baseCharacterGlyphData =
      GlyphDataForCharacter(baseCharacterForBaseFont, false, NormalVariant, emojiPolicy);
    if (!baseCharacterGlyphData.Glyph)
    {
      return nullptr;
    }

    auto fontMatchesEmojiPolicy = [](const Font *font, ResolvedEmojiPolicy emojiPolicy) -> bool
    {
      if (!font)
      {
        return false;
      }

      switch (emojiPolicy)
      {
        case ResolvedEmojiPolicy::RequireEmoji: return font->PlatformData().IsColorBitmapFont();
        case ResolvedEmojiPolicy::RequireText:  return !font->PlatformData().IsColorBitmapFont();
        case ResolvedEmojiPolicy::NoPreference: break;
      }
      return true;
    };

    const Font *baseFont = baseCharacterGlyphData.Font.get();
    if (isOnlySingleCodePoint && !shouldForceEmojiFont && fontMatchesEmojiPolicy(baseFont, emojiPolicy))
    {
      return ShareRefPtr(baseFont);
    }

    bool triedBaseCharacterFont = false;
    for (uint32 i = 0; !FallbackRangesAt(i).IsNull(); ++i)
    {
      auto &fontRanges = FallbackRangesAt(i);
      if (fontRanges.IsGenericFontFamily() && IsPrivateUseAreaCharacter(baseCharacter))
      {
        continue;
      }

      const Font *font = fontRanges.FontForCharacter(baseCharacter);
      if (!font)
      {
        continue;
      }

      if (!fontMatchesEmojiPolicy(font, emojiPolicy))
      {
        continue;
      }

      if (font == baseFont)
      {
        triedBaseCharacterFont = true;
      }

      if (font->CanRenderCombiningCharacterSequence(text))
      {
        return ShareRefPtr(font);
      }
    }

    if (!triedBaseCharacterFont && baseFont && baseFont->CanRenderCombiningCharacterSequence(text))
    {
      return ShareRefPtr(baseFont);
    }

    bool clusterContainsOtherNonDefaultIgnorableCodePoints = [&]() -> bool
    {
      if (isOnlySingleCodePoint)
      {
        return false;
      }

      for (; !codePointsIterator.AtEnd(); ++codePointsIterator)
      {
        if (!IsDefaultIgnorableCodePoint(*codePointsIterator))
        {
          return true;
        }
      }

      return false;
    }();

    // Try a system fallback for the whole cluster if needed.
    if (clusterContainsOtherNonDefaultIgnorableCodePoints)
    {
      auto preferColoredFont = emojiPolicy == ResolvedEmojiPolicy::RequireEmoji
                                 ? FontCache::PreferColoredFont::Yes
                                 : FontCache::PreferColoredFont::No;
      if (auto systemFallback = FontCache::Singleton().SystemFallbackForCharacterCluster(
            _fontDescription, FallbackRangesAt(0).FontForFirstRange(), IsForPlatformFont::No,
            preferColoredFont, text))
      {
        if (systemFallback->CanRenderCombiningCharacterSequence(text))
        {
          return systemFallback;
        }
      }
    }

    return nullptr;
  }

  bool FontCascade::IsLoadingCustomFonts() const noexcept
  {
    return _fonts && _fonts->IsLoadingCustomFonts();
  }

  bool FontCascade::ComputeUseBackslashAsYenSymbol() const noexcept
  {
    return FontCache::Singleton().UseBackslashAsYenSignForFamily(_fontDescription.FirstFamily());
  }

  ResolvedEmojiPolicy FontCascade::ResolveEmojiPolicy(FontVariantEmoji fontVariantEmoji, char32) noexcept
  {
    // FIXME: https://bugs.webkit.org/show_bug.cgi?id=259205 We can't return RequireText or RequireEmoji
    // unless we have a way of knowing whether a font/glyph is color or not.
    switch (fontVariantEmoji)
    {
      case FontVariantEmoji::Normal:
      case FontVariantEmoji::Unicode: return ResolvedEmojiPolicy::NoPreference;
      case FontVariantEmoji::Text:    return ResolvedEmojiPolicy::RequireText;
      case FontVariantEmoji::Emoji:   return ResolvedEmojiPolicy::RequireEmoji;
    }
    return ResolvedEmojiPolicy::NoPreference;
  }

  bool FontCascade::IsCJKIdeograph(char32 c) noexcept
  {
    // The basic CJK Unified Ideographs block.
    if (c >= 0x4E00 && c <= 0x9FFF)
    {
      return true;
    }

    // CJK Unified Ideographs Extension A.
    if (c >= 0x3400 && c <= 0x4DBF)
    {
      return true;
    }

    // CJK Radicals Supplement.
    if (c >= 0x2E80 && c <= 0x2EFF)
    {
      return true;
    }

    // Kangxi Radicals.
    if (c >= 0x2F00 && c <= 0x2FDF)
    {
      return true;
    }

    // CJK Strokes.
    if (c >= 0x31C0 && c <= 0x31EF)
    {
      return true;
    }

    // CJK Compatibility Ideographs.
    if (c >= 0xF900 && c <= 0xFAFF)
    {
      return true;
    }

    // CJK Unified Ideographs Extension B.
    if (c >= 0x20000 && c <= 0x2A6DF)
    {
      return true;
    }

    // CJK Unified Ideographs Extension C.
    if (c >= 0x2A700 && c <= 0x2B73F)
    {
      return true;
    }

    // CJK Unified Ideographs Extension D.
    if (c >= 0x2B740 && c <= 0x2B81F)
    {
      return true;
    }

    // CJK Compatibility Ideographs Supplement.
    if (c >= 0x2F800 && c <= 0x2FA1F)
    {
      return true;
    }

    return false;
  }

  bool FontCascade::IsCJKIdeographOrSymbol(char32 c) noexcept
  {
    // 0x2C7 Caron, Mandarin Chinese 3rd Tone
    // 0x2CA Modifier Letter Acute Accent, Mandarin Chinese 2nd Tone
    // 0x2CB Modifier Letter Grave Access, Mandarin Chinese 4th Tone
    // 0x2D9 Dot Above, Mandarin Chinese 5th Tone
    // 0x2EA Modifier Letter Yin Departing Tone Mark
    // 0x2EB Modifier Letter Yang Departing Tone Mark
    if ((c == 0x2C7) || (c == 0x2CA) || (c == 0x2CB) || (c == 0x2D9) || (c == 0x2EA) || (c == 0x2EB))
    {
      return true;
    }

    if ((c == 0x2020) || (c == 0x2021) || (c == 0x2030) || (c == 0x203B) || (c == 0x203C) || (c == 0x2042)
        || (c == 0x2047) || (c == 0x2048) || (c == 0x2049) || (c == 0x2051) || (c == 0x20DD) || (c == 0x20DE)
        || (c == 0x2100) || (c == 0x2103) || (c == 0x2105) || (c == 0x2109) || (c == 0x210A) || (c == 0x2113)
        || (c == 0x2116) || (c == 0x2121) || (c == 0x212B) || (c == 0x213B) || (c == 0x2150) || (c == 0x2151)
        || (c == 0x2152))
    {
      return true;
    }

    if (c >= 0x2156 && c <= 0x215A)
    {
      return true;
    }

    if (c >= 0x2160 && c <= 0x216B)
    {
      return true;
    }

    if (c >= 0x2170 && c <= 0x217B)
    {
      return true;
    }

    if ((c == 0x217F) || (c == 0x2189) || (c == 0x2307) || (c == 0x2312) || (c == 0x23BE) || (c == 0x23BF))
    {
      return true;
    }

    if (c >= 0x23C0 && c <= 0x23CC)
    {
      return true;
    }

    if ((c == 0x23CE) || (c == 0x2423))
    {
      return true;
    }

    if (c >= 0x2460 && c <= 0x2492)
    {
      return true;
    }

    if (c >= 0x249C && c <= 0x24FF)
    {
      return true;
    }

    if ((c == 0x25A0) || (c == 0x25A1) || (c == 0x25A2) || (c == 0x25AA) || (c == 0x25AB))
    {
      return true;
    }

    if ((c == 0x25B1) || (c == 0x25B2) || (c == 0x25B3) || (c == 0x25B6) || (c == 0x25B7) || (c == 0x25BC)
        || (c == 0x25BD))
    {
      return true;
    }

    if ((c == 0x25C0) || (c == 0x25C1) || (c == 0x25C6) || (c == 0x25C7) || (c == 0x25C9) || (c == 0x25CB)
        || (c == 0x25CC))
    {
      return true;
    }

    if (c >= 0x25CE && c <= 0x25D3)
    {
      return true;
    }

    if (c >= 0x25E2 && c <= 0x25E6)
    {
      return true;
    }

    if (c == 0x25EF)
    {
      return true;
    }

    if (c >= 0x2600 && c <= 0x2603)
    {
      return true;
    }

    if ((c == 0x2605) || (c == 0x2606) || (c == 0x260E) || (c == 0x2616) || (c == 0x2617) || (c == 0x2640)
        || (c == 0x2642))
    {
      return true;
    }

    if (c >= 0x2660 && c <= 0x266F)
    {
      return true;
    }

    if (c >= 0x2672 && c <= 0x267D)
    {
      return true;
    }

    if ((c == 0x26A0) || (c == 0x26BD) || (c == 0x26BE) || (c == 0x2713) || (c == 0x271A) || (c == 0x273F)
        || (c == 0x2740) || (c == 0x2756))
    {
      return true;
    }

    if (c >= 0x2776 && c <= 0x277F)
    {
      return true;
    }

    if (c == 0x2B1A)
    {
      return true;
    }

    // Ideographic Description Characters.
    if (c >= 0x2FF0 && c <= 0x2FFF)
    {
      return true;
    }

    // CJK Symbols and Punctuation, excluding 0x3030.
    if (c >= 0x3000 && c < 0x3030)
    {
      return true;
    }

    if (c > 0x3030 && c <= 0x303F)
    {
      return true;
    }

    // Hiragana
    if (c >= 0x3040 && c <= 0x309F)
    {
      return true;
    }

    // Katakana
    if (c >= 0x30A0 && c <= 0x30FF)
    {
      return true;
    }

    // Bopomofo
    if (c >= 0x3100 && c <= 0x312F)
    {
      return true;
    }

    if (c >= 0x3190 && c <= 0x319F)
    {
      return true;
    }

    // Bopomofo Extended
    if (c >= 0x31A0 && c <= 0x31BF)
    {
      return true;
    }

    // Enclosed CJK Letters and Months.
    if (c >= 0x3200 && c <= 0x32FF)
    {
      return true;
    }

    // CJK Compatibility.
    if (c >= 0x3300 && c <= 0x33FF)
    {
      return true;
    }

    if (c >= 0xF860 && c <= 0xF862)
    {
      return true;
    }

    // CJK Compatibility Forms.
    if (c >= 0xFE30 && c <= 0xFE4F)
    {
      return true;
    }

    if ((c == 0xFE10) || (c == 0xFE11) || (c == 0xFE12) || (c == 0xFE19))
    {
      return true;
    }

    if ((c == 0xFF0D) || (c == 0xFF1B) || (c == 0xFF1C) || (c == 0xFF1E))
    {
      return false;
    }

    // Halfwidth and Fullwidth Forms
    // Usually only used in CJK
    if (c >= 0xFF00 && c <= 0xFFEF)
    {
      return true;
    }

    // Emoji.
    if (c == 0x1F100)
    {
      return true;
    }

    if (c >= 0x1F110 && c <= 0x1F129)
    {
      return true;
    }

    if (c >= 0x1F130 && c <= 0x1F149)
    {
      return true;
    }

    if (c >= 0x1F150 && c <= 0x1F169)
    {
      return true;
    }

    if (c >= 0x1F170 && c <= 0x1F189)
    {
      return true;
    }

    if (c >= 0x1F200 && c <= 0x1F6C5)
    {
      return true;
    }

    return IsCJKIdeograph(c);
  }

  bool FontCascade::CanTakeFixedPitchFastContentMeasuring() const noexcept
  {
    return EnsureFonts().CanTakeFixedPitchFastContentMeasuring(_fontDescription, _fontSelector.get());
  }

  float FontCascade::TabWidth(const Font &font, const boo::TabSize &tabSize, float position,
                              Font::SyntheticBoldInclusion syntheticBoldInclusion) const noexcept
  {
    float baseTabWidth = tabSize.WidthInPixels(font.SpaceWidth());
    float result = 0;
    if (!baseTabWidth)
    {
      result = LetterSpacing();
    }
    else
    {
      result = baseTabWidth - std::fmod(position, baseTabWidth);
      if (result < font.SpaceWidth() / 2)
      {
        result += baseTabWidth;
      }
    }
    // If our caller passes in SyntheticBoldInclusion::Exclude, that means they're going to apply synthetic
    // bold themselves later. However, regardless of that, the space characters that are fed into the width
    // calculation need to have their correct width, including the synthetic bold. So, we've already got
    // synthetic bold applied, so if we're supposed to exclude it, we need to subtract it out here.
    return result
           - (syntheticBoldInclusion == Font::SyntheticBoldInclusion::Exclude ? font.SyntheticBoldOffset()
                                                                              : 0);
  }

  float FontCascade::WidthOfTextRange(const TextRun &run, uint32 from, uint32 to, float &outWidthBeforeRange,
                                      float &outWidthAfterRange) const noexcept
  {
    krys_debug_assert(from <= to);
    krys_debug_assert(to <= run.Length());

    if (!run.Length())
    {
      return 0;
    }

    float offsetBeforeRange = 0;
    float offsetAfterRange = 0;
    float totalWidth = 0;

    if (ShouldUseComplexTextController(GetCodePath(run)))
    {
      ComplexTextController complexIterator(*this, run);
      complexIterator.Advance(from, nullptr, GlyphIterationStyle::IncludePartialGlyphs, nullptr);
      offsetBeforeRange = complexIterator.RunWidthSoFar();
      complexIterator.Advance(to, nullptr, GlyphIterationStyle::IncludePartialGlyphs, nullptr);
      offsetAfterRange = complexIterator.RunWidthSoFar();
      complexIterator.Advance(run.Length(), nullptr, GlyphIterationStyle::IncludePartialGlyphs, nullptr);
      totalWidth = complexIterator.RunWidthSoFar();
    }
    else
    {
      WidthIterator simpleIterator(*this, run);
      GlyphBuffer glyphBuffer;
      simpleIterator.Advance(from, glyphBuffer);
      offsetBeforeRange = simpleIterator.RunWidthSoFar();
      simpleIterator.Advance(to, glyphBuffer);
      offsetAfterRange = simpleIterator.RunWidthSoFar();
      simpleIterator.Advance(run.Length(), glyphBuffer);
      totalWidth = simpleIterator.RunWidthSoFar();
      simpleIterator.Finalize(glyphBuffer);
      // FIXME: Finalizing the WidthIterator can affect the total width.
      // We might need to adjust the various widths we've measured to account for that.
    }

    outWidthBeforeRange = offsetBeforeRange;
    outWidthAfterRange = totalWidth - offsetAfterRange;
    return offsetAfterRange - offsetBeforeRange;
  }

  float FontCascade::Width(utf8_stringview text) const noexcept
  {
    TextRun run {text};
    return Width(run);
  }

  float FontCascade::Width(const TextRun &run, HashSet<RefPtr<const Font>> *fallbackFonts,
                           GlyphOverflow *glyphOverflow) const noexcept
  {
    if (!run.Length())
    {
      return 0;
    }

    CodePath codePathToUse = GetCodePath(run);
    if (codePathToUse != CodePath::Complex)
    {
      // The complex path is more restrictive about returning fallback fonts than the simple path, so we need
      // an explicit test to make their behaviors match.
      if constexpr (!CanReturnFallbackFontsForComplexText())
      {
        fallbackFonts = nullptr;
      }
      // The simple path can optimize the case where glyph overflow is not observable.
      if (codePathToUse != CodePath::SimpleWithGlyphOverflow
          && (glyphOverflow && !glyphOverflow->ComputeBounds))
      {
        glyphOverflow = nullptr;
      }
    }

    bool hasWordSpacingOrLetterSpacing = WordSpacing() || LetterSpacing();
    float *cacheEntry = EnsureFonts().GetWidthCache().Add(
      run, std::numeric_limits<float>::quiet_NaN(), EnableKerning() || RequiresShaping(),
      hasWordSpacingOrLetterSpacing, !GetTextAutospace().IsNoAutospace(), glyphOverflow);
    if (cacheEntry && !std::isnan(*cacheEntry))
    {
      return *cacheEntry;
    }

    HashSet<RefPtr<const Font>> localFallbackFonts;
    if (!fallbackFonts)
    {
      fallbackFonts = &localFallbackFonts;
    }

    float result = Width(codePathToUse, run, fallbackFonts, glyphOverflow);
    if (cacheEntry && fallbackFonts->empty())
    {
      *cacheEntry = result;
    }
    return result;
  }

  float FontCascade::Width(CodePath codePathToUse, const TextRun &run,
                           HashSet<RefPtr<const Font>> *fallbackFonts,
                           GlyphOverflow *glyphOverflow) const noexcept
  {
    if (ShouldUseComplexTextController(codePathToUse))
    {
      ComplexTextController controller(*this, run, true, fallbackFonts);
      if (glyphOverflow)
      {
        glyphOverflow->Top = LayoutUnit(std::max<double>(
          glyphOverflow->Top, -controller.MinGlyphBoundingBoxY()
                                - (glyphOverflow->ComputeBounds ? 0 : MetricsOfPrimaryFont().Ascent())));
        glyphOverflow->Bottom = LayoutUnit(std::max<double>(
          glyphOverflow->Bottom, controller.MaxGlyphBoundingBoxY()
                                   - (glyphOverflow->ComputeBounds ? 0 : MetricsOfPrimaryFont().Descent())));
        glyphOverflow->Left = LayoutUnit(std::max<double>(0, -controller.MinGlyphBoundingBoxX()));
        glyphOverflow->Right = LayoutUnit(
          std::max<double>(0, controller.MaxGlyphBoundingBoxX() - controller.TotalAdvance().Width()));
      }
      return controller.TotalAdvance().Width();
    }

    WidthIterator it(*this, run, fallbackFonts, glyphOverflow != nullptr);
    GlyphBuffer glyphBuffer;
    it.Advance(run.Length(), glyphBuffer);
    it.Finalize(glyphBuffer);

    if (glyphOverflow)
    {
      glyphOverflow->Top = LayoutUnit(std::max<double>(
        glyphOverflow->Top,
        -it.MinGlyphBoundingBoxY() - (glyphOverflow->ComputeBounds ? 0 : MetricsOfPrimaryFont().Ascent())));
      glyphOverflow->Bottom = LayoutUnit(std::max<double>(
        glyphOverflow->Bottom,
        it.MaxGlyphBoundingBoxY() - (glyphOverflow->ComputeBounds ? 0 : MetricsOfPrimaryFont().Descent())));
      glyphOverflow->Left = LayoutUnit(it.FirstGlyphOverflow());
      glyphOverflow->Right = LayoutUnit(it.LastGlyphOverflow());
    }

    return it.RunWidthSoFar();
  }

  float FontCascade::WidthForTextUsingSimplifiedMeasuring(utf8_stringview text,
                                                          TextDirection textDirection) const noexcept
  {
    if (text.empty())
    {
      return 0;
    }

    krys_debug_assert(GetCodePath(TextRun(text)) != CodePath::Complex);
    float *cacheEntry = EnsureFonts().GetWidthCache().Add(text, std::numeric_limits<float>::quiet_NaN());
    if (cacheEntry && !std::isnan(*cacheEntry))
    {
      return *cacheEntry;
    }

    return WidthForSimpleTextSlow(text, textDirection, cacheEntry);
  }

  float FontCascade::WidthForSimpleTextSlow(utf8_stringview text, TextDirection textDirection,
                                            float *cacheEntry) const noexcept
  {
    TextRun run {text, 0, 0, ExpansionBehavior::DefaultBehavior(), textDirection, false, false};
    float result = Width(CodePath::Simple, run);

    if (cacheEntry)
    {
      *cacheEntry = result;
    }
    return result;
  }

  float FontCascade::WidthForSimpleTextWithFixedPitch(utf8_stringview text,
                                                      bool whitespaceIsCollapsed) const noexcept
  {
    if (text.empty())
    {
      return 0;
    }

    auto monospaceCharacterWidth = PrimaryFont()->SpaceWidth();
    if (whitespaceIsCollapsed)
    {
      uint32 length = 0;
      for (text::CodePointIterator it {text}; !it.AtEnd(); ++it)
      {
        ++length;
      }
      return length * monospaceCharacterWidth;
    }

    float *cacheEntry = EnsureFonts().GetWidthCache().Add(text, std::numeric_limits<float>::quiet_NaN());
    if (cacheEntry && !std::isnan(*cacheEntry))
    {
      return *cacheEntry;
    }

    auto width = 0.f;
    bool isFirst = true;
    for (text::CodePointIterator it {text}; !it.AtEnd(); ++it, isFirst = false)
    {
      auto character = *it;
      // CanUseSimplifiedTextMeasuring will return false for tab character with !whitespaceIsCollapsed.
      krys_debug_assert(character != text::TabCharacter);
      if (character == text::NewlineCharacter || character == text::LineSeparator
          || character == text::ParagraphSeparator)
      {
        // Zero width.
      }
      else if (character >= text::Space)
      {
        width += monospaceCharacterWidth;
      }
      if (!isFirst && character == text::Space)
      {
        width += WordSpacing();
      }
    }

    if (cacheEntry)
    {
      *cacheEntry = width;
    }
    return width;
  }

  float FontCascade::WidthForCharacterInRun(const TextRun &run, uint32 characterPosition) const noexcept
  {
    auto length = text::DecodeUTF8(Span<const char8> {run.Text()}.subspan(characterPosition)).Length;
    auto shortenedRun = run.SubRun(characterPosition, length);
    return Width(GetCodePath(run), shortenedRun);
  }

  bool FontCascade::CanUseSimplifiedTextMeasuring(char32 character, FontVariant fontVariant,
                                                  bool whitespaceIsCollapsed,
                                                  const Font &primaryFont) const noexcept
  {
    if (character == text::TabCharacter && !whitespaceIsCollapsed)
    {
      return false;
    }

    // We cache whitespaceIsCollapsed = true result. false case is handled above.
    whitespaceIsCollapsed = true;

    bool isCacheable = fontVariant == AutoVariant && character <= 0xFF;
    size_t baseIndex =
      static_cast<size_t>(character) * BitsPerCharacterInCanUseSimplifiedTextMeasuringForAutoVariantCache;
    if (isCacheable)
    {
      static_assert(0 < BitsPerCharacterInCanUseSimplifiedTextMeasuringForAutoVariantCache);
      static_assert(1 < BitsPerCharacterInCanUseSimplifiedTextMeasuringForAutoVariantCache);
      if (_canUseSimplifiedTextMeasuringForAutoVariantCache.test(baseIndex))
      {
        return _canUseSimplifiedTextMeasuringForAutoVariantCache.test(baseIndex + 1);
      }
    }

    bool result = WidthIterator::CharacterCanUseSimplifiedTextMeasuring(character, whitespaceIsCollapsed);
    if (result)
    {
      constexpr bool mirror = false;
      auto glyphData = GlyphDataForCharacter(character, mirror, fontVariant);
      result = glyphData.IsValid() && glyphData.Font.get() == &primaryFont;
    }

    if (isCacheable)
    {
      _canUseSimplifiedTextMeasuringForAutoVariantCache.set(baseIndex, true);
      _canUseSimplifiedTextMeasuringForAutoVariantCache.set(baseIndex + 1, result);
    }
    return result;
  }

  // For font families where any of the fonts don't have a valid entry in the OS/2 table for avgCharWidth,
  // fallback to the legacy behavior of getting the avgCharWidth from the width of a '0'. This only seems to
  // apply to a fixed number of Mac fonts, but, in order to get similar rendering across platforms, we do this
  // check for all platforms.
  bool FontCascade::HasValidAverageCharWidth() const noexcept
  {
    const StringAtom &family = FirstFamily();
    if (family.View().empty())
    {
      return false;
    }

    static constexpr Array<utf8_stringview, 34> Names = {
      u8"#GungSeo",
      u8"#HeadLineA",
      u8"#PCMyungjo",
      u8"#PilGi",
      u8"American Typewriter",
      u8"Apple Braille",
      u8"Apple LiGothic",
      u8"Apple LiSung",
      u8"Apple Symbols",
      u8"AppleGothic",
      u8"AppleMyungjo",
      u8"Arial Hebrew",
      u8"Chalkboard",
      u8"Cochin",
      u8"Corsiva Hebrew",
      u8"Courier",
      u8"Euphemia UCAS",
      u8"Geneva",
      u8"Gill Sans",
      u8"Hei",
      u8"Helvetica",
      u8"Hoefler Text",
      u8"InaiMathi",
      u8"Kai",
      u8"Lucida Grande",
      u8"Marker Felt",
      u8"Monaco",
      u8"Mshtakan",
      u8"New Peninim MT",
      u8"Osaka",
      u8"Raanana",
      u8"STHeiti",
      u8"Symbol",
      u8"Times",
    };
    return std::ranges::find(Names, family.View()) == Names.end();
  }

  bool FontCascade::FastAverageCharWidthIfAvailable(float &width) const noexcept
  {
    bool success = HasValidAverageCharWidth();
    if (success)
    {
      // FIXME: PrimaryFont() might not correspond to FirstFamily().
      width = std::round(PrimaryFont()->AvgCharWidth());
    }
    return success;
  }

  List<LayoutRect> FontCascade::CharacterSelectionRectsForText(const TextRun &run,
                                                               const LayoutRect &selectionRect, uint32 from,
                                                               Maybe<uint32> toOrEndOfRun) const noexcept
  {
    uint32 to = toOrEndOfRun.value_or(run.Length());
    krys_debug_assert(from <= to);

    bool rtl = run.Rtl();

    // FIXME: We could further optimize this by using the simple text codepath when applicable.
    ComplexTextController controller(*this, run);
    controller.Advance(from);

    List<LayoutRect> rects;
    rects.reserve(to - from);
    for (uint32 i = 0; i < to - from; ++i)
    {
      auto current = from + i + 1;
      auto characterRect = selectionRect;
      auto beforeWidth = controller.RunWidthSoFar();
      controller.Advance(current);
      auto afterWidth = controller.RunWidthSoFar();
      characterRect.Move(rtl ? controller.TotalAdvance().Width() - afterWidth : beforeWidth, 0);
      characterRect.SetWidth(LayoutUnit::FromFloatCeil(afterWidth - beforeWidth));
      rects.push_back(characterRect);
    }
    return rects;
  }

  void FontCascade::AdjustSelectionRectForText(bool canUseSimplifiedTextMeasuring, const TextRun &run,
                                               LayoutRect &selectionRect, uint32 from,
                                               Maybe<uint32> to) const noexcept
  {
    uint32 destination = to.value_or(run.Length());

    CodePath codePathToUse = GetCodePath(run);
    if (codePathToUse != CodePath::Complex)
    {
      if (canUseSimplifiedTextMeasuring && CanTakeFixedPitchFastContentMeasuring())
      {
        return AdjustSelectionRectForSimpleTextWithFixedPitch(run, selectionRect, from, destination);
      }
      if (!CanHandleRunAsSimpleText(run, from, destination))
      {
        codePathToUse = CodePath::Complex;
      }
    }

    if (ShouldUseComplexTextController(codePathToUse))
    {
      return AdjustSelectionRectForComplexText(run, selectionRect, from, destination);
    }

    return AdjustSelectionRectForSimpleText(run, selectionRect, from, destination);
  }

  int32 FontCascade::OffsetForPosition(const TextRun &run, float x, bool includePartialGlyphs) const noexcept
  {
    if (ShouldUseComplexTextController(GetCodePath(run)))
    {
      return OffsetForPositionForComplexText(run, x, includePartialGlyphs);
    }

    return OffsetForPositionForSimpleText(run, x, includePartialGlyphs);
  }

  bool FontCascade::CanHandleRunAsSimpleText(const TextRun &, uint32, uint32) const noexcept
  {
    return true;
  }

  bool FontCascade::ShouldUseComplexTextControllerForSimpleText() const noexcept
  {
    // As Font::ApplyTransforms() doesn't shape, we can only use WidthIterator when we know for sure that
    // Font::ApplyTransforms() is not actually needed.
    if (GetFontDescription().VariantCaps() != FontVariantCaps::Normal
        || GetFontDescription().VariantEmoji() == FontVariantEmoji::Emoji)
    {
      return true;
    }

    return EnableKerning() || RequiresShaping();
  }

  void FontCascade::SetCodePath(CodePath p) noexcept
  {
    s_codePath = p;
  }

  FontCascade::CodePath FontCascade::GetCodePath() noexcept
  {
    return s_codePath;
  }

  FontCascade::CodePath FontCascade::GetCodePath(const TextRun &run, Maybe<uint32> from,
                                                 Maybe<uint32> to) const noexcept
  {
    if (s_codePath != CodePath::Auto)
    {
      return s_codePath;
    }

    if (!CanHandleRunAsSimpleText(run, from.value_or(0), to.value_or(run.Length())))
    {
      return CodePath::Complex;
    }

    if (!run.CharacterScanForCodePath())
    {
      return CodePath::Simple;
    }

    // Start from 0 since drawing and highlighting also measure the characters before run->from.
    return CharacterRangeCodePath(run.Text());
  }

  FontCascade::CodePath FontCascade::CharacterRangeCodePath(utf8_stringview text) noexcept
  {
    // FIXME: Should use a UnicodeSet. Note that we can't simply use UnicodeCharacter Property/class because
    // some characters are not 'combining', but still need to go to the complex path. Alternatively, we may as
    // well consider binary search over a sorted list of ranges.
    CodePath result = CodePath::Simple;
    bool previousCharacterIsEmojiGroupCandidate = false;
    for (text::CodePointIterator it {text}; !it.AtEnd(); ++it)
    {
      auto c = *it;
      if (c == text::ZeroWidthJoiner && previousCharacterIsEmojiGroupCandidate)
      {
        return CodePath::Complex;
      }

      previousCharacterIsEmojiGroupCandidate = false;
      if (c < 0x2E5) // U+02E5 through U+02E9 (Modifier Letters : Tone letters)
      {
        continue;
      }
      if (c <= 0x2E9)
      {
        return CodePath::Complex;
      }

      if (c < 0x300) // U+0300 through U+036F Combining diacritical marks
      {
        continue;
      }
      if (c <= 0x36F)
      {
        return CodePath::Complex;
      }

      // U+0591 through U+05CF excluding U+05BE Hebrew combining marks, Hebrew punctuation Paseq, Sof Pasuq
      // and Nun Hafukha
      if (c < 0x0591 || c == 0x05BE)
      {
        continue;
      }
      if (c <= 0x05CF)
      {
        return CodePath::Complex;
      }

      // U+0600 through U+109F Arabic, Syriac, Thaana, NKo, Samaritan, Mandaic,
      // Devanagari, Bengali, Gurmukhi, Gujarati, Oriya, Tamil, Telugu, Kannada,
      // Malayalam, Sinhala, Thai, Lao, Tibetan, Myanmar
      if (c < 0x0600)
      {
        continue;
      }
      if (c <= 0x109F)
      {
        return CodePath::Complex;
      }

      // U+1100 through U+11FF Hangul Jamo (only Ancient Korean should be left here if you precompose;
      // Modern Korean will be precomposed as a result of step A)
      if (c < 0x1100)
      {
        continue;
      }
      if (c <= 0x11FF)
      {
        return CodePath::Complex;
      }

      if (c < 0x135D) // U+135D through U+135F Ethiopic combining marks
      {
        continue;
      }
      if (c <= 0x135F)
      {
        return CodePath::Complex;
      }

      if (c < 0x1700) // U+1780 through U+18AF Tagalog, Hanunoo, Buhid, Taghanwa,Khmer, Mongolian
      {
        continue;
      }
      if (c <= 0x18AF)
      {
        return CodePath::Complex;
      }

      if (c < 0x1900) // U+1900 through U+194F Limbu (Unicode 4.0)
      {
        continue;
      }
      if (c <= 0x194F)
      {
        return CodePath::Complex;
      }

      if (c < 0x1980) // U+1980 through U+19DF New Tai Lue
      {
        continue;
      }
      if (c <= 0x19DF)
      {
        return CodePath::Complex;
      }

      if (c < 0x1A00) // U+1A00 through U+1CFF Buginese, Tai Tham, Balinese, Batak, Lepcha, Vedic
      {
        continue;
      }
      if (c <= 0x1CFF)
      {
        return CodePath::Complex;
      }

      if (c < 0x1DC0) // U+1DC0 through U+1DFF Comining diacritical mark supplement
      {
        continue;
      }
      if (c <= 0x1DFF)
      {
        return CodePath::Complex;
      }

      // U+1E00 through U+2000 characters with diacritics and stacked diacritics
      if (c <= 0x2000)
      {
        result = CodePath::SimpleWithGlyphOverflow;
        continue;
      }

      if (c < 0x20D0) // U+20D0 through U+20FF Combining marks for symbols
      {
        continue;
      }
      if (c <= 0x20FF)
      {
        return CodePath::Complex;
      }

      if (c < 0x26F9)
      {
        continue;
      }
      if (c < 0x26FA)
      {
        return CodePath::Complex;
      }

      if (c < 0x2CEF) // U+2CEF through U+2CF1 Combining marks for Coptic
      {
        continue;
      }
      if (c <= 0x2CF1)
      {
        return CodePath::Complex;
      }

      if (c < 0x302A) // U+302A through U+302F Ideographic and Hangul Tone marks
      {
        continue;
      }
      if (c <= 0x302F)
      {
        return CodePath::Complex;
      }

      if (c < 0x3099)
      {
        continue;
      }
      if (c < 0x309D)
      {
        return CodePath::Complex; // KATAKANA-HIRAGANA (SEMI-)VOICED SOUND MARKS require character composition
      }

      if (c < 0xA67C) // U+A67C through U+A67D Combining marks for old Cyrillic
      {
        continue;
      }
      if (c <= 0xA67D)
      {
        return CodePath::Complex;
      }

      if (c < 0xA6F0) // U+A6F0 through U+A6F1 Combining mark for Bamum
      {
        continue;
      }
      if (c <= 0xA6F1)
      {
        return CodePath::Complex;
      }

      // U+A800 through U+ABFF Nagri, Phags-pa, Saurashtra, Devanagari Extended,
      // Hangul Jamo Ext. A, Javanese, Myanmar Extended A, Tai Viet, Meetei Mayek,
      if (c < 0xA800)
      {
        continue;
      }
      if (c <= 0xABFF)
      {
        return CodePath::Complex;
      }

      if (c < 0xD7B0) // U+D7B0 through U+D7FF Hangul Jamo Ext. B
      {
        continue;
      }
      if (c <= 0xD7FF)
      {
        return CodePath::Complex;
      }

      if (c > 0xFFFF)
      {
        if (c < 0x10A00)
        {
          continue;
        }
        if (c < 0x10A60) // Kharoshthi
        {
          return CodePath::Complex;
        }
        if (c < 0x11000)
        {
          continue;
        }
        if (c < 0x11080) // Brahmi
        {
          return CodePath::Complex;
        }
        if (c < 0x110D0) // Kaithi
        {
          return CodePath::Complex;
        }
        if (c < 0x11100)
        {
          continue;
        }
        if (c < 0x11150) // Chakma
        {
          return CodePath::Complex;
        }
        if (c < 0x11180) // Mahajani
        {
          return CodePath::Complex;
        }
        if (c < 0x111E0) // Sharada
        {
          return CodePath::Complex;
        }
        if (c < 0x11200)
        {
          continue;
        }
        if (c < 0x11250) // Khojki
        {
          return CodePath::Complex;
        }
        if (c < 0x112B0)
        {
          continue;
        }
        if (c < 0x11300) // Khudawadi
        {
          return CodePath::Complex;
        }
        if (c < 0x11380) // Grantha
        {
          return CodePath::Complex;
        }
        if (c < 0x11400)
        {
          continue;
        }
        if (c < 0x11480) // Newa
        {
          return CodePath::Complex;
        }
        if (c < 0x114E0) // Tirhuta
        {
          return CodePath::Complex;
        }
        if (c < 0x11580)
        {
          continue;
        }
        if (c < 0x11600) // Siddham
        {
          return CodePath::Complex;
        }
        if (c < 0x11660) // Modi
        {
          return CodePath::Complex;
        }
        if (c < 0x11680)
        {
          continue;
        }
        if (c < 0x116D0) // Takri
        {
          return CodePath::Complex;
        }
        if (c < 0x11700)
        {
          continue;
        }
        // Ahom, Dogra, Dives Akuru, Nandinagari, Zanabazar Square, Soyombo, Warang Citi, Pau Cin Hau
        if (c < 0x11C00)
        {
          return CodePath::Complex;
        }
        if (c < 0x11C70) // Bhaiksuki
        {
          return CodePath::Complex;
        }
        if (c < 0x11CC0) // Marchen
        {
          return CodePath::Complex;
        }
        if (c < 0x1E900)
        {
          continue;
        }
        if (c < 0x1E960) // Adlam
        {
          return CodePath::Complex;
        }
        if (c < 0x1F1E6) // U+1F1E6 through U+1F1FF Regional Indicator Symbols
        {
          continue;
        }
        if (c <= 0x1F1FF)
        {
          return CodePath::Complex;
        }

        if (IsEmojiFitzpatrickModifier(c))
        {
          return CodePath::Complex;
        }
        if (IsEmojiGroupCandidate(c))
        {
          previousCharacterIsEmojiGroupCandidate = true;
          continue;
        }

        if (c < 0xE0000)
        {
          continue;
        }
        if (c < 0xE0080) // Tags
        {
          return CodePath::Complex;
        }
        if (c < 0xE0100) // U+E0100 through U+E01EF Unicode variation selectors.
        {
          continue;
        }
        if (c <= 0xE01EF)
        {
          return CodePath::Complex;
        }

        // FIXME: Check for Brahmi (U+11000 block), Kaithi (U+11080 block) and other complex scripts
        // in plane 1 or higher.

        continue;
      }

      if (c < 0xFE00) // U+FE00 through U+FE0F Unicode variation selectors
      {
        continue;
      }
      if (c <= 0xFE0F)
      {
        return CodePath::Complex;
      }

      if (c < 0xFE20) // U+FE20 through U+FE2F Combining half marks
      {
        continue;
      }
      if (c <= 0xFE2F)
      {
        return CodePath::Complex;
      }
    }
    return result;
  }

  std::pair<uint32, bool> FontCascade::ExpansionOpportunityCount(utf8_stringview text,
                                                                 TextDirection direction,
                                                                 ExpansionBehavior expansionBehavior) noexcept
  {
    // For each character, iterating from left to right:
    //   If it is recognized as a space, insert an opportunity after it
    //   If it is an ideograph, insert one opportunity before it and one opportunity after it
    // Do this such a way so that there are not two opportunities next to each other.
    uint32 count = 0;
    bool isAfterExpansion = expansionBehavior.Left == ExpansionBehavior::Behavior::Forbid;
    if (expansionBehavior.Left == ExpansionBehavior::Behavior::Force)
    {
      ++count;
      isAfterExpansion = true;
    }

    auto countCharacter = [&](char32 character)
    {
      if (TreatAsSpace(character))
      {
        ++count;
        isAfterExpansion = true;
        return;
      }
      if (CanExpandAroundIdeographsInComplexText() && IsCJKIdeographOrSymbol(character))
      {
        if (!isAfterExpansion)
        {
          ++count;
        }
        ++count;
        isAfterExpansion = true;
        return;
      }
      isAfterExpansion = false;
    };

    if (direction == TextDirection::LTR)
    {
      for (text::CodePointIterator it {text}; !it.AtEnd(); ++it)
      {
        countCharacter(*it);
      }
    }
    else
    {
      SmallList<char32, 64> characters;
      for (text::CodePointIterator it {text}; !it.AtEnd(); ++it)
      {
        characters.push_back(*it);
      }
      for (auto character : characters | std::views::reverse)
      {
        countCharacter(character);
      }
    }

    if (!isAfterExpansion && expansionBehavior.Right == ExpansionBehavior::Behavior::Force)
    {
      ++count;
      isAfterExpansion = true;
    }
    else if (isAfterExpansion && expansionBehavior.Right == ExpansionBehavior::Behavior::Forbid)
    {
      krys_debug_assert(count);
      --count;
      isAfterExpansion = false;
    }
    return std::make_pair(count, isAfterExpansion);
  }

  bool FontCascade::LeftExpansionOpportunity(utf8_stringview text, TextDirection direction) noexcept
  {
    if (text.empty())
    {
      return false;
    }

    char32 initialCharacter =
      direction == TextDirection::LTR ? *text::CodePointIterator {text} : LastCodePoint(text);

    return CanExpandAroundIdeographsInComplexText() && IsCJKIdeographOrSymbol(initialCharacter);
  }

  bool FontCascade::RightExpansionOpportunity(utf8_stringview text, TextDirection direction) noexcept
  {
    if (text.empty())
    {
      return false;
    }

    char32 finalCharacter =
      direction == TextDirection::LTR ? LastCodePoint(text) : *text::CodePointIterator {text};

    return TreatAsSpace(finalCharacter)
           || (CanExpandAroundIdeographsInComplexText() && IsCJKIdeographOrSymbol(finalCharacter));
  }

  // https://www.w3.org/TR/css-text-decor-3/#text-emphasis-style-property
  bool FontCascade::CanReceiveTextEmphasis(char32 c) noexcept
  {
    auto mask = U_GET_GC_MASK(static_cast<UChar32>(c));
    if (mask & (U_GC_Z_MASK | U_GC_CN_MASK | U_GC_CC_MASK | U_GC_CF_MASK))
    {
      return false;
    }

    // Additional word-separator characters listed in CSS Text Level 3 Editor's Draft 3 November 2010.
    // https://www.w3.org/TR/css-text-3/#word-separator
    if (c == text::EthiopicWordspace || c == text::AegeanWordSeparatorLine
        || c == text::AegeanWordSeparatorDot || c == text::UgariticWordDivider
        || c == text::TibetanMarkIntersyllabicTsheg || c == text::TibetanMarkDelimiterTshegBstar)
    {
      return false;
    }

    if (mask & U_GC_P_MASK)
    {
      return c == U'#' || c == U'%' || c == U'&' || c == U'@' || c == text::ArabicIndicPerMilleSign
             || c == text::ArabicIndicPerTenThousandSign || c == text::ArabicPercentSign
             || c == text::FullwidthAmpersand || c == text::FullwidthCommercialAt
             || c == text::FullwidthNumberSign || c == text::FullwidthPercentSign
             || c == text::PartAlternationMark || c == text::PerMilleSign || c == text::PerTenThousandSign
             || c == text::PilcrowSign || c == text::ReversedPilcrowSign || c == text::SectionSign
             || c == text::SmallAmpersand || c == text::SmallCommercialAt || c == text::SmallNumberSign
             || c == text::SmallPercentSign || c == text::SwungDash || c == text::TironianSignEt;
    }

    return true;
  }

  // FIXME: This function may not work if the emphasis mark uses a complex script, but none of the
  // standard emphasis marks do so.
  Maybe<GlyphData> FontCascade::GetEmphasisMarkGlyphData(const StringAtom &mark) const noexcept
  {
    if (mark.View().empty())
    {
      return null;
    }

    char32 character = *text::CodePointIterator {mark.View()};

    auto glyphData = GlyphDataForCharacter(character, false, EmphasisMarkVariant);
    if (!glyphData.IsValid())
    {
      return null;
    }
    return glyphData;
  }

  int32 FontCascade::EmphasisMarkAscent(const StringAtom &mark) const noexcept
  {
    if (auto *font = FontForEmphasisMark(mark))
    {
      return font->GetFontMetrics().IntAscent();
    }
    return 0;
  }

  int32 FontCascade::EmphasisMarkDescent(const StringAtom &mark) const noexcept
  {
    if (auto *font = FontForEmphasisMark(mark))
    {
      return font->GetFontMetrics().IntDescent();
    }
    return 0;
  }

  const Font *FontCascade::FontForEmphasisMark(const StringAtom &mark) const noexcept
  {
    auto markGlyphData = GetEmphasisMarkGlyphData(mark);
    if (!markGlyphData)
    {
      return {};
    }

    krys_debug_assert(markGlyphData->Font);
    return markGlyphData->Font.get();
  }

  int32 FontCascade::EmphasisMarkHeight(const StringAtom &mark) const noexcept
  {
    if (auto *font = FontForEmphasisMark(mark))
    {
      return font->GetFontMetrics().IntHeight();
    }
    return {};
  }

  float FontCascade::FloatEmphasisMarkHeight(const StringAtom &mark) const noexcept
  {
    if (auto *font = FontForEmphasisMark(mark))
    {
      return font->GetFontMetrics().Height();
    }
    return {};
  }

  void FontCascade::AdjustSelectionRectForSimpleText(const TextRun &run, LayoutRect &selectionRect,
                                                     uint32 from, uint32 to) const noexcept
  {
    GlyphBuffer glyphBuffer;
    WidthIterator it(*this, run);
    it.Advance(from, glyphBuffer);
    float beforeWidth = it.RunWidthSoFar();
    it.Advance(to, glyphBuffer);
    float afterWidth = it.RunWidthSoFar();

    if (run.Rtl())
    {
      it.Advance(run.Length(), glyphBuffer);
      it.Finalize(glyphBuffer);
      float totalWidth = it.RunWidthSoFar();
      selectionRect.Move(totalWidth - afterWidth, 0);
    }
    else
    {
      it.Finalize(glyphBuffer);
      selectionRect.Move(beforeWidth, 0);
    }
    selectionRect.SetWidth(LayoutUnit::FromFloatCeil(afterWidth - beforeWidth));
  }

  void FontCascade::AdjustSelectionRectForComplexText(const TextRun &run, LayoutRect &selectionRect,
                                                      uint32 from, uint32 to) const noexcept
  {
    ComplexTextController controller(*this, run);
    controller.Advance(from);
    float beforeWidth = controller.RunWidthSoFar();
    controller.Advance(to);
    float afterWidth = controller.RunWidthSoFar();

    if (run.Rtl())
    {
      selectionRect.Move(controller.TotalAdvance().Width() - afterWidth, 0);
    }
    else
    {
      selectionRect.Move(beforeWidth, 0);
    }
    selectionRect.SetWidth(LayoutUnit::FromFloatCeil(afterWidth - beforeWidth));
  }

  void FontCascade::AdjustSelectionRectForSimpleTextWithFixedPitch(const TextRun &run,
                                                                   LayoutRect &selectionRect, uint32 from,
                                                                   uint32 to) const noexcept
  {
    bool whitespaceIsCollapsed = !run.AllowTabs();
    float beforeWidth = WidthForSimpleTextWithFixedPitch(run.Text().substr(0, from), whitespaceIsCollapsed);
    float afterWidth = WidthForSimpleTextWithFixedPitch(run.Text().substr(0, to), whitespaceIsCollapsed);

    if (run.Rtl())
    {
      float totalWidth = WidthForSimpleTextWithFixedPitch(run.Text(), whitespaceIsCollapsed);
      selectionRect.Move(totalWidth - afterWidth, 0);
    }
    else
    {
      selectionRect.Move(beforeWidth, 0);
    }
    selectionRect.SetWidth(LayoutUnit::FromFloatCeil(afterWidth - beforeWidth));
  }

  int32 FontCascade::OffsetForPositionForSimpleText(const TextRun &run, float x,
                                                    bool includePartialGlyphs) const noexcept
  {
    float delta = x;

    WidthIterator it(*this, run);
    GlyphBuffer localGlyphBuffer;
    uint32 offset;
    if (run.Rtl())
    {
      delta -= Width(CodePath::Simple, run);
      while (true)
      {
        offset = it.CurrentCharacterIndex();
        float w;
        if (!it.AdvanceOneCharacter(w, localGlyphBuffer))
        {
          break;
        }
        delta += w;
        if (includePartialGlyphs)
        {
          if (delta - w / 2 >= 0)
          {
            break;
          }
        }
        else
        {
          if (delta >= 0)
          {
            break;
          }
        }
      }
    }
    else
    {
      while (true)
      {
        offset = it.CurrentCharacterIndex();
        float w;
        if (!it.AdvanceOneCharacter(w, localGlyphBuffer))
        {
          break;
        }
        delta -= w;
        if (includePartialGlyphs)
        {
          if (delta + w / 2 <= 0)
          {
            break;
          }
        }
        else
        {
          if (delta <= 0)
          {
            break;
          }
        }
      }
    }

    it.Finalize(localGlyphBuffer);
    return static_cast<int32>(offset);
  }

  int32 FontCascade::OffsetForPositionForComplexText(const TextRun &run, float x,
                                                     bool includePartialGlyphs) const noexcept
  {
    ComplexTextController controller(*this, run);
    return static_cast<int32>(controller.OffsetForPosition(x, includePartialGlyphs));
  }

  bool ShouldSynthesizeSmallCaps(bool dontSynthesizeSmallCaps, const Font *nextFont, char32 baseCharacter,
                                 Maybe<char32> capitalizedBase, FontVariantCaps fontVariantCaps,
                                 bool engageAllSmallCapsProcessing) noexcept
  {
    if (fontVariantCaps == FontVariantCaps::Normal)
    {
      return false;
    }

    if (dontSynthesizeSmallCaps)
    {
      return false;
    }
    if (!nextFont || nextFont->IsSystemFontFallbackPlaceholder())
    {
      return false;
    }
    if (engageAllSmallCapsProcessing && text::IsUnicodeCompatibleASCIIWhitespace(baseCharacter))
    {
      return false;
    }
    if (!engageAllSmallCapsProcessing && !capitalizedBase)
    {
      return false;
    }
    return !nextFont->VariantCapsSupportedForSynthesis(fontVariantCaps);
  }

  Maybe<char32> Capitalized(char32 baseCharacter) noexcept
  {
    if (U_GET_GC_MASK(static_cast<UChar32>(baseCharacter)) & U_GC_M_MASK)
    {
      return null;
    }

    auto uppercaseCharacter = static_cast<char32>(u_toupper(static_cast<UChar32>(baseCharacter)));
    krys_debug_assert(uppercaseCharacter == baseCharacter
                      || ((baseCharacter <= 0xFFFF) == (uppercaseCharacter <= 0xFFFF)));
    if (uppercaseCharacter != baseCharacter)
    {
      return uppercaseCharacter;
    }
    return null;
  }
}
