#include "Krystal.Booey/Graphics/WidthIterator.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>
#include <unicode/uchar.h>
#include <utility>

namespace krys::boo
{
  /// @brief Iterates the code points of a run's UTF-8 text between two byte offsets.
  class Utf8TextIterator
  {
  private:
    utf8_stringview _text;
    uint32 _currentIndex;
    uint32 _endIndex;

  public:
    Utf8TextIterator(utf8_stringview text, uint32 currentIndex, uint32 endIndex) noexcept
        : _text(text), _currentIndex(currentIndex), _endIndex(endIndex)
    {
    }

    /// @brief Reads the code point at the current index without advancing past it.
    KRYS_NODISCARD bool Consume(char32 &character, uint32 &clusterLength) const noexcept
    {
      if (_currentIndex >= _endIndex)
      {
        return false;
      }

      auto decoded = text::DecodeUTF8(Span<const char8> {_text}.subspan(_currentIndex));
      character = decoded.CodePoint;
      clusterLength = decoded.Length;
      return true;
    }

    void Advance(uint32 advanceLength) noexcept
    {
      _currentIndex += advanceLength;
    }

    void Reset(uint32 index) noexcept
    {
      krys_debug_assert(index <= _endIndex);
      _currentIndex = index;
    }

    KRYS_NODISCARD uint32 CurrentIndex() const noexcept
    {
      return _currentIndex;
    }
  };

  WidthIterator::WidthIterator(const FontCascade &fontCascade, const TextRun &run,
                               HashSet<RefPtr<const Font>> *fallbackFonts, bool accountForGlyphBounds,
                               bool forTextEmphasis) noexcept
      : _fontCascade(fontCascade), _run(run), _fallbackFonts(fallbackFonts), _expansion(run.Expansion()),
        _direction(_run.Direction()),
        _isAfterExpansion(run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Forbid),
        _accountForGlyphBounds(accountForGlyphBounds), _enableKerning(fontCascade.EnableKerning()),
        _requiresShaping(fontCascade.RequiresShaping()), _forTextEmphasis(forTextEmphasis)
  {
    // FIXME: Should we clamp _expansion so it can never be negative?
    if (!_expansion)
    {
      _expansionPerOpportunity = 0;
    }
    else
    {
      uint32 expansionOpportunityCount =
        FontCascade::ExpansionOpportunityCount(_run.Text(), Direction(), run.ExpansionBehavior()).first;
      if (!expansionOpportunityCount)
      {
        _expansionPerOpportunity = 0;
      }
      else
      {
        _expansionPerOpportunity = _expansion / expansionOpportunityCount;
      }
    }
  }

  struct OriginalAdvancesForCharacterTreatedAsSpace
  {
    GlyphBufferStringOffset StringOffset {0};
    bool CharacterIsSpace {false};
    float Advance {0};

    explicit OriginalAdvancesForCharacterTreatedAsSpace(GlyphBufferStringOffset stringOffset, bool isSpace,
                                                        float advance) noexcept
        : StringOffset(stringOffset), CharacterIsSpace(isSpace), Advance(advance)
    {
    }
  };

  auto WidthIterator::ApplyFontTransforms(GlyphBuffer &glyphBuffer, uint32 lastGlyphCount, const Font &font,
                                          CharactersTreatedAsSpace &charactersTreatedAsSpace) noexcept
    -> ApplyFontTransformsResult
  {
    auto glyphBufferSize = glyphBuffer.Size();
    krys_debug_assert(lastGlyphCount <= glyphBufferSize);
    if (lastGlyphCount >= glyphBufferSize)
    {
      return {0, MakeGlyphBufferAdvance()};
    }

    auto advances = glyphBuffer.Advances();
    float beforeWidth = 0;
    for (uint32 i = lastGlyphCount; i < glyphBufferSize; ++i)
    {
      beforeWidth += Width(advances[i]);
    }

    auto initialAdvance = font.ApplyTransforms(
      glyphBuffer, lastGlyphCount, _currentCharacterIndex, _enableKerning, _requiresShaping,
      _fontCascade.GetFontDescription().ComputedLocale(), _run.Text(), Direction());

    glyphBufferSize = glyphBuffer.Size();
    advances = glyphBuffer.Advances();
    auto origins = glyphBuffer.Origins();

    for (uint32 i = lastGlyphCount; i < glyphBufferSize; ++i)
    {
      SetHeight(advances[i], -Height(advances[i]));
      SetY(origins[i], -Y(origins[i]));
    }

    for (uint32 i = lastGlyphCount; i < glyphBufferSize; ++i)
    {
      auto characterIndex = glyphBuffer.UncheckedStringOffsetAt(i);
      auto iterator =
        std::lower_bound(charactersTreatedAsSpace.begin(), charactersTreatedAsSpace.end(), characterIndex,
                         [](const OriginalAdvancesForCharacterTreatedAsSpace &l,
                            GlyphBufferStringOffset r) -> bool { return l.StringOffset < r; });
      if (iterator == charactersTreatedAsSpace.end() || iterator->StringOffset != characterIndex)
      {
        continue;
      }
      auto &originalAdvances = *iterator;
      SetWidth(glyphBuffer.AdvanceAt(i), originalAdvances.Advance);
    }
    charactersTreatedAsSpace.clear();

    float afterWidth = 0;
    for (uint32 i = lastGlyphCount; i < glyphBufferSize; ++i)
    {
      afterWidth += Width(advances[i]);
    }

    return {afterWidth - beforeWidth, initialAdvance};
  }

