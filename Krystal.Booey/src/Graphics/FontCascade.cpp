#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontCache.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    uint32 LastFontCascadeGeneration = 0;
  }

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
    krys_debug_assert(!text.empty());
    char32 baseCharacter = *text::CodePointIterator {text};
    GlyphData baseCharacterGlyphData = GlyphDataForCharacter(baseCharacter, false, NormalVariant);

    if (!baseCharacterGlyphData.IsValid())
    {
      return nullptr;
    }
    return ShareRefPtr(baseCharacterGlyphData.Font.get());
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
}
