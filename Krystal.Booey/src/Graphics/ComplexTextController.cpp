#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <algorithm>
#include <unicode/uchar.h>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsContinuationByte(char8 c) noexcept
    {
      return (static_cast<uint8>(c) & 0xC0) == 0x80;
    }

    /// @brief The number of code points of @p text that start in [@p from, @p to).
    KRYS_NODISCARD uint32 CodePointCount(utf8_stringview text, uint32 from, uint32 to) noexcept
    {
      to = std::min<uint32>(to, static_cast<uint32>(text.length()));
      uint32 count = 0;
      for (auto i = from; i < to; ++i)
      {
        if (!IsContinuationByte(text[i]))
        {
          ++count;
        }
      }
      return count;
    }

    /// @brief The offset of the code point @p count code points after the one at @p from (or the end of @p
    /// text).
    KRYS_NODISCARD uint32 OffsetAfterCodePoints(utf8_stringview text, uint32 from, uint32 count) noexcept
    {
      auto offset = from;
      while (count && offset < text.length())
      {
        offset += text::DecodeUTF8(Span<const char8> {text}.subspan(offset)).Length;
        --count;
      }
      return offset;
    }

    /// @brief The code point at byte @p offset of @p text, and its length in bytes.
    KRYS_NODISCARD text::DecodedCodePoint CharacterAt(utf8_stringview text, uint32 offset) noexcept
    {
      return text::DecodeUTF8(Span<const char8> {text}.subspan(offset));
    }

    /// @brief Writes @p character to @p buffer at @p offset in place of the code point there if its UTF-8 is
    /// as long, else copies @p original's code point.
    void WriteCharacter(utf8_string &buffer, utf8_stringview original, uint32 offset,
                        char32 character) noexcept
    {
      auto originalLength = CharacterAt(original, offset).Length;
      Array<char8, 4> encoded {};
      uint8 length = 0;
      if (character < 0x80)
      {
        encoded[length++] = static_cast<char8>(character);
      }
      else if (character < 0x800)
      {
        encoded[length++] = static_cast<char8>(0xC0 | (character >> 6));
        encoded[length++] = static_cast<char8>(0x80 | (character & 0x3F));
      }
      else if (character < 0x10000)
      {
        encoded[length++] = static_cast<char8>(0xE0 | (character >> 12));
        encoded[length++] = static_cast<char8>(0x80 | ((character >> 6) & 0x3F));
        encoded[length++] = static_cast<char8>(0x80 | (character & 0x3F));
      }
      else
      {
        encoded[length++] = static_cast<char8>(0xF0 | (character >> 18));
        encoded[length++] = static_cast<char8>(0x80 | ((character >> 12) & 0x3F));
        encoded[length++] = static_cast<char8>(0x80 | ((character >> 6) & 0x3F));
        encoded[length++] = static_cast<char8>(0x80 | (character & 0x3F));
      }

      // FIXME: A capital whose UTF-8 is longer or shorter than its lowercase letter's isn't synthesized.
      if (length != originalLength)
      {
        for (uint32 i = 0; i < originalLength; ++i)
        {
          buffer[offset + i] = original[offset + i];
        }
        return;
      }
      for (uint32 i = 0; i < length; ++i)
      {
        buffer[offset + i] = encoded[i];
      }
    }
  }

  void ComplexTextController::ComputeExpansionOpportunity() noexcept
  {
    if (!_expansion)
    {
      _expansionPerOpportunity = 0;
    }
    else
    {
      uint32 expansionOpportunityCount =
        FontCascade::ExpansionOpportunityCount(
          _run.Text(), _run.Ltr() ? TextDirection::LTR : TextDirection::RTL, _run.ExpansionBehavior())
          .first;

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

  ComplexTextController::ComplexTextController(const FontCascade &fontCascade, const TextRun &run,
                                               bool mayUseNaturalWritingDirection,
                                               HashSet<RefPtr<const Font>> *fallbackFonts,
                                               bool forTextEmphasis) noexcept
      : _fallbackFonts(fallbackFonts), _fontCascade(fontCascade), _run(run), _end(run.Length()),
        _expansion(run.Expansion()), _mayUseNaturalWritingDirection(mayUseNaturalWritingDirection),
        _forTextEmphasis(forTextEmphasis), _textSpacingState(run.TextSpacingState())
  {
    ComputeExpansionOpportunity();

    CollectComplexTextRuns();

    FinishConstruction();
  }

  ComplexTextController::ComplexTextController(const FontCascade &fontCascade, const TextRun &run,
                                               List<Ref<ComplexTextRun>> &runs) noexcept
      : _fontCascade(fontCascade), _run(run), _end(run.Length()), _expansion(run.Expansion())
  {
    ComputeExpansionOpportunity();

    for (auto &complexTextRun : runs)
    {
      _complexTextRuns.push_back(complexTextRun);
    }

    FinishConstruction();
  }

  ComplexTextController::ComplexTextController(const TextRun &run, const FontCascade &fontCascade) noexcept
      : _fontCascade(fontCascade), _run(run), _end(run.Length())
  {
  }

  ComplexTextController::~ComplexTextController() noexcept = default;

  std::pair<float, float>
    ComplexTextController::EnclosingGlyphBoundsForTextRun(const FontCascade &fontCascade,
                                                          const TextRun &textRun) noexcept
  {
    auto textController = ComplexTextController {textRun, fontCascade};
    textController.CollectComplexTextRuns();

    Maybe<float> enclosingAscent;
    Maybe<float> enclosingDescent;

    for (auto &complexTextRun : textController._complexTextRuns)
    {
      auto &font = complexTextRun->GetFont();
      auto glyphs = complexTextRun->Glyphs();
      krys_debug_assert(glyphs.size() == complexTextRun->GlyphCount());

      for (auto glyph : glyphs)
      {
        auto bounds = font.BoundsForGlyph(glyph);
        enclosingAscent = std::min(enclosingAscent.value_or(bounds.Y()), bounds.Y());
        enclosingDescent = std::max(enclosingDescent.value_or(bounds.MaxY()), bounds.MaxY());
      }
    }
    return {enclosingAscent.value_or(0.f), enclosingDescent.value_or(0.f)};
  }

  List<float> ComplexTextController::GlyphAdvancesForTextRun(const FontCascade &fontCascade,
                                                             const TextRun &textRun) noexcept
  {
    krys_debug_assert(textRun.Rtl());
    auto textController = ComplexTextController {fontCascade, textRun};

    size_t numberOfCharacters = 0;
    for (auto &complexTextRun : textController._complexTextRuns)
    {
      numberOfCharacters += complexTextRun->StringLength();
    }

    List<float> resolvedAdvances(numberOfCharacters, 0.f);
    size_t offset = 0;
    for (auto runIndex = textController._complexTextRuns.size(); runIndex--;)
    {
      auto &complexRun = textController._complexTextRuns[runIndex];
      auto advances = complexRun->BaseAdvances();
      for (size_t index = 0; index < advances.size(); ++index)
      {
        resolvedAdvances[offset + complexRun->IndexAt(static_cast<uint32>(index))] += advances[index].Width();
      }
      offset += complexRun->StringLength();
    }
    return resolvedAdvances;
  }

  void ComplexTextController::FinishConstruction() noexcept
  {
    AdjustGlyphsAndAdvances();

    if (!_isLTROnly)
    {
      auto length = static_cast<uint32>(_complexTextRuns.size());
      _runIndices.clear();
      for (uint32 i = 0; i < length; ++i)
      {
        _runIndices.push_back(length - i - 1);
      }
      std::ranges::sort(_runIndices, [this](auto a, auto b)
                        { return StringBegin(*_complexTextRuns[a]) < StringBegin(*_complexTextRuns[b]); });

      uint32 glyphCountSoFar = 0;
      _glyphCountFromStartToIndex.clear();
      for (uint32 i = 0; i < length; ++i)
      {
        _glyphCountFromStartToIndex.push_back(glyphCountSoFar);
        glyphCountSoFar += _complexTextRuns[i]->GlyphCount();
      }
    }
  }

  uint32 ComplexTextController::OffsetForPosition(float h, bool includePartialGlyphs) noexcept
  {
    if (h >= _totalAdvance.Width())
    {
      return _run.Ltr() ? _end : 0;
    }

    if (h < 0)
    {
      return _run.Ltr() ? 0 : _end;
    }

    float x = h;

    size_t runCount = _complexTextRuns.size();
    uint32 offsetIntoAdjustedGlyphs = 0;

    for (size_t r = 0; r < runCount; ++r)
    {
      const ComplexTextRun &complexTextRun = *_complexTextRuns[r];
      auto characters = complexTextRun.Characters();
      for (uint32 j = 0; j < complexTextRun.GlyphCount(); ++j)
      {
        uint32 index = offsetIntoAdjustedGlyphs + j;
        float adjustedAdvance = _adjustedBaseAdvances[index].Width();
        bool hit = _run.Ltr() ? x < adjustedAdvance : (x <= adjustedAdvance && adjustedAdvance);
        if (hit)
        {
          uint32 hitGlyphStart = complexTextRun.IndexAt(j);
          uint32 hitGlyphEnd;
          if (_run.Ltr())
          {
            hitGlyphEnd =
              std::max(hitGlyphStart, j + 1 < complexTextRun.GlyphCount() ? complexTextRun.IndexAt(j + 1)
                                                                          : complexTextRun.IndexEnd());
          }
          else
          {
            hitGlyphEnd =
              std::max(hitGlyphStart, j > 0 ? complexTextRun.IndexAt(j - 1) : complexTextRun.IndexEnd());
          }

          // The glyph's characters, rather than its bytes, divide its advance.
          auto hitGlyphCharacters = CodePointCount(characters, hitGlyphStart, hitGlyphEnd);

          // FIXME: Instead of dividing the glyph's advance equally between the characters, this could use the
          // glyph's "ligature carets".
          uint32 hitIndex;
          if (_run.Ltr())
          {
            hitIndex = OffsetAfterCodePoints(characters, hitGlyphStart,
                                             static_cast<uint32>(hitGlyphCharacters * (x / adjustedAdvance)));
          }
          else
          {
            if (hitGlyphStart == hitGlyphEnd)
            {
              hitIndex = hitGlyphStart;
            }
            else if (x)
            {
              auto charactersFromStart =
                static_cast<uint32>(hitGlyphCharacters - hitGlyphCharacters * (x / adjustedAdvance));
              hitIndex = OffsetAfterCodePoints(characters, hitGlyphStart, charactersFromStart);
            }
            else
            {
              hitIndex = OffsetAfterCodePoints(characters, hitGlyphStart, hitGlyphCharacters - 1);
            }
          }

          uint32 stringLength = complexTextRun.StringLength();
          CachedTextBreakIterator cursorPositionIterator(characters, {}, TextBreakIterator::CaretMode {},
                                                         StringAtom::Null());
          uint32 clusterStart;
          if (cursorPositionIterator.IsBoundary(hitIndex))
          {
            clusterStart = hitIndex;
          }
          else
          {
            clusterStart = cursorPositionIterator.Preceding(hitIndex).value_or(0);
          }

          if (!includePartialGlyphs)
          {
            return complexTextRun.StringLocation() + clusterStart;
          }

          uint32 clusterEnd = cursorPositionIterator.Following(hitIndex).value_or(stringLength);

          float clusterWidth;
          // FIXME: The search stops at the boundaries of complexTextRun. In theory, it should go on into
          // neighboring ComplexTextRuns shaped from the same item. In practice, we do not expect there to be
          // more than one run per item, as no reordering and no font fallback should occur within an item.
          if (CodePointCount(characters, clusterStart, clusterEnd) > 1)
          {
            clusterWidth = adjustedAdvance;
            if (j)
            {
              uint32 firstGlyphBeforeCluster = j - 1;
              while (complexTextRun.IndexAt(firstGlyphBeforeCluster) >= clusterStart
                     && complexTextRun.IndexAt(firstGlyphBeforeCluster) < clusterEnd)
              {
                float width =
                  _adjustedBaseAdvances[offsetIntoAdjustedGlyphs + firstGlyphBeforeCluster].Width();
                clusterWidth += width;
                x += width;
                if (!firstGlyphBeforeCluster)
                {
                  break;
                }
                firstGlyphBeforeCluster--;
              }
            }
            uint32 firstGlyphAfterCluster = j + 1;
            while (firstGlyphAfterCluster < complexTextRun.GlyphCount()
                   && complexTextRun.IndexAt(firstGlyphAfterCluster) >= clusterStart
                   && complexTextRun.IndexAt(firstGlyphAfterCluster) < clusterEnd)
            {
              clusterWidth +=
                _adjustedBaseAdvances[offsetIntoAdjustedGlyphs + firstGlyphAfterCluster].Width();
              firstGlyphAfterCluster++;
            }
          }
          else
          {
            clusterWidth = adjustedAdvance / std::max<uint32>(hitGlyphCharacters, 1);
            x -= clusterWidth
                 * (_run.Ltr() ? CodePointCount(characters, hitGlyphStart, hitIndex)
                               : CodePointCount(characters, hitIndex, hitGlyphEnd) - 1);
          }
          if (x <= clusterWidth / 2)
          {
            return complexTextRun.StringLocation() + (_run.Ltr() ? clusterStart : clusterEnd);
          }
          return complexTextRun.StringLocation() + (_run.Ltr() ? clusterEnd : clusterStart);
        }
        x -= adjustedAdvance;
      }
      offsetIntoAdjustedGlyphs += complexTextRun.GlyphCount();
    }

    krys_debug_assert(false);
    return 0;
  }

  void ComplexTextController::AdvanceByCombiningCharacterSequence(
    const CachedTextBreakIterator &graphemeClusterIterator, uint32 &currentIndex,
    char32 &baseCharacter) noexcept
  {
    uint32 remainingCharacters = _end - currentIndex;
    krys_debug_assert(remainingCharacters);

    auto decoded = CharacterAt(_run.Text(), currentIndex);
    baseCharacter = decoded.CodePoint;
    if (!decoded.IsValid)
    {
      currentIndex += decoded.Length;
      return;
    }

    uint32 delta = remainingCharacters;
    if (auto following = graphemeClusterIterator.Following(currentIndex))
    {
      delta = *following - currentIndex;
    }
    currentIndex += delta;
  }

  void ComplexTextController::CollectComplexTextRuns() noexcept
  {
    if (!_end || !_fontCascade.Size())
    {
      return;
    }

    // We break up glyph run generation for the string by Font.
    utf8_stringview baseOfString = _run.Text();

    auto &fontDescription = _fontCascade.GetFontDescription();
    auto fontVariantCaps = fontDescription.VariantCaps();
    bool dontSynthesizeSmallCaps = !fontDescription.HasAutoFontSynthesisSmallCaps();
    bool engageAllSmallCapsProcessing =
      fontVariantCaps == FontVariantCaps::AllSmall || fontVariantCaps == FontVariantCaps::AllPetite;
    bool engageSmallCapsProcessing = engageAllSmallCapsProcessing || fontVariantCaps == FontVariantCaps::Small
                                     || fontVariantCaps == FontVariantCaps::Petite;
    auto shouldProcessTextSpacingTrim = !_fontCascade.GetTextSpacingTrim().IsSpaceAll();

    if (engageAllSmallCapsProcessing || engageSmallCapsProcessing)
    {
      _smallCapsBuffer.assign(baseOfString);
    }

    uint32 currentIndex = 0;
    uint32 indexOfFontTransition = 0;

    RefPtr<const Font> font;
    RefPtr<const Font> nextFont;
    RefPtr<const Font> synthesizedFont;
    RefPtr<const Font> smallSynthesizedFont;
    RefPtr<const Font> halfWidthFont;

    CachedTextBreakIterator graphemeClusterIterator(baseOfString, {}, TextBreakIterator::CharacterMode {},
                                                    fontDescription.ComputedLocale());

    char32 baseCharacter;
    AdvanceByCombiningCharacterSequence(graphemeClusterIterator, currentIndex, baseCharacter);

    // We don't perform font fallback on the capitalized characters when small caps is synthesized.
    // We may want to change this code to do so in the future.
    nextFont = _fontCascade.FontForCombiningCharacterSequence(baseOfString.substr(0, currentIndex));

    if (shouldProcessTextSpacingTrim && nextFont && !nextFont->IsSystemFontFallbackPlaceholder())
    {
      text_spacing::CharactersData charactersData = {.CurrentCharacter = baseCharacter,
                                                     .CurrentCharacterClass =
                                                       text_spacing::GetCharacterClass(baseCharacter)};
      halfWidthFont =
        text_spacing::GetHalfWidthFontIfNeeded(*nextFont, _fontCascade.GetTextSpacingTrim(), charactersData);
      if (halfWidthFont)
      {
        nextFont = halfWidthFont;
      }
    }

    bool isSmallCaps = false;
    bool nextIsSmallCaps = false;

    auto capitalizedBase = Capitalized(baseCharacter);
    if (ShouldSynthesizeSmallCaps(dontSynthesizeSmallCaps, nextFont.get(), baseCharacter, capitalizedBase,
                                  fontVariantCaps, engageAllSmallCapsProcessing))
    {
      synthesizedFont = ShareRefPtr(&nextFont->NoSynthesizableFeaturesFont());
      smallSynthesizedFont = ShareRefPtr(synthesizedFont->SmallCapsFont(fontDescription));
      char32 characterToWrite = capitalizedBase ? *capitalizedBase : CharacterAt(baseOfString, 0).CodePoint;
      WriteCharacter(_smallCapsBuffer, baseOfString, 0, characterToWrite);
      nextIsSmallCaps = true;
    }

    while (currentIndex < _end)
    {
      font = nextFont;
      isSmallCaps = nextIsSmallCaps;
      auto previousIndex = currentIndex;
      halfWidthFont = nullptr;

      AdvanceByCombiningCharacterSequence(graphemeClusterIterator, currentIndex, baseCharacter);

      if (synthesizedFont)
      {
        if (auto capitalizedCharacter = Capitalized(baseCharacter))
        {
          WriteCharacter(_smallCapsBuffer, baseOfString, previousIndex, *capitalizedCharacter);
          nextIsSmallCaps = true;
        }
        else
        {
          nextIsSmallCaps = engageAllSmallCapsProcessing;
        }
      }

      nextFont = _fontCascade.FontForCombiningCharacterSequence(
        baseOfString.substr(previousIndex, currentIndex - previousIndex));

      if (shouldProcessTextSpacingTrim && nextFont && !nextFont->IsSystemFontFallbackPlaceholder())
      {
        text_spacing::CharactersData charactersData = {.CurrentCharacter = baseCharacter,
                                                       .CurrentCharacterClass =
                                                         text_spacing::GetCharacterClass(baseCharacter)};
        halfWidthFont = text_spacing::GetHalfWidthFontIfNeeded(*nextFont, _fontCascade.GetTextSpacingTrim(),
                                                               charactersData);
        if (halfWidthFont)
        {
          nextFont = halfWidthFont;
        }
      }

      capitalizedBase = Capitalized(baseCharacter);
      if (!synthesizedFont
          && ShouldSynthesizeSmallCaps(dontSynthesizeSmallCaps, nextFont.get(), baseCharacter,
                                       capitalizedBase, fontVariantCaps, engageAllSmallCapsProcessing))
      {
        // Rather than synthesize each character individually, we should synthesize the entire "run" if any
        // character requires synthesis.
        synthesizedFont = ShareRefPtr(&nextFont->NoSynthesizableFeaturesFont());
        smallSynthesizedFont = ShareRefPtr(synthesizedFont->SmallCapsFont(fontDescription));
        nextIsSmallCaps = true;
        currentIndex = indexOfFontTransition;
        continue;
      }

      if (nextFont != font || nextIsSmallCaps != isSmallCaps)
      {
        uint32 itemLength = previousIndex - indexOfFontTransition;
        if (itemLength)
        {
          uint32 itemStart = indexOfFontTransition;
          if (synthesizedFont)
          {
            if (isSmallCaps)
            {
              CollectComplexTextRunsForCharacters(
                utf8_stringview {_smallCapsBuffer}.substr(itemStart, itemLength), itemStart,
                smallSynthesizedFont.get());
            }
            else
            {
              CollectComplexTextRunsForCharacters(baseOfString.substr(itemStart, itemLength), itemStart,
                                                  synthesizedFont.get());
            }
          }
          else
          {
            CollectComplexTextRunsForCharacters(baseOfString.substr(itemStart, itemLength), itemStart,
                                                font.get());
          }
          if (nextFont != font)
          {
            synthesizedFont = nullptr;
            smallSynthesizedFont = nullptr;
            nextIsSmallCaps = false;
          }
        }
        indexOfFontTransition = previousIndex;
      }
    }

    krys_debug_assert(_end >= indexOfFontTransition);
    uint32 itemLength = _end - indexOfFontTransition;
    if (itemLength)
    {
      uint32 itemStart = indexOfFontTransition;
      if (synthesizedFont)
      {
        if (nextIsSmallCaps)
        {
          CollectComplexTextRunsForCharacters(
            utf8_stringview {_smallCapsBuffer}.substr(itemStart, itemLength), itemStart,
            smallSynthesizedFont.get());
        }
        else
        {
          CollectComplexTextRunsForCharacters(baseOfString.substr(itemStart, itemLength), itemStart,
                                              synthesizedFont.get());
        }
      }
      else
      {
        CollectComplexTextRunsForCharacters(baseOfString.substr(itemStart, itemLength), itemStart,
                                            nextFont.get());
      }
    }

    if (!_run.Ltr())
    {
      std::ranges::reverse(_complexTextRuns);
    }
  }

  void ComplexTextController::ComplexTextRun::SetIsNonMonotonic() noexcept
  {
    krys_debug_assert(_isMonotonic);
    _isMonotonic = false;

    SmallList<bool, 64> mappedIndices(StringLength(), false);
    for (uint32 i = 0; i < _glyphCount; ++i)
    {
      mappedIndices[IndexAt(i)] = true;
    }

    _glyphEndOffsets.resize(_glyphCount);
    for (uint32 i = 0; i < _glyphCount; ++i)
    {
      uint32 nextMappedIndex = _indexEnd;
      for (uint32 j = IndexAt(i) + 1; j < StringLength(); ++j)
      {
        if (mappedIndices[j])
        {
          nextMappedIndex = j;
          break;
        }
      }
      _glyphEndOffsets[i] = nextMappedIndex;
    }
  }

  uint32 ComplexTextController::IndexOfCurrentRun(uint32 &leftmostGlyph) noexcept
  {
    leftmostGlyph = 0;

    auto runCount = static_cast<uint32>(_complexTextRuns.size());
    if (_currentRun >= runCount)
    {
      return runCount;
    }

    if (_isLTROnly)
    {
      for (uint32 i = 0; i < _currentRun; ++i)
      {
        leftmostGlyph += _complexTextRuns[i]->GlyphCount();
      }
      return _currentRun;
    }

    uint32 currentRunIndex = _runIndices[_currentRun];
    leftmostGlyph = _glyphCountFromStartToIndex[currentRunIndex];
    return currentRunIndex;
  }

  uint32 ComplexTextController::IncrementCurrentRun(uint32 &leftmostGlyph) noexcept
  {
    if (_isLTROnly)
    {
      leftmostGlyph += _complexTextRuns[_currentRun++]->GlyphCount();
      return _currentRun;
    }

    _currentRun++;
    leftmostGlyph = 0;
    return IndexOfCurrentRun(leftmostGlyph);
  }

  float ComplexTextController::RunWidthSoFarFraction(const ComplexTextRun &complexTextRun,
                                                     uint32 glyphStartOffset, uint32 glyphEndOffset,
                                                     uint32 oldCharacterInCurrentGlyph,
                                                     GlyphIterationStyle iterationStyle) const noexcept
  {
    // FIXME: Instead of dividing the glyph's advance equally between the characters, this could use the
    // glyph's "ligature carets".
    if (glyphStartOffset == glyphEndOffset)
    {
      // When there are multiple glyphs per character we need to advance by the full width of the glyph.
      krys_debug_assert(_characterInCurrentGlyph == oldCharacterInCurrentGlyph);
      return 1;
    }

    if (iterationStyle == GlyphIterationStyle::ByWholeGlyphs)
    {
      if (!oldCharacterInCurrentGlyph)
      {
        return 1;
      }
      return 0;
    }

    // The glyph's characters, rather than its bytes, divide its advance: a character counts once its first
    // byte is reached.
    auto characters = complexTextRun.Characters();
    auto glyphCharacters = CodePointCount(characters, glyphStartOffset, glyphEndOffset);
    auto reached = CodePointCount(characters, glyphStartOffset, glyphStartOffset + _characterInCurrentGlyph);
    auto reachedBefore =
      CodePointCount(characters, glyphStartOffset, glyphStartOffset + oldCharacterInCurrentGlyph);
    return static_cast<float>(reached - reachedBefore) / std::max<uint32>(glyphCharacters, 1);
  }

  void ComplexTextController::Advance(uint32 offset, GlyphBuffer *glyphBuffer,
                                      GlyphIterationStyle iterationStyle,
                                      HashSet<RefPtr<const Font>> *fallbackFonts) noexcept
  {
    if (offset > _end)
    {
      offset = _end;
    }

    if (offset < _currentCharacter)
    {
      _runWidthSoFar = 0;
      _numGlyphsSoFar = 0;
      _currentRun = 0;
      _glyphInCurrentRun = 0;
      _characterInCurrentGlyph = 0;
    }

    _currentCharacter = offset;

    auto runCount = static_cast<uint32>(_complexTextRuns.size());

    uint32 indexOfLeftmostGlyphInCurrentRun = 0; // Relative to the beginning of ComplexTextController.
    uint32 currentRunIndex = IndexOfCurrentRun(indexOfLeftmostGlyphInCurrentRun);
    while (_currentRun < runCount)
    {
      const ComplexTextRun &complexTextRun = *_complexTextRuns[currentRunIndex];
      bool ltr = complexTextRun.IsLTR();
      uint32 glyphCount = complexTextRun.GlyphCount();
      uint32 glyphIndexIntoCurrentRun = ltr ? _glyphInCurrentRun : glyphCount - 1 - _glyphInCurrentRun;
      uint32 glyphIndexIntoComplexTextController =
        indexOfLeftmostGlyphInCurrentRun + glyphIndexIntoCurrentRun;
      if (fallbackFonts && &complexTextRun.GetFont() != _fontCascade.PrimaryFont().get())
      {
        fallbackFonts->insert(ShareRefPtr(&complexTextRun.GetFont()));
      }

      // We must store the initial advance for the first glyph we are going to draw.
      // When leftmostGlyph is 0, it represents the first glyph to draw, taking into
      // account the text direction.
      if (!indexOfLeftmostGlyphInCurrentRun && glyphBuffer)
      {
        glyphBuffer->SetInitialAdvance(MakeGlyphBufferAdvance(complexTextRun.InitialAdvance()));
      }

      while (_glyphInCurrentRun < glyphCount)
      {
        uint32 glyphStartOffset = complexTextRun.IndexAt(glyphIndexIntoCurrentRun);
        uint32 glyphEndOffset;
        if (complexTextRun.IsMonotonic())
        {
          if (ltr)
          {
            glyphEndOffset =
              std::max(glyphStartOffset, glyphIndexIntoCurrentRun + 1 < glyphCount
                                           ? complexTextRun.IndexAt(glyphIndexIntoCurrentRun + 1)
                                           : complexTextRun.IndexEnd());
          }
          else
          {
            glyphEndOffset =
              std::max(glyphStartOffset, glyphIndexIntoCurrentRun > 0
                                           ? complexTextRun.IndexAt(glyphIndexIntoCurrentRun - 1)
                                           : complexTextRun.IndexEnd());
          }
        }
        else
        {
          glyphEndOffset = complexTextRun.EndOffsetAt(glyphIndexIntoCurrentRun);
        }

        FloatSize adjustedBaseAdvance = _adjustedBaseAdvances[glyphIndexIntoComplexTextController];

        if (glyphStartOffset + complexTextRun.StringLocation() >= _currentCharacter)
        {
          return;
        }

        if (glyphBuffer && !_characterInCurrentGlyph)
        {
          krys_debug_assert(_textAutoSpaceSpacings.empty()
                            || _textAutoSpaceSpacings.size() == _adjustedBaseAdvances.size());
          auto textAutoSpaceSpacing = _textAutoSpaceSpacings.empty()
                                        ? 0.f
                                        : _textAutoSpaceSpacings[glyphIndexIntoComplexTextController];
          auto currentGlyphOrigin = GlyphOrigin(glyphIndexIntoComplexTextController);
          currentGlyphOrigin.Move(textAutoSpaceSpacing, 0);
          GlyphBufferAdvance paintAdvance = MakeGlyphBufferAdvance(adjustedBaseAdvance);
          if (!glyphIndexIntoCurrentRun)
          {
            // The first layout advance of every run includes the "initial layout advance." However, here, we
            // need paint advances, so subtract it out before transforming the layout advance into a paint
            // advance.
            SetWidth(paintAdvance, Width(paintAdvance)
                                     - (complexTextRun.InitialAdvance().Width() - currentGlyphOrigin.X()));
            SetHeight(paintAdvance, Height(paintAdvance)
                                      - (complexTextRun.InitialAdvance().Height() - currentGlyphOrigin.Y()));
          }
          SetWidth(paintAdvance, Width(paintAdvance)
                                   + GlyphOrigin(glyphIndexIntoComplexTextController + 1).X()
                                   - currentGlyphOrigin.X());
          SetHeight(paintAdvance, Height(paintAdvance)
                                    + GlyphOrigin(glyphIndexIntoComplexTextController + 1).Y()
                                    - currentGlyphOrigin.Y());
          if (glyphIndexIntoCurrentRun == glyphCount - 1 && currentRunIndex + 1 < runCount)
          {
            // Our paint advance points to the end of the run. However, the next run may have an
            // initial advance, and our paint advance needs to point to the location of the next
            // glyph. So, we need to add in the next run's initial advance.
            SetWidth(paintAdvance, Width(paintAdvance)
                                     - GlyphOrigin(glyphIndexIntoComplexTextController + 1).X()
                                     + _complexTextRuns[currentRunIndex + 1]->InitialAdvance().Width());
            SetHeight(paintAdvance, Height(paintAdvance)
                                      - GlyphOrigin(glyphIndexIntoComplexTextController + 1).Y()
                                      + _complexTextRuns[currentRunIndex + 1]->InitialAdvance().Height());
          }
          SetHeight(paintAdvance, -Height(paintAdvance)); // Increasing y points down
          glyphBuffer->Add(_adjustedGlyphs[glyphIndexIntoComplexTextController], complexTextRun.GetFont(),
                           paintAdvance,
                           complexTextRun.IndexAt(glyphIndexIntoCurrentRun) + complexTextRun.StringLocation(),
                           FloatPoint(textAutoSpaceSpacing, 0));
        }

        uint32 oldCharacterInCurrentGlyph = _characterInCurrentGlyph;
        _characterInCurrentGlyph =
          std::min(_currentCharacter - complexTextRun.StringLocation(), glyphEndOffset) - glyphStartOffset;
        _runWidthSoFar += adjustedBaseAdvance.Width()
                          * RunWidthSoFarFraction(complexTextRun, glyphStartOffset, glyphEndOffset,
                                                  oldCharacterInCurrentGlyph, iterationStyle);

        if (glyphEndOffset + complexTextRun.StringLocation() > _currentCharacter)
        {
          return;
        }

        _numGlyphsSoFar++;
        _glyphInCurrentRun++;
        _characterInCurrentGlyph = 0;
        if (ltr)
        {
          glyphIndexIntoCurrentRun++;
          glyphIndexIntoComplexTextController++;
        }
        else
        {
          glyphIndexIntoCurrentRun--;
          glyphIndexIntoComplexTextController--;
        }
      }
      currentRunIndex = IncrementCurrentRun(indexOfLeftmostGlyphInCurrentRun);
      _glyphInCurrentRun = 0;
    }
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
      expandLeft = false;
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

  void ComplexTextController::AdjustGlyphsAndAdvances() noexcept
  {
    bool afterExpansion = _run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Forbid;
    size_t runCount = _complexTextRuns.size();
    bool hasExtraSpacing =
      (_fontCascade.LetterSpacing() || _fontCascade.WordSpacing() || _expansion) && !_run.SpacingDisabled();
    bool runForcesLeftExpansion = _run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Force;
    bool runForcesRightExpansion = _run.ExpansionBehavior().Right == ExpansionBehavior::Behavior::Force;
    bool runForbidsLeftExpansion = _run.ExpansionBehavior().Left == ExpansionBehavior::Behavior::Forbid;
    bool runForbidsRightExpansion = _run.ExpansionBehavior().Right == ExpansionBehavior::Behavior::Forbid;
    text_spacing::CharacterClass previousCharacterClass = _textSpacingState.LastCharacterClassFromPreviousRun;

    // We are iterating in glyph order, not string order. Compare this to WidthIterator::AdvanceInternal()
    for (size_t runIndex = 0; runIndex < runCount; ++runIndex)
    {
      ComplexTextRun &complexTextRun = *_complexTextRuns[runIndex];
      uint32 glyphCount = complexTextRun.GlyphCount();
      const Font &font = complexTextRun.GetFont();

      if (!complexTextRun.IsLTR())
      {
        _isLTROnly = false;
      }

      auto glyphs = complexTextRun.Glyphs();
      auto advances = complexTextRun.BaseAdvances();

      // Lower in this function, synthetic bold is blanket-applied to everything, so no need to double-apply
      // it here.
      float spaceWidth = font.SpaceWidth(Font::SyntheticBoldInclusion::Exclude);
      auto charactersSpan = complexTextRun.Characters();
      FloatPoint glyphOrigin;
      uint32 previousCharacterIndex =
        _run.Ltr() ? std::numeric_limits<uint32>::min() : std::numeric_limits<uint32>::max();
      bool isMonotonic = true;

      for (uint32 glyphIndex = 0; glyphIndex < glyphCount; glyphIndex++)
      {
        uint32 characterIndex = complexTextRun.IndexAt(glyphIndex);
        if (_run.Ltr())
        {
          if (characterIndex < previousCharacterIndex)
          {
            isMonotonic = false;
          }
        }
        else
        {
          if (characterIndex > previousCharacterIndex)
          {
            isMonotonic = false;
          }
        }
        auto decoded = CharacterAt(charactersSpan, characterIndex);
        char32 character = decoded.CodePoint;
        bool treatAsSpace = FontCascade::TreatAsSpace(character);
        Glyph glyph = glyphs[glyphIndex];
        FloatSize advance =
          treatAsSpace ? FloatSize(spaceWidth, advances[glyphIndex].Height()) : advances[glyphIndex];

        if (character == text::TabCharacter && _run.AllowTabs())
        {
          advance.SetWidth(_fontCascade.TabWidth(font, _run.TabSize(), _run.XPos() + _totalAdvance.Width(),
                                                 Font::SyntheticBoldInclusion::Exclude));
          // Like simple text path in WidthIterator::ApplyCSSVisibilityRules,
          // make tabCharacter glyph invisible after advancing.
          glyph = DeletedGlyph;
        }
        else if (character == text::ZeroWidthNonJoiner)
        {
          // ZeroWidthNonJoiner is rendered as DeletedGlyph for compatibility with other engines.
          advance.SetWidth(0);
          glyph = DeletedGlyph;
        }
        else if (!treatAsSpace && FontCascade::TreatAsZeroWidthSpace(character))
        {
          advance.SetWidth(0);
          glyph = font.SpaceGlyph();
        }

        // https://www.w3.org/TR/css-text-3/#white-space-processing
        // "Control characters (Unicode category Cc)—other than tabs (U+0009), line feeds (U+000A), carriage
        // returns (U+000D) and sequences that form a segment break—must be rendered as a visible glyph"
        // Also, we're omitting Null (U+0000) from this set because Chrome and Firefox do so and it's needed
        // for compat. See https://github.com/w3c/csswg-drafts/pull/6983.
        if (character != text::NewlineCharacter && character != text::CarriageReturn
            && character != text::NoBreakSpace && character != text::TabCharacter
            && character != text::NullCharacter && IsControlCharacter(character))
        {
          // Let's assume that .notdef is visible.
          glyph = 0;
          advance.SetWidth(font.WidthForGlyph(glyph));
        }

        if (!glyphIndex)
        {
          advance.Expand(complexTextRun.InitialAdvance().Width(), complexTextRun.InitialAdvance().Height());
          if (auto origins = complexTextRun.GlyphOrigins(); !origins.empty())
          {
            advance.Expand(-origins[0].X(), -origins[0].Y());
          }
        }

        advance.Expand(font.SyntheticBoldOffset(), 0);

        if (hasExtraSpacing)
        {
          // If we're a glyph with an advance, add in letter-spacing.
          // That way we weed out zero width lurkers. This behavior matches the fast text code path.
          if (advance.Width())
          {
            advance.Expand(_fontCascade.LetterSpacing(), 0);
          }

          uint32 characterIndexInRun = characterIndex + complexTextRun.StringLocation();
          bool isFirstCharacter = !(characterIndex + complexTextRun.StringLocation());
          bool isLastCharacter = characterIndexInRun + decoded.Length == _run.Length();

          bool forceLeftExpansion = false;  // On the left, regardless of _run.Ltr()
          bool forceRightExpansion = false; // On the right, regardless of _run.Ltr()
          bool forbidLeftExpansion = false;
          bool forbidRightExpansion = false;
          if (runForcesLeftExpansion)
          {
            forceLeftExpansion = _run.Ltr() ? isFirstCharacter : isLastCharacter;
          }
          if (runForcesRightExpansion)
          {
            forceRightExpansion = _run.Ltr() ? isLastCharacter : isFirstCharacter;
          }
          if (runForbidsLeftExpansion)
          {
            forbidLeftExpansion = _run.Ltr() ? isFirstCharacter : isLastCharacter;
          }
          if (runForbidsRightExpansion)
          {
            forbidRightExpansion = _run.Ltr() ? isLastCharacter : isFirstCharacter;
          }
          // Handle justification and word-spacing.
          bool ideograph = FontCascade::CanExpandAroundIdeographsInComplexText()
                           && FontCascade::IsCJKIdeographOrSymbol(character);
          if (treatAsSpace || ideograph || forceLeftExpansion || forceRightExpansion)
          {
            // Distribute the run's total expansion evenly over all expansion opportunities in the run.
            if (_expansion)
            {
              auto [expandLeft, expandRight] =
                ExpansionLocation(ideograph, treatAsSpace, _run.Ltr(), afterExpansion, forbidLeftExpansion,
                                  forbidRightExpansion, forceLeftExpansion, forceRightExpansion);
              if (expandLeft)
              {
                _expansion -= _expansionPerOpportunity;
                // Increase previous width
                if (_adjustedBaseAdvances.empty())
                {
                  advance.Expand(_expansionPerOpportunity, 0);
                  complexTextRun.GrowInitialAdvanceHorizontally(_expansionPerOpportunity);
                }
                else
                {
                  _adjustedBaseAdvances.back().Expand(_expansionPerOpportunity, 0);
                  _totalAdvance.Expand(_expansionPerOpportunity, 0);
                }
              }
              if (expandRight)
              {
                _expansion -= _expansionPerOpportunity;
                advance.Expand(_expansionPerOpportunity, 0);
                afterExpansion = true;
              }
            }
            else
            {
              afterExpansion = false;
            }

            // Account for word-spacing.
            if (treatAsSpace && (character != text::TabCharacter || !_run.AllowTabs())
                && (characterIndex > 0 || runIndex > 0 || character == text::NoBreakSpace)
                && _fontCascade.WordSpacing())
            {
              advance.Expand(_fontCascade.WordSpacing(), 0);
            }
          }
          else
          {
            afterExpansion = false;
          }
        }

        const auto &textAutoSpace = _fontCascade.GetTextAutospace();
        float textAutoSpaceSpacing = 0;
        auto characterClass = text_spacing::CharacterClass::Undefined;
        // Since we are iterating through glyphs here we skip combining marks, since we just care about the
        // grapheme cluster base for text-autospace.
        if (!textAutoSpace.IsNoAutospace() && !IsCombiningMark(character))
        {
          characterClass = text_spacing::GetCharacterClass(character);
          if (textAutoSpace.ShouldApplySpacing(previousCharacterClass, characterClass))
          {
            textAutoSpaceSpacing = complexTextRun.TextAutospaceSize();
            advance.Expand(textAutoSpaceSpacing, 0);
          }
          previousCharacterClass = characterClass;
        }
        if (!textAutoSpace.IsNoAutospace())
        {
          _textAutoSpaceSpacings.push_back(textAutoSpaceSpacing);
        }

        _totalAdvance += advance;

        if (_forTextEmphasis)
        {
          // FIXME: Combining marks should receive a text emphasis mark if they are combine with a space.
          if (!FontCascade::CanReceiveTextEmphasis(character)
              || (U_GET_GC_MASK(static_cast<UChar32>(character)) & U_GC_M_MASK))
          {
            glyph = DeletedGlyph;
          }
        }

        _adjustedBaseAdvances.push_back(advance);
        if (auto origins = complexTextRun.GlyphOrigins(); !origins.empty())
        {
          krys_debug_assert(_glyphOrigins.size() < _adjustedBaseAdvances.size());
          _glyphOrigins.resize(_adjustedBaseAdvances.size());
          _glyphOrigins.back() = origins[glyphIndex] + FloatSize(textAutoSpaceSpacing, 0);
          krys_debug_assert(_glyphOrigins.size() == _adjustedBaseAdvances.size());
        }
        _adjustedGlyphs.push_back(glyph);

        auto glyphBounds = font.BoundsForGlyph(glyph);
        glyphBounds.Move(glyphOrigin.X(), glyphOrigin.Y());
        _minGlyphBoundingBoxX = std::min(_minGlyphBoundingBoxX, glyphBounds.X());
        _maxGlyphBoundingBoxX = std::max(_maxGlyphBoundingBoxX, glyphBounds.MaxX());
        _minGlyphBoundingBoxY = std::min(_minGlyphBoundingBoxY, glyphBounds.Y());
        _maxGlyphBoundingBoxY = std::max(_maxGlyphBoundingBoxY, glyphBounds.MaxY());
        glyphOrigin.Move(advance);

        previousCharacterIndex = characterIndex;
      }
      if (!isMonotonic)
      {
        complexTextRun.SetIsNonMonotonic();
      }
    }
  }

  // Missing glyphs run constructor. Shaping doesn't generate a run of missing glyphs, so we use the primary
  // font's missing glyph in order to match the fast text code path.
  ComplexTextController::ComplexTextRun::ComplexTextRun(const Font &font, utf8_stringview characters,
                                                        uint32 stringLocation, uint32 indexBegin,
                                                        uint32 indexEnd, bool ltr) noexcept
      : _font(ShareRef(font)), _characters(characters), _indexBegin(indexBegin), _indexEnd(indexEnd),
        _stringLocation(stringLocation), _isLTR(ltr),
        _textAutospaceSize(TextAutospace::TextAutospaceSize(font))
  {
    uint32 r = _indexBegin;
    while (r < _indexEnd)
    {
      auto currentIndex = r;
      auto decoded = CharacterAt(_characters, r);
      r += decoded.Length;
      // https://drafts.csswg.org/css-text-3/#white-space-processing
      // "Unsupported Default_ignorable characters must be ignored for text rendering."
      if (!FontCascade::IsCharacterWhoseGlyphsShouldBeDeletedForTextRendering(decoded.CodePoint))
      {
        _coreTextIndices.push_back(currentIndex);
      }
    }
    _glyphCount = static_cast<uint32>(_coreTextIndices.size());
    if (!ltr)
    {
      std::ranges::reverse(_coreTextIndices);
    }

    // Synthesize a run of missing glyphs.
    _glyphs.assign(_glyphCount, 0);
    // Synthetic bold will be handled later in AdjustGlyphsAndAdvances().
    _baseAdvances.assign(_glyphCount,
                         FloatSize(_font->WidthForGlyph(0, Font::SyntheticBoldInclusion::Exclude), 0));
  }

  ComplexTextController::ComplexTextRun::ComplexTextRun(
    Span<const FloatSize> advances, Span<const FloatPoint> origins, Span<const Glyph> glyphs,
    Span<const uint32> stringIndices, FloatSize initialAdvance, const Font &font, utf8_stringview characters,
    uint32 stringLocation, uint32 indexBegin, uint32 indexEnd, bool ltr) noexcept
      : _baseAdvances(advances.begin(), advances.end()), _glyphOrigins(origins.begin(), origins.end()),
        _glyphs(glyphs.begin(), glyphs.end()), _coreTextIndices(stringIndices.begin(), stringIndices.end()),
        _initialAdvance(initialAdvance), _font(ShareRef(font)), _characters(characters),
        _indexBegin(indexBegin), _indexEnd(indexEnd), _glyphCount(static_cast<uint32>(glyphs.size())),
        _stringLocation(stringLocation), _isLTR(ltr),
        _textAutospaceSize(TextAutospace::TextAutospaceSize(font))
  {
  }
}