  static std::pair<bool, bool> ExpansionLocation(bool ideograph, bool treatAsSpace, bool ltr,
                                                 bool isAfterExpansion, bool forbidLeftExpansion,
                                                 bool forbidRightExpansion, bool forceLeftExpansion,
                                                 bool forceRightExpansion) noexcept
  {
    bool expandLeft = ideograph;
    bool expandRight = ideograph;
    if (treatAsSpace)
    {
      if (ltr)
      {
        expandRight = true;
      }
      else
      {
        expandLeft = true;
      }
    }
    if (isAfterExpansion)
    {
      if (ltr)
      {
        expandLeft = false;
      }
      else
      {
        expandRight = false;
      }
    }
    krys_debug_assert(!forbidLeftExpansion || !forceLeftExpansion);
    krys_debug_assert(!forbidRightExpansion || !forceRightExpansion);
    if (forbidLeftExpansion)
    {
      expandLeft = false;
    }
    if (forbidRightExpansion)
    {
      expandRight = false;
    }
    if (forceLeftExpansion)
    {
      expandLeft = true;
    }
    if (forceRightExpansion)
    {
      expandRight = true;
    }
    return std::make_pair(expandLeft, expandRight);
  }

  static void ExpandWithInitialAdvance(GlyphBufferAdvance &advanceToExpand,
                                       const GlyphBufferAdvance &initialAdvance) noexcept
  {
    SetWidth(advanceToExpand, Width(advanceToExpand) + Width(initialAdvance));
    SetHeight(advanceToExpand, Height(advanceToExpand) + Height(initialAdvance));
  }

  void WidthIterator::ApplyInitialAdvance(GlyphBuffer &glyphBuffer, GlyphBufferAdvance initialAdvance,
                                          uint32 lastGlyphCount) noexcept
  {
    krys_debug_assert(glyphBuffer.Size() >= lastGlyphCount);

    if (glyphBuffer.Size() <= lastGlyphCount)
    {
      return;
    }

    krys_debug_assert(lastGlyphCount
                      || (!Width(_leftoverInitialAdvance) && !Height(_leftoverInitialAdvance)));

    if (Rtl() && lastGlyphCount)
    {
      auto &visuallyLastAdvance = glyphBuffer.AdvanceAt(lastGlyphCount);
      ExpandWithInitialAdvance(visuallyLastAdvance, _leftoverInitialAdvance);
      _runWidthSoFar += Width(_leftoverInitialAdvance);
      _leftoverInitialAdvance = MakeGlyphBufferAdvance();
    }

    if (Rtl())
    {
      _leftoverInitialAdvance = initialAdvance;
    }
    else
    {
      if (lastGlyphCount)
      {
        auto &visuallyPreviousAdvance = glyphBuffer.AdvanceAt(lastGlyphCount - 1);
        ExpandWithInitialAdvance(visuallyPreviousAdvance, initialAdvance);
        _runWidthSoFar += Width(initialAdvance);
      }
      else
      {
        glyphBuffer.ExpandInitialAdvance(initialAdvance);
      }
    }
  }

  bool WidthIterator::HasExtraSpacing() const noexcept
  {
    return (_fontCascade.LetterSpacing() || _fontCascade.WordSpacing() || _expansion)
           && !_run.SpacingDisabled();
  }

  static void ResetGlyphBuffer(GlyphBuffer &glyphBuffer, GlyphBufferStringOffset index) noexcept
  {
    krys_debug_assert(index < glyphBuffer.Size());
    glyphBuffer.Shrink(index);
  }

  static void AddToGlyphBuffer(GlyphBuffer &glyphBuffer, Glyph glyph, const Font &font, float width,
                               GlyphBufferStringOffset currentCharacterIndex) noexcept
  {
    glyphBuffer.Add(glyph, font, width, currentCharacterIndex);
  }

  struct SmallCapsState
  {
    RefPtr<const Font> CurrentFont;
    RefPtr<const Font> SynthesizedFont;
    RefPtr<const Font> SmallSynthesizedFont;
    bool IsSmallCaps {false};
    bool IsLastSmallCaps {false};
    bool ShouldSynthesizeCharacter {false};
    const FontVariantCaps VariantCaps {FontVariantCaps::Normal};
    const bool DontSynthesizeSmallCaps {false};
    const bool EngageAllSmallCapsProcessing {false};

    explicit SmallCapsState(const FontCascadeDescription &fontDescription) noexcept
        : VariantCaps {fontDescription.VariantCaps()},
          DontSynthesizeSmallCaps {!fontDescription.HasAutoFontSynthesisSmallCaps()},
          EngageAllSmallCapsProcessing {VariantCaps == FontVariantCaps::AllSmall
                                        || VariantCaps == FontVariantCaps::AllPetite}
    {
    }

    void SetSmallCapsData(const Font *font, const FontDescription &fontDescription) noexcept
    {
      krys_debug_assert(font);
      CurrentFont = ShareRefPtr(font);
      SynthesizedFont = ShareRefPtr(&font->NoSynthesizableFeaturesFont());
      SmallSynthesizedFont = ShareRefPtr(SynthesizedFont->SmallCapsFont(fontDescription));
      IsLastSmallCaps = IsSmallCaps;
      IsSmallCaps = true;
    }

    void Clear() noexcept
    {
      SynthesizedFont = nullptr;
      SmallSynthesizedFont = nullptr;
      IsSmallCaps = false;
    }

    void SetIsSmallCaps(bool isSmallCaps) noexcept
    {
      IsLastSmallCaps = IsSmallCaps;
      IsSmallCaps = isSmallCaps;
    }

    KRYS_NODISCARD bool SkipSmallCapsProcessing() const noexcept
    {
      return VariantCaps == FontVariantCaps::Normal;
    }
  };

  struct AdvanceInternalState
  {
    RefPtr<const Font> CurrentFont;
    RefPtr<const Font> LastFont;
    // RangeFont and CurrentFont are not necessarily the same, since small-caps might change the range fot for
    // a synthesized font, or a small-caps-synthesized font.
    RefPtr<const Font> RangeFont;
    RefPtr<const Font> NextRangeFont;
    GlyphBuffer &Buffer;
    uint32 LastGlyphCount {0};
    Ref<const Font> PrimaryFont;
    float WidthOfCurrentFontRange {0};
    CharactersTreatedAsSpace TreatedAsSpace;
    uint32 CurrentCharacterIndex {0};
    uint32 IndexOfFontTransition {0};

    AdvanceInternalState(GlyphBuffer &glyphBuffer, const Font &primaryFont,
                         uint32 currentCharacterIndex) noexcept
        : CurrentFont {ShareRefPtr(&primaryFont)}, LastFont {CurrentFont}, RangeFont {CurrentFont},
          NextRangeFont {CurrentFont}, Buffer {glyphBuffer}, LastGlyphCount {glyphBuffer.Size()},
          PrimaryFont {ShareRef(primaryFont)}, CurrentCharacterIndex {currentCharacterIndex},
          IndexOfFontTransition {currentCharacterIndex}
    {
    }

    KRYS_NODISCARD bool FontChanged() const noexcept
    {
      return CurrentFont != LastFont;
    }

    void UpdateFont(const Font *newFont) noexcept
    {
      LastFont = std::exchange(CurrentFont, ShareRefPtr(newFont));
    }
  };

  void WidthIterator::CommitCurrentFontRange(AdvanceInternalState &advanceInternalState) noexcept
  {
#if KRYS_DEBUG
    krys_debug_assert(advanceInternalState.RangeFont);
    for (uint32 i = advanceInternalState.LastGlyphCount; i < advanceInternalState.Buffer.Size(); ++i)
    {
      krys_debug_assert(&advanceInternalState.Buffer.FontAt(i) == advanceInternalState.RangeFont.get());
    }
#endif

    auto applyFontTransformsResult =
      ApplyFontTransforms(advanceInternalState.Buffer, advanceInternalState.LastGlyphCount,
                          *advanceInternalState.RangeFont, advanceInternalState.TreatedAsSpace);
    _runWidthSoFar += applyFontTransformsResult.AdditionalAdvance;
    ApplyInitialAdvance(advanceInternalState.Buffer, applyFontTransformsResult.InitialAdvance,
                        advanceInternalState.LastGlyphCount);
    _currentCharacterIndex = advanceInternalState.CurrentCharacterIndex;

    if (advanceInternalState.WidthOfCurrentFontRange && _fallbackFonts
        && advanceInternalState.RangeFont.get() != advanceInternalState.PrimaryFont.get())
    {
      _fallbackFonts->insert(advanceInternalState.RangeFont);
    }

    advanceInternalState.WidthOfCurrentFontRange = 0;
    advanceInternalState.LastGlyphCount = advanceInternalState.Buffer.Size();
  }

  static const Font *FontForRange(const Font *font, const SmallCapsState &smallCapsData,
                                  bool isSmallCaps) noexcept
  {
    if (!smallCapsData.SynthesizedFont)
    {
      return font;
    }

    return isSmallCaps ? smallCapsData.SmallSynthesizedFont.get() : smallCapsData.SynthesizedFont.get();
  }

  void WidthIterator::StartNewFontRangeIfNeeded(AdvanceInternalState &advanceInternalState,
                                                SmallCapsState &smallCapsState,
                                                const FontCascadeDescription &fontDescription) noexcept
  {
    // New range is needed if there is a font change or "small caps" status change.
    if (advanceInternalState.FontChanged() || smallCapsState.IsSmallCaps != smallCapsState.IsLastSmallCaps)
    {
      CommitCurrentFontRange(advanceInternalState);
      if (!smallCapsState.SkipSmallCapsProcessing())
      {
        if (advanceInternalState.FontChanged())
        {
          smallCapsState.Clear();
        }
        if (smallCapsState.ShouldSynthesizeCharacter)
        {
          smallCapsState.SetSmallCapsData(advanceInternalState.CurrentFont.get(), fontDescription);
        }
      }
      advanceInternalState.NextRangeFont = ShareRefPtr(
        FontForRange(advanceInternalState.CurrentFont.get(), smallCapsState, smallCapsState.IsSmallCaps));
      advanceInternalState.IndexOfFontTransition = advanceInternalState.CurrentCharacterIndex;
    }
  }

  static bool ResetFontRangeIfNeeded(AdvanceInternalState &advanceInternalState,
                                     SmallCapsState &smallCapsState,
                                     const FontCascadeDescription &fontDescription,
                                     Utf8TextIterator &textIterator) noexcept
  {
    if (smallCapsState.SkipSmallCapsProcessing())
    {
      return false;
    }

    if (!smallCapsState.SynthesizedFont && smallCapsState.ShouldSynthesizeCharacter)
    {
      // Rather than synthesize each character individually, we should synthesize the entire "run" if any
      // character requires synthesis.
      smallCapsState.SetSmallCapsData(advanceInternalState.CurrentFont.get(), fontDescription);
      textIterator.Reset(advanceInternalState.IndexOfFontTransition);
      ResetGlyphBuffer(advanceInternalState.Buffer, advanceInternalState.LastGlyphCount);
      return true;
    }
    return false;
  }

  static void UpdateCharacterAndSmallCapsIfNeeded(SmallCapsState &smallCapsState,
                                                  Maybe<char32> capitalizedCharacter,
                                                  char32 &characterToWrite) noexcept
  {
    if (smallCapsState.SkipSmallCapsProcessing())
    {
      return;
    }

    if (smallCapsState.SynthesizedFont || smallCapsState.ShouldSynthesizeCharacter)
    {
      if (capitalizedCharacter)
      {
        smallCapsState.SetIsSmallCaps(true);
        characterToWrite = *capitalizedCharacter;
      }
      else
      {
        smallCapsState.SetIsSmallCaps(smallCapsState.EngageAllSmallCapsProcessing);
      }
    }
  }

  void WidthIterator::AdvanceInternal(Utf8TextIterator &textIterator, GlyphBuffer &glyphBuffer) noexcept
  {
    // The core logic here needs to match FontCascade::WidthForTextUsingSimplifiedMeasuring()
    FloatRect bounds;
    auto &fontDescription = _fontCascade.GetFontDescription();
    Ref<const Font> primaryFont = _fontCascade.PrimaryFont();
    AdvanceInternalState advanceInternalState(glyphBuffer, *primaryFont, textIterator.CurrentIndex());
    SmallCapsState smallCapsState(fontDescription);
    char32 character = 0;
    float width = 0;
    uint32 clusterLength = 0;

    // We are iterating in string order, not glyph order.
    if (!textIterator.Consume(character, clusterLength))
    {
      return;
    }

    auto firstGlyphData = _fontCascade.GlyphDataForCharacter(character, false, FontVariant::NormalVariant);
    RefPtr<Font> halfWidthFont;
    auto shouldProcessTextSpacingTrim = !fontDescription.GetTextSpacingTrim().IsSpaceAll();
    if (shouldProcessTextSpacingTrim && firstGlyphData.Font)
    {
      text_spacing::CharactersData charactersData = {
        .CurrentCharacter = character, .CurrentCharacterClass = text_spacing::GetCharacterClass(character)};
      halfWidthFont = text_spacing::GetHalfWidthFontIfNeeded(
        *firstGlyphData.Font, fontDescription.GetTextSpacingTrim(), charactersData);
      if (halfWidthFont)
      {
        firstGlyphData.Font = WeakPtr<const Font>::WithRef(halfWidthFont.get());
      }
    }
    advanceInternalState.UpdateFont(firstGlyphData.Font ? firstGlyphData.Font.get() : primaryFont.get());
    auto capitalizedCharacter = Capitalized(character);
    if (ShouldSynthesizeSmallCaps(smallCapsState.DontSynthesizeSmallCaps,
                                  advanceInternalState.CurrentFont.get(), character, capitalizedCharacter,
                                  smallCapsState.VariantCaps, smallCapsState.EngageAllSmallCapsProcessing))
    {
      smallCapsState.SetSmallCapsData(advanceInternalState.CurrentFont.get(), fontDescription);
    }

    advanceInternalState.RangeFont = ShareRefPtr(
      FontForRange(advanceInternalState.CurrentFont.get(), smallCapsState, smallCapsState.IsSmallCaps));
    advanceInternalState.NextRangeFont = advanceInternalState.RangeFont;

    while (textIterator.Consume(character, clusterLength))
    {
      _containsTabs |= character == text::TabCharacter;
      advanceInternalState.CurrentCharacterIndex = textIterator.CurrentIndex();
      uint32 advanceLength = clusterLength;
      if (advanceInternalState.CurrentCharacterIndex + advanceLength == _run.Length())
      {
        _lastCharacterIndex = advanceInternalState.CurrentCharacterIndex;
      }
      bool characterMustDrawSomething = !IsDefaultIgnorableCodePoint(character);
      capitalizedCharacter = Capitalized(character);
      char32 characterToWrite = character;

      // Characters with the Default_Ignorable unicode property are only overridden when the font doesn't
      // support the code point, so ignore them here to ensure they are not displayed.
      if (!characterMustDrawSomething)
      {
        textIterator.Advance(advanceLength);
        continue;
      }

      auto glyphData = _fontCascade.GlyphDataForCharacter(character, false, FontVariant::NormalVariant);
      if (shouldProcessTextSpacingTrim && glyphData.Font)
      {
        text_spacing::CharactersData charactersData = {
          .CurrentCharacter = character, .CurrentCharacterClass = text_spacing::GetCharacterClass(character)};
        halfWidthFont = text_spacing::GetHalfWidthFontIfNeeded(
          *glyphData.Font, fontDescription.GetTextSpacingTrim(), charactersData);
        if (halfWidthFont)
        {
          glyphData.Font = WeakPtr<const Font>::WithRef(halfWidthFont.get());
        }
      }
      advanceInternalState.UpdateFont(glyphData.Font ? glyphData.Font.get() : primaryFont.get());
      smallCapsState.ShouldSynthesizeCharacter = ShouldSynthesizeSmallCaps(
        smallCapsState.DontSynthesizeSmallCaps, advanceInternalState.CurrentFont.get(), character,
        capitalizedCharacter, smallCapsState.VariantCaps, smallCapsState.EngageAllSmallCapsProcessing);

      UpdateCharacterAndSmallCapsIfNeeded(smallCapsState, capitalizedCharacter, characterToWrite);

      advanceInternalState.RangeFont = ShareRefPtr(
        FontForRange(advanceInternalState.LastFont.get(), smallCapsState, smallCapsState.IsLastSmallCaps));
      StartNewFontRangeIfNeeded(advanceInternalState, smallCapsState, fontDescription);

      if (ResetFontRangeIfNeeded(advanceInternalState, smallCapsState, fontDescription, textIterator))
      {
        continue;
      }

      if (Rtl())
      {
        characterToWrite = u_charMirror(characterToWrite);
      }

      Glyph glyph = glyphData.Glyph;
      if (glyphData.Font.get() != advanceInternalState.NextRangeFont.get() || character != characterToWrite)
      {
        glyph = advanceInternalState.NextRangeFont->GlyphForCharacter(characterToWrite);
      }

      if (!glyph && !characterMustDrawSomething)
      {
        CommitCurrentFontRange(advanceInternalState);

        AddToGlyphBuffer(advanceInternalState.Buffer, DeletedGlyph, *primaryFont, 0,
                         advanceInternalState.CurrentCharacterIndex);

        textIterator.Advance(advanceLength);
        advanceInternalState.CurrentCharacterIndex = textIterator.CurrentIndex();
        advanceInternalState.UpdateFont(primaryFont.get());
        continue;
      }

      // Synthetic bold is applied after shaping, in ApplyCSSVisibilityRules().
      width = advanceInternalState.NextRangeFont->WidthForGlyph(glyph, Font::SyntheticBoldInclusion::Exclude);
      advanceInternalState.WidthOfCurrentFontRange += width;

      if (FontCascade::TreatAsSpace(characterToWrite))
      {
        advanceInternalState.TreatedAsSpace.emplace_back(
          advanceInternalState.CurrentCharacterIndex, characterToWrite == text::Space,
          characterToWrite == text::TabCharacter
            ? width
            : advanceInternalState.NextRangeFont->SpaceWidth(Font::SyntheticBoldInclusion::Exclude));
      }

      if (_accountForGlyphBounds)
      {
        bounds = advanceInternalState.NextRangeFont->BoundsForGlyph(glyph);
        if (!advanceInternalState.CurrentCharacterIndex)
        {
          _firstGlyphOverflow = std::max<float>(0, -bounds.X());
        }
      }

      if (_forTextEmphasis && !FontCascade::CanReceiveTextEmphasis(characterToWrite))
      {
        glyph = DeletedGlyph;
      }

      AddToGlyphBuffer(glyphBuffer, glyph, *advanceInternalState.NextRangeFont, width,
                       advanceInternalState.CurrentCharacterIndex);

      // Advance past the character we just dealt with.
      textIterator.Advance(advanceLength);
      advanceInternalState.CurrentCharacterIndex = textIterator.CurrentIndex();

      _runWidthSoFar += width;

      if (_accountForGlyphBounds)
      {
        _maxGlyphBoundingBoxY = std::max(_maxGlyphBoundingBoxY, bounds.MaxY());
        _minGlyphBoundingBoxY = std::min(_minGlyphBoundingBoxY, bounds.Y());
        _lastGlyphOverflow = std::max<float>(0, bounds.MaxX() - width);
      }
    }

    advanceInternalState.RangeFont = advanceInternalState.NextRangeFont;
    CommitCurrentFontRange(advanceInternalState);
  }

  auto WidthIterator::CalculateAdditionalWidth(GlyphBuffer &glyphBuffer,
                                               GlyphBufferStringOffset currentCharacterIndex,
                                               uint32 leadingGlyphIndex, uint32 trailingGlyphIndex,
                                               float position) const noexcept -> AdditionalWidth
  {
    float leftAdditionalWidth = 0;
    float rightAdditionalWidth = 0;
    float leftExpansionAdditionalWidth = 0;
    float rightExpansionAdditionalWidth = 0;

    auto character = _run.CharacterAt(currentCharacterIndex);

    if (character == text::TabCharacter && _run.AllowTabs())
    {
      auto &font = glyphBuffer.FontAt(trailingGlyphIndex);
      // Synthetic bold will be handled in ApplyCSSVisibilityRules() later.
      auto newWidth =
        _fontCascade.TabWidth(font, _run.TabSize(), position, Font::SyntheticBoldInclusion::Exclude);
      auto currentWidth = Width(glyphBuffer.AdvanceAt(trailingGlyphIndex));
      rightAdditionalWidth += newWidth - currentWidth;
    }

    if (HasExtraSpacing())
    {
      bool treatAsSpace = FontCascade::TreatAsSpace(character);

      // This is a heuristic to determine if the character is non-visible. Non-visible characters don't get
      // letter-spacing.
      float baseWidth = 0;
      for (uint32 i = leadingGlyphIndex; i <= trailingGlyphIndex; ++i)
      {
        baseWidth += Width(glyphBuffer.AdvanceAt(i));
      }
      if (baseWidth)
      {
        rightAdditionalWidth += _fontCascade.LetterSpacing();
      }

      if (treatAsSpace && (character != text::TabCharacter || !_run.AllowTabs())
          && (currentCharacterIndex || character == text::NoBreakSpace) && _fontCascade.WordSpacing())
      {
        rightAdditionalWidth += _fontCascade.WordSpacing();
      }

      if (_expansion > 0)
      {
        bool currentIsLastCharacter =
          _lastCharacterIndex
          && currentCharacterIndex == static_cast<GlyphBufferStringOffset>(*_lastCharacterIndex);

        bool isLeftmostCharacter = !currentCharacterIndex;
        bool isRightmostCharacter = currentIsLastCharacter;
        if (Rtl())
        {
          std::swap(isLeftmostCharacter, isRightmostCharacter);
        }

        bool forceLeftExpansion =
          isLeftmostCharacter && _run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Force;
        bool forceRightExpansion =
          isRightmostCharacter && _run.ExpansionBehavior().Right == ExpansionBehavior::Behavior::Force;
        bool forbidLeftExpansion =
          isLeftmostCharacter && _run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Forbid;
        bool forbidRightExpansion =
          isRightmostCharacter && _run.ExpansionBehavior().Right == ExpansionBehavior::Behavior::Forbid;

        bool isIdeograph = FontCascade::CanExpandAroundIdeographsInComplexText()
                           && FontCascade::IsCJKIdeographOrSymbol(character);

        if (treatAsSpace || isIdeograph || forceLeftExpansion || forceRightExpansion)
        {
          auto [expandLeft, expandRight] =
            ExpansionLocation(isIdeograph, treatAsSpace, Ltr(), _isAfterExpansion, forbidLeftExpansion,
                              forbidRightExpansion, forceLeftExpansion, forceRightExpansion);

          if (expandLeft)
          {
            leftExpansionAdditionalWidth += _expansionPerOpportunity;
          }
          if (expandRight)
          {
            rightExpansionAdditionalWidth += _expansionPerOpportunity;
          }
        }
      }
    }

    return {leftAdditionalWidth, rightAdditionalWidth, leftExpansionAdditionalWidth,
            rightExpansionAdditionalWidth};
  }

  struct GlyphIndexRange
  {
    // This means the character got expanded to glyphs inside the GlyphBuffer at indices [LeadingGlyphIndex,
    // TrailingGlyphIndex].
    uint32 LeadingGlyphIndex;
    uint32 TrailingGlyphIndex;
  };

  void WidthIterator::ApplyAdditionalWidth(GlyphBuffer &glyphBuffer, GlyphIndexRange glyphIndexRange,
                                           float leftAdditionalWidth, float rightAdditionalWidth,
                                           float leftExpansionAdditionalWidth,
                                           float rightExpansionAdditionalWidth) noexcept
  {
    _expansion -= leftExpansionAdditionalWidth + rightExpansionAdditionalWidth;

    leftAdditionalWidth += leftExpansionAdditionalWidth;
    rightAdditionalWidth += rightExpansionAdditionalWidth;

    _runWidthSoFar += leftAdditionalWidth;
    _runWidthSoFar += rightAdditionalWidth;

    if (leftAdditionalWidth)
    {
      if (Ltr())
      {
        // Left additional width in LTR means the previous (leading) glyph's right side gets expanded.
        auto leadingGlyphIndex = glyphIndexRange.LeadingGlyphIndex;
        if (leadingGlyphIndex)
        {
          glyphBuffer.ExpandAdvance(leadingGlyphIndex - 1, leftAdditionalWidth);
        }
        else
        {
          glyphBuffer.ExpandInitialAdvance(leftAdditionalWidth);
        }
      }
      else
      {
        // Left additional width in RTL means the next (trailing) glyph's right side gets expanded.
        auto trailingGlyphIndex = glyphIndexRange.TrailingGlyphIndex;
        if (trailingGlyphIndex + 1 < glyphBuffer.Size())
        {
          glyphBuffer.ExpandAdvance(trailingGlyphIndex + 1, leftAdditionalWidth);
        }
        else
        {
          _leftoverJustificationWidth = leftAdditionalWidth;
          // We can't actually add in this width just yet.
          // Add it in when the client calls Advance() again or Finalize().
          _runWidthSoFar -= _leftoverJustificationWidth;
        }
      }
    }

    if (rightAdditionalWidth)
    {
      // Right additional width means the current glyph's right side gets expanded. This is true for both LTR
      // and RTL.
      glyphBuffer.ExpandAdvance(glyphIndexRange.TrailingGlyphIndex, rightAdditionalWidth);
    }
  }

  void WidthIterator::ApplyExtraSpacingAfterShaping(GlyphBuffer &glyphBuffer, uint32 characterStartIndex,
                                                    uint32 glyphBufferStartIndex,
                                                    uint32 characterDestinationIndex,
                                                    float startingRunWidth) noexcept
  {
    List<Maybe<GlyphIndexRange>> characterIndexToGlyphIndexRange(_run.Length(), null);
    List<float> advanceWidths(_run.Length(), 0);
    for (uint32 i = glyphBufferStartIndex; i < glyphBuffer.Size(); ++i)
    {
      auto stringOffset = glyphBuffer.CheckedStringOffsetAt(i, _run.Length());
      if (!stringOffset)
      {
        continue;
      }
      advanceWidths[*stringOffset] += Width(glyphBuffer.AdvanceAt(i));
      auto &glyphIndexRange = characterIndexToGlyphIndexRange[*stringOffset];
      if (glyphIndexRange)
      {
        glyphIndexRange->TrailingGlyphIndex = i;
      }
      else
      {
        glyphIndexRange = GlyphIndexRange {i, i};
      }
    }

    // SVG can stretch advances
    if (_run.HorizontalGlyphStretch() != 1)
    {
      for (uint32 i = glyphBufferStartIndex; i < glyphBuffer.Size(); ++i)
      {
        // All characters' advances get stretched, except apparently tab characters...
        // This doesn't make much sense, because even tab characters get letter-spacing...
        auto stringOffset = glyphBuffer.CheckedStringOffsetAt(i, _run.Length());
        if (stringOffset && _run.CharacterAt(*stringOffset) == text::TabCharacter)
        {
          continue;
        }

        auto currentAdvance = Width(glyphBuffer.AdvanceAt(i));
        auto newAdvance = currentAdvance * _run.HorizontalGlyphStretch();
        glyphBuffer.ExpandAdvance(i, newAdvance - currentAdvance);
      }
    }

    auto previousCharacterClass = _run.TextSpacingState().LastCharacterClassFromPreviousRun;
    float position = _run.XPos() + startingRunWidth;
    auto textAutospace = _fontCascade.GetTextAutospace();
    for (auto i = characterStartIndex; i < characterDestinationIndex; ++i)
    {
      auto &glyphIndexRange = characterIndexToGlyphIndexRange[i];
      if (!glyphIndexRange)
      {
        continue;
      }

      auto width = CalculateAdditionalWidth(glyphBuffer, i, glyphIndexRange->LeadingGlyphIndex,
                                            glyphIndexRange->TrailingGlyphIndex, position);
      ApplyAdditionalWidth(glyphBuffer, *glyphIndexRange, width.Left, width.Right, width.LeftExpansion,
                           width.RightExpansion);

      auto textAutospaceSpacing = 0.f;
      auto characterClass = text_spacing::CharacterClass::Undefined;
      if (!textAutospace.IsNoAutospace())
      {
        characterClass = text_spacing::GetCharacterClass(_run.CharacterAt(i));
        if (textAutospace.ShouldApplySpacing(characterClass, previousCharacterClass))
        {
          textAutospaceSpacing =
            TextAutospace::TextAutospaceSize(glyphBuffer.FontAt(glyphIndexRange->LeadingGlyphIndex));
          glyphBuffer.ExpandAdvanceToLogicalRight(glyphIndexRange->LeadingGlyphIndex, textAutospaceSpacing);
          _runWidthSoFar += textAutospaceSpacing;
        }
      }
      previousCharacterClass = characterClass;

      _isAfterExpansion = (Ltr() && width.RightExpansion) || (Rtl() && width.LeftExpansion);

      // This isn't quite perfect, because we may come across a tab character in between two glyphs which both
      // report to correspond to a previous character. But, the fundamental concept of tabs isn't really
      // compatible with complex text shaping, so this is probably okay. We can probably just do the best we
      // can here. The only alternative, to calculate this position in glyph-space rather than
      // character-space, is O(n^2) because we're iterating across the string here, rather than glyphs, so we
      // can't keep the calculation up-to-date, which means CalculateAdditionalWidth() would have to calculate
      // the result from scratch whenever it's needed. And we can't do some sort of prefix-sum thing because
      // ApplyAdditionalWidth() would modify the values, so updating the data structure each turn of this loop
      // would also end up being O(n^2). Unfortunately, strings with tabs are more likely to be long
      // data-table kind of strings, which means O(n^2) is not acceptable. Also, even if we did the O(n^2)
      // thing, there would still be cases that wouldn't be perfect (because the fundamental concept of tabs
      // isn't really compatible with complex text shaping), so let's choose the fast-wrong approach here
      // instead of the slow-wrong approach.
      position += advanceWidths[i] + width.Left + width.Right + width.LeftExpansion + width.RightExpansion;
    }
  }

  bool WidthIterator::CharacterCanUseSimplifiedTextMeasuring(char32 codePoint,
                                                             bool whitespaceIsCollapsed) noexcept
  {
    // This function needs to be kept in sync with ApplyCSSVisibilityRules().

    switch (codePoint)
    {
      case text::NewlineCharacter:
      case text::CarriageReturn:   return true;
      case text::TabCharacter:
        if (!whitespaceIsCollapsed)
        {
          return false;
        }
        break;
      case text::NoBreakSpace:
      case text::SoftHyphen:
      case text::LeftToRightMark:
      case text::RightToLeftMark:
      case text::LeftToRightEmbed:
      case text::RightToLeftEmbed:
      case text::LeftToRightOverride:
      case text::RightToLeftOverride:
      case text::LeftToRightIsolate:
      case text::RightToLeftIsolate:
      case text::PopDirectionalFormatting:
      case text::PopDirectionalIsolate:
      case text::FirstStrongIsolate:
      case text::ObjectReplacementCharacter:
      case text::ZeroWidthNoBreakSpace:
      case text::ZeroWidthNonJoiner:
      case text::ZeroWidthJoiner:
      case text::WordJoiner:
      case text::ZeroWidthSpace:
      case text::FunctionApplication:
      case text::InvisibleTimes:
      case text::InvisibleSeparator:
      case text::InhibitSymmetricSwapping:
      case text::ActivateSymmetricSwapping:
      case text::InhibitArabicFormShaping:
      case text::ActivateArabicFormShaping:
      case text::NationalDigitShapes:
      case text::NominalDigitShapes:
      case text::MediumShade:                return false;
      default:                               break;
    }

    if (codePoint >= text::HiraganaLetterSmallA || IsControlCharacter(codePoint))
    {
      return false;
    }
    return true;
  }

  void WidthIterator::ApplyCSSVisibilityRules(GlyphBuffer &glyphBuffer, uint32 glyphBufferStartIndex) noexcept
  {
    // This function needs to be kept in sync with CharacterCanUseSimplifiedTextMeasuring().

    List<uint32> glyphsIndicesToBeDeleted;

    float yPosition = Height(glyphBuffer.InitialAdvance());

    auto adjustForSyntheticBold = [&](auto index)
    {
      auto glyph = glyphBuffer.GlyphAt(index);
      auto syntheticBoldOffset = glyph == DeletedGlyph ? 0 : glyphBuffer.FontAt(index).SyntheticBoldOffset();
      _runWidthSoFar += syntheticBoldOffset;
      auto &advance = glyphBuffer.Advances(index)[0];
      SetWidth(advance, Width(advance) + syntheticBoldOffset);
    };

    auto clobberGlyph = [&](auto index, auto newGlyph)
    {
      glyphBuffer.Glyphs(index)[0] = newGlyph;
    };

    // FIXME: It's technically wrong to call clobberAdvance or deleteGlyph here, because this is after
    // initialAdvances have been applied. If the last glyph in a run needs to have its advance clobbered, but
    // the next run has an initial advance, we need to apply the initial advance on the new clobbered advance,
    // rather than clobbering the initial advance entirely.
    auto clobberAdvance = [&](auto index, auto newAdvance)
    {
      auto advanceBeforeClobbering = glyphBuffer.AdvanceAt(index);
      glyphBuffer.Advances(index)[0] = MakeGlyphBufferAdvance(newAdvance, Height(advanceBeforeClobbering));
      _runWidthSoFar += Width(glyphBuffer.AdvanceAt(index)) - Width(advanceBeforeClobbering);
      glyphBuffer.Origins(index)[0] = MakeGlyphBufferOrigin(0, -yPosition);
    };

    auto deleteGlyph = [&](auto index)
    {
      _runWidthSoFar -= Width(glyphBuffer.AdvanceAt(index));
      glyphBuffer.DeleteGlyphWithoutAffectingSize(index);
    };

    auto makeGlyphInvisible = [&](auto index)
    {
      glyphBuffer.MakeGlyphInvisible(index);
    };

    for (uint32 i = glyphBufferStartIndex; i < glyphBuffer.Size();
         yPosition += Height(glyphBuffer.AdvanceAt(i)), ++i)
    {
      auto stringOffset = glyphBuffer.CheckedStringOffsetAt(i, _run.Length());
      if (!stringOffset)
      {
        continue;
      }
      auto characterResponsibleForThisGlyph = _run.CharacterAt(*stringOffset);

      switch (characterResponsibleForThisGlyph)
      {
        case text::NewlineCharacter:
        case text::CarriageReturn:
          krys_debug_assert(glyphBuffer.Fonts(i)[0]);
          // FIXME: It isn't quite right to use the space glyph here, because the space character may be
          // supposed to render with a totally unrelated font (because of fallback). Instead, we should
          // probably somehow have the caller pass in a Font/glyph pair to use in this situation.
          if (auto spaceGlyph = glyphBuffer.FontAt(i).SpaceGlyph())
          {
            clobberGlyph(i, spaceGlyph);
          }
          adjustForSyntheticBold(i);
          continue;
        case text::NoBreakSpace: adjustForSyntheticBold(i); continue;
        case text::TabCharacter:
          makeGlyphInvisible(i);
          adjustForSyntheticBold(i);
          continue;
        default: break;
      }

      // https://www.w3.org/TR/css-text-3/#white-space-processing
      // "Unsupported Default_ignorable characters must be ignored for text rendering."
      if (FontCascade::IsCharacterWhoseGlyphsShouldBeDeletedForTextRendering(
            characterResponsibleForThisGlyph))
      {
        deleteGlyph(i);
        continue;
      }

      // "Control characters (Unicode category Cc)—other than tabs (U+0009), line feeds (U+000A), carriage
      // returns (U+000D) and sequences that form a segment break—must be rendered as a visible glyph"
      if (IsControlCharacter(characterResponsibleForThisGlyph))
      {
        // Let's assume that .notdef is visible.
        GlyphBufferGlyph visibleGlyph = 0;
        clobberGlyph(i, visibleGlyph);
        clobberAdvance(i, glyphBuffer.FontAt(i).WidthForGlyph(visibleGlyph));
        continue;
      }

      adjustForSyntheticBold(i);
    }
  }

  void WidthIterator::Finalize(GlyphBuffer &buffer) noexcept
  {
    krys_debug_assert(Rtl() || !_leftoverJustificationWidth);
    // In LTR these do nothing. In RTL, these add left width by moving the whole run to the right.
    buffer.ExpandInitialAdvance(_leftoverInitialAdvance);
    _runWidthSoFar += Width(_leftoverInitialAdvance);
    buffer.ExpandInitialAdvance(_leftoverJustificationWidth);
    _runWidthSoFar += _leftoverJustificationWidth;
    _leftoverJustificationWidth = 0;
  }

  void WidthIterator::Advance(uint32 offset, GlyphBuffer &glyphBuffer) noexcept
  {
    _containsTabs = false;
    uint32 length = _run.Length();

    if (offset > length)
    {
      offset = length;
    }

    if (_currentCharacterIndex >= offset)
    {
      return;
    }

    uint32 characterStartIndex = _currentCharacterIndex;
    uint32 glyphBufferStartIndex = glyphBuffer.Size();

    float startingRunWidth = _runWidthSoFar;

    Utf8TextIterator textIterator(_run.Text(), _currentCharacterIndex, offset);
    AdvanceInternal(textIterator, glyphBuffer);

    // In general, we have to apply spacing after shaping, because shaping requires its input to be
    // unperturbed. So, if there's extra spacing to add, do it here after shaping occurs.

    if (glyphBufferStartIndex < glyphBuffer.Size())
    {
      glyphBuffer.ExpandAdvance(glyphBufferStartIndex, _leftoverJustificationWidth);
      _runWidthSoFar += _leftoverJustificationWidth;
      _leftoverJustificationWidth = 0;
    }

    if (HasExtraSpacing() || _containsTabs || _run.HorizontalGlyphStretch() != 1
        || !_fontCascade.GetTextAutospace().IsNoAutospace())
    {
      ApplyExtraSpacingAfterShaping(glyphBuffer, characterStartIndex, glyphBufferStartIndex, offset,
                                    startingRunWidth);
    }

    ApplyCSSVisibilityRules(glyphBuffer, glyphBufferStartIndex);
  }

  // FIXME: It's pretty much never right to advance just one character.
  bool WidthIterator::AdvanceOneCharacter(float &width, GlyphBuffer &glyphBuffer) noexcept
  {
    uint32 oldSize = glyphBuffer.Size();
    auto characterLength =
      _currentCharacterIndex < _run.Length()
        ? text::DecodeUTF8(Span<const char8> {_run.Text()}.subspan(_currentCharacterIndex)).Length
        : 1u;
    Advance(_currentCharacterIndex + characterLength, glyphBuffer);
    float w = 0;
    for (uint32 i = oldSize; i < glyphBuffer.Size(); ++i)
    {
      w += Width(glyphBuffer.AdvanceAt(i));
    }
    width = w;
    return glyphBuffer.Size() > oldSize;
  }
}
