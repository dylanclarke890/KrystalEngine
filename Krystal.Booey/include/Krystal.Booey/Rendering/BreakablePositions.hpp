#pragma once

#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <unicode/ubrk.h>

namespace krys::boo
{
  /// @brief Finds line break opportunities, with fast paths for common characters in front of ICU's line
  /// break iterator. Positions are byte offsets into UTF-8 text.
  class BreakablePositions
  {
  public:
    enum class NoBreakSpaceBehavior : uint8
    {
      Normal,
      Break,
    };
    enum class WordBreakBehavior : uint8
    {
      Normal,
      BreakAll,
      KeepAll,
      AutoPhrase,
    };
    enum class LineBreakRules : uint8
    {
      Normal,  // Fast path available when using default line-breaking rules within ASCII.
      Special, // Uses ICU to handle special line-breaking rules.
    };

    BreakablePositions() = delete;

    template <LineBreakRules TRules, WordBreakBehavior TWords, NoBreakSpaceBehavior TSpaces>
    KRYS_NODISCARD static inline uint32 Next(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                             size_t startPosition) noexcept;

    KRYS_NODISCARD static inline uint32 Next(CachedLineBreakIteratorFactory &iterator,
                                             size_t startPosition) noexcept
    {
      return Next<LineBreakRules::Normal, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Normal>(
        iterator, startPosition);
    }

    KRYS_NODISCARD static inline bool IsBreakable(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                                  uint32 startPosition, Maybe<uint32> &nextBreakable,
                                                  bool breakNBSP, bool canUseShortcut, bool keepAllWords,
                                                  bool breakAnywhere) noexcept;

  private:
    // Iterator implementations.
    template <LineBreakRules TRules, WordBreakBehavior TWords, NoBreakSpaceBehavior TSpaces>
    KRYS_NODISCARD static inline size_t NextBreakablePosition(CachedLineBreakIteratorFactory &factory,
                                                              utf8_stringview string,
                                                              size_t startPosition) noexcept;

    template <NoBreakSpaceBehavior TSpaces>
    KRYS_NODISCARD static inline size_t NextBreakableSpace(utf8_stringview string,
                                                           size_t startPosition) noexcept;

    KRYS_NODISCARD static inline uint32
      NextCharacter(CachedLineBreakIteratorFactory &lineBreakIteratorFactory, uint32 startPosition) noexcept;

    // Helper functions.
    enum BreakClass : uint16;
    template <LineBreakRules TRules, NoBreakSpaceBehavior TSpaces>
    KRYS_NODISCARD static inline BreakClass Classify(char32 character) noexcept;

    template <NoBreakSpaceBehavior TSpaces>
    KRYS_NODISCARD static inline bool IsBreakableSpace(char32 character) noexcept;

    KRYS_NODISCARD static size_t CodePointLengthAt(utf8_stringview string, size_t position) noexcept
    {
      return text::DecodeUTF8(Span<const char8>(string.data() + position, string.size() - position)).Length;
    }

    KRYS_NODISCARD static char32 CodePointAt(utf8_stringview string, size_t position) noexcept
    {
      return text::DecodeUTF8(Span<const char8>(string.data() + position, string.size() - position))
        .CodePoint;
    }

    /// @brief The byte offset of the code point that ends at @p position.
    KRYS_NODISCARD static size_t PreviousCodePointStart(utf8_stringview string, size_t position) noexcept
    {
      krys_debug_assert(position);
      do
      {
        --position;
      } while (position && text::IsTrailingUTF8(string[position]));
      return position;
    }

    /// @brief The byte offset of the last code point of @p string.
    KRYS_NODISCARD static size_t LastCodePointStart(utf8_stringview string) noexcept
    {
      return string.empty() ? 0 : PreviousCodePointStart(string, string.size());
    }

    // Data types.
    enum BreakClass : uint16
    {
      // See UAX14 and LineBreak.txt
      // https://www.unicode.org/Public/UCD/latest/ucd/LineBreak.txt
      Indeterminate = 0,
      AL = 1,
      ID = 1 << 1,
      CM = 1 << 2,
      OP = 1 << 3,
      CP = 1 << 4,
      CL = 1 << 5,
      GL = 1 << 6,
      QU = 1 << 7,
      SP = 1 << 8,
      NU = AL,
      Weird = 1 << 15,
      // Currently we map
      //     1. HL to AL
      //     2. H2 and H3 to ID
      // We also don't distinguish AL and NU.
      // If we pull more logic into isBreakable, these may need to be distinguished.
    };

    class LineBreakTable
    {
    public:
      static constexpr char32 FirstCharacter = '!';
      static constexpr char32 LastCharacter = 0xFF;
      static constexpr uint32 RowCount = LastCharacter - FirstCharacter + 1;
      static constexpr uint32 ColumnCount = (LastCharacter - FirstCharacter) / 8 + 1;

      // Must range check before calling.
      KRYS_NODISCARD static inline bool UnsafeLookup(char32 before, char32 after) noexcept
      {
        const uint32 beforeIndex = before - FirstCharacter;
        const uint32 afterIndex = after - FirstCharacter;
        return BreakTable[beforeIndex][afterIndex / 8] & (1 << (afterIndex % 8));
      }

    private:
      static const Array<Array<uint8, ColumnCount>, RowCount> BreakTable;
    };
  };

  template <BreakablePositions::NoBreakSpaceBehavior TSpaces>
  inline bool BreakablePositions::IsBreakableSpace(char32 character) noexcept
  {
    switch (character)
    {
      case ' ':
      case '\n':
      case '\t':               return true;
      case text::NoBreakSpace: return TSpaces == NoBreakSpaceBehavior::Break;
      default:                 return false;
    }
  }

  template <BreakablePositions::LineBreakRules TRules, BreakablePositions::WordBreakBehavior TWords,
            BreakablePositions::NoBreakSpaceBehavior TSpaces>
  inline size_t BreakablePositions::NextBreakablePosition(CachedLineBreakIteratorFactory &factory,
                                                          utf8_stringview string,
                                                          size_t startPosition) noexcept
  {
    struct CharacterInfo
    {
      char32 Character {0};
      BreakClass Type {Indeterminate};
    };

    // Don't break if positioned at start of primary context and there is no prior context.
    auto &priorContext = factory.GetPriorContext();
    if (!startPosition && !priorContext.GetLength())
    {
      if (string.empty())
      {
        return 0;
      }
      startPosition = CodePointLengthAt(string, 0);
      if (startPosition == string.size())
      {
        return string.size();
      }
    }

    CharacterInfo beforeBefore;
    CharacterInfo before;
    if (startPosition)
    {
      auto beforeStart = PreviousCodePointStart(string, startPosition);
      before.Character = CodePointAt(string, beforeStart);
      beforeBefore.Character = beforeStart ? CodePointAt(string, PreviousCodePointStart(string, beforeStart))
                                           : priorContext.LastCharacter();
    }
    else
    {
      before.Character = priorContext.LastCharacter();
      beforeBefore.Character = priorContext.SecondToLastCharacter();
    }
    CharacterInfo after;

    Maybe<size_t> nextBreak;
    for (size_t i = startPosition; i < string.size();
         beforeBefore = before, before = after, i += CodePointLengthAt(string, i))
    {
      after = {CodePointAt(string, i)};

      // Breakable spaces.
      if (IsBreakableSpace<TSpaces>(after.Character))
      {
        return i;
      }

      // ASCII rapid lookup.
      if constexpr (TRules == LineBreakRules::Normal)
      { // Not valid for 'loose' line-breaking.
        // Don't allow line breaking between '-' and a digit if the '-' may mean a minus sign in the context,
        // while allow breaking in 'ABCD-1234' and '1234-5678' which may be in long URLs.
        if (before.Character == '-' && text::IsASCIIDigit(after.Character))
        {
          if (text::IsASCIIAlphanumeric(beforeBefore.Character))
          {
            return i;
          }
          continue;
        }

        // If both characters are ASCII, use a lookup table for enhanced speed
        // and for compatibility with other browsers (see comments on lineBreakTable for details).
        if (before.Character <= LineBreakTable::LastCharacter
            && after.Character <= LineBreakTable::LastCharacter)
        {
          if (before.Character >= LineBreakTable::FirstCharacter
              && after.Character >= LineBreakTable::FirstCharacter)
          {
            if (LineBreakTable::UnsafeLookup(before.Character, after.Character))
            {
              return i;
            }
          } // Else at least one is an ASCII control character; don't break.
          continue;
        }
      }

      // Non-ASCII rapid lookup.
      if constexpr (TWords != WordBreakBehavior::AutoPhrase)
      {
        if (!before.Type)
        {
          before.Type = Classify<TRules, TSpaces>(before.Character);
        }
        after.Type = Classify<TRules, TSpaces>(after.Character);
        // Short-circuit the commonest cases: letter + letter.
        uint32 pair = before.Type | after.Type;
        // AL+AL SP+AL SP+QU AL+QU QU+QU QU+AL (after's SP is already filtered out).
        if (!(pair & ~(SP | AL | QU)))
        {
          if constexpr (TWords == WordBreakBehavior::BreakAll)
          {
            if (pair == AL)
            {
              return i;
            }
          }
          continue;
        }
        if ((pair | AL) == (ID | AL))
        {
          if constexpr (TWords == WordBreakBehavior::KeepAll)
          {
            continue;
          }
          return i;
        }
        // Handle special cases.
        if (pair & (GL | QU) && !(pair & Weird))
        { // Keep nbsp high in our list.
          continue;
        }
        if (after.Type == CM)
        {
          after.Type = before.Type;
          continue;
        }
        if constexpr (TRules == LineBreakRules::Normal)
        {
          if (!(pair & Weird))
          {
            // Handle some common and obvious punctuation behaviors.
            if (pair & (CL | CP | OP))
            {
              if (after.Type == CL || after.Type == CP || before.Type == OP)
              {
                continue;
              }
              if (pair & ID)
              {
                return i;
              }
            }
          }
        }
      }

      // ICU lookup (slow).
      if (!nextBreak || *nextBreak < i)
      {
        // At the start of the string, i - 1 wraps around and the iterator lands in the prior context.
        nextBreak = factory.Get().Following(static_cast<uint32>(i - 1));
      }
      // Fast forward while our behavior matches ICU.
      if (nextBreak && i < *nextBreak)
      {
        for (size_t max = std::min(*nextBreak, LastCodePointStart(string)); i < max;
             beforeBefore = before, before = after)
        {
          auto lookaheadPosition = i + CodePointLengthAt(string, i);
          char32 lookahead = CodePointAt(string, lookaheadPosition);
          if ((lookahead <= LineBreakTable::LastCharacter && !text::IsASCIIAlpha(lookahead))
              || (TSpaces == NoBreakSpaceBehavior::Break && lookahead == text::NoBreakSpace))
          {
            break;
          }
          i = lookaheadPosition;
        }
      }
      if (i == nextBreak && !IsBreakableSpace<TSpaces>(before.Character))
      {
        return i;
      }
    }

    return string.size();
  }

  template <BreakablePositions::NoBreakSpaceBehavior TSpaces>
  inline size_t BreakablePositions::NextBreakableSpace(utf8_stringview string, size_t startPosition) noexcept
  {
    // FIXME: Use ICU instead.
    for (size_t i = startPosition; i < string.size(); i += CodePointLengthAt(string, i))
    {
      auto character = CodePointAt(string, i);
      if (IsBreakableSpace<TSpaces>(character))
      {
        return i;
      }
      // FIXME: This should either be in isBreakableSpace (though previous attempts broke the world) or should
      // use ICU instead.
      if (character == text::ZeroWidthSpace)
      {
        return i;
      }
      if (character == text::IdeographicSpace)
      {
        return i + CodePointLengthAt(string, i);
      }
    }
    return string.size();
  }

  inline uint32 BreakablePositions::NextCharacter(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                                  uint32 startPosition) noexcept
  {
    auto stringView = lineBreakIteratorFactory.StringView();
    krys_debug_assert(startPosition <= stringView.length());
    // FIXME: Can/Should we implement this using a Shared Iterator (performance issue)
    // https://bugs.webkit.org/show_bug.cgi?id=197876
    NonSharedCharacterBreakIterator iterator(stringView);
    auto next = ubrk_following(iterator, static_cast<int32_t>(startPosition));
    if (next == UBRK_DONE)
    {
      return static_cast<uint32>(stringView.length());
    }
    return static_cast<uint32>(next);
  }

  template <BreakablePositions::LineBreakRules TRules, BreakablePositions::WordBreakBehavior TWords,
            BreakablePositions::NoBreakSpaceBehavior TSpaces>
  inline uint32 BreakablePositions::Next(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                         size_t startPosition) noexcept
  {
    auto stringView = lineBreakIteratorFactory.StringView();
    if constexpr (TWords == WordBreakBehavior::KeepAll)
    {
      return static_cast<uint32>(NextBreakableSpace<TSpaces>(stringView, startPosition));
    }
    else
    {
      return static_cast<uint32>(
        NextBreakablePosition<TRules, TWords, TSpaces>(lineBreakIteratorFactory, stringView, startPosition));
    }
  }

  inline bool BreakablePositions::IsBreakable(CachedLineBreakIteratorFactory &lineBreakIteratorFactory,
                                              uint32 startPosition, Maybe<uint32> &nextBreakable,
                                              bool breakNBSP, bool canUseShortcut, bool keepAllWords,
                                              bool breakAnywhere) noexcept
  {
    if (nextBreakable && *nextBreakable >= startPosition)
    {
      return startPosition == *nextBreakable;
    }

    if (breakAnywhere)
    {
      return startPosition == NextCharacter(lineBreakIteratorFactory, startPosition);
    }

    if (keepAllWords)
    {
      if (breakNBSP)
      {
        return startPosition
               == Next<LineBreakRules::Special, WordBreakBehavior::KeepAll, NoBreakSpaceBehavior::Break>(
                 lineBreakIteratorFactory, startPosition);
      }
      return startPosition
             == Next<LineBreakRules::Special, WordBreakBehavior::KeepAll, NoBreakSpaceBehavior::Normal>(
               lineBreakIteratorFactory, startPosition);
    }

    if (canUseShortcut)
    {
      if (breakNBSP)
      {
        return startPosition
               == Next<LineBreakRules::Normal, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Break>(
                 lineBreakIteratorFactory, startPosition);
      }
      return startPosition
             == Next<LineBreakRules::Normal, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Normal>(
               lineBreakIteratorFactory, startPosition);
    }

    if (breakNBSP)
    {
      return startPosition
             == Next<LineBreakRules::Special, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Break>(
               lineBreakIteratorFactory, startPosition);
    }

    return startPosition
           == Next<LineBreakRules::Special, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Normal>(
             lineBreakIteratorFactory, startPosition);
  }

  template <BreakablePositions::LineBreakRules TRules, BreakablePositions::NoBreakSpaceBehavior TSpaces>
  inline BreakablePositions::BreakClass BreakablePositions::Classify(char32 character) noexcept
  {
    // This function is optimized for letters and NBSP.
    // See LineBreak.txt for classification.

    static constexpr char32 blockLast3 = ~char32 {0x07};
    static constexpr char32 blockLast4 = ~char32 {0x0F};
    static constexpr char32 blockLast6 = ~char32 {0x3F};
    static constexpr char32 blockLast7 = ~char32 {0x7F};
    static constexpr char32 blockLast8 = ~char32 {0xFF};

    switch ((character & blockLast7) / 0x80)
    {
        // Compilers are not smart enough to make a jump table if the step is not 1.
      case 0x0000 / 0x80: // ASCII
        switch ((character & blockLast4) / 0x10)
        {
          case 0x0000 / 0x10: return Weird;
          case 0x0010 / 0x10: return CM;
          case 0x0020 / 0x10:
            if (character == 0x0020)
            { // space
              return SP;
            }
            if (character == 0x0022 || character == 0x0027)
            {
              return QU;
            }
            if (character == 0x0028)
            {
              return OP;
            }
            if (character == 0x0029)
            {
              return CP;
            }
            return Weird;
          case 0x0030 / 0x10:
            if (character <= 0x0039)
            { // Numbers.
              return NU;
            }
            return Weird;
          case 0x0040 / 0x10: return AL;
          case 0x0050 / 0x10:
            if (character <= 0x005A)
            {
              return AL;
            }
            if (character == 0x005B)
            {
              return OP;
            }
            if (character == 0x005D)
            {
              return CP;
            }
            return Weird;
          case 0x0060 / 0x10: return AL;
          case 0x0070 / 0x10:
            if (character <= 0x007A)
            {
              return AL;
            }
            if (character == 0x007B)
            {
              return OP;
            }
            if (character == 0x007D)
            {
              return CL;
            }
            return Weird;
          default: return Weird;
        }
      case 0x0080 / 0x80: // Latin-1
        if (character == 0x00A0)
        { // noBreakSpace
          return TSpaces == NoBreakSpaceBehavior::Normal ? GL : SP;
        }
        if (character > 0x00C0)
        {
          return AL;
        }
        if (character == 0x00A1 || character == 0x00BF)
        {
          return OP;
        }
        if (character == 0x00AB || character == 0x00BB)
        {
          return QU;
        }
        return Weird;
      case 0x0100 / 0x80:
      case 0x0180 / 0x80:
      case 0x0200 / 0x80: return AL;
      case 0x0280 / 0x80:
        switch (character)
        {
          case 0x02C8:
          case 0x02CC:
          case 0x02DF: return Weird;
          default:     return AL;
        }
      case 0x0300 / 0x80:
        if (character == 0x034F || (0x035C <= character && character <= 0x0362))
        {
          return GL;
        }
        if (character < 0x370)
        {
          return CM;
        }
        if (character == 0x037E) KRYS_UNLIKELY
        {
          return Weird;
        }
        return AL;
      case 0x0380 / 0x80:
      case 0x0400 / 0x80: return AL;
      case 0x0480 / 0x80:
        if (0x0483 <= character && character <= 0x0489)
        {
          return CM;
        }
        return AL;
      case 0x0500 / 0x80: return AL;
      case 0x0580 / 0x80:
        if (character <= 0x0588 || 0x05C8 <= character)
        {
          return AL; // WARNING: Some of these are actually HL.
        }
        if (0x0591 <= character && character <= 0x05BD)
        {
          return CM;
        }
        // 0x05BE to 0x05C7 is mixed up.
        switch (character)
        {
          case 0x05BF:
          case 0x05C1:
          case 0x05C2:
          case 0x05C4:
          case 0x05C5:
          case 0x05C7: return CM;
          default:     return Weird;
        }
      case 0x0600 / 0x80:
      case 0x0680 / 0x80:
      case 0x0700 / 0x80:
      case 0x0780 / 0x80:
      case 0x0800 / 0x80:
      case 0x0880 / 0x80:
      case 0x0900 / 0x80:
      case 0x0980 / 0x80:
      case 0x1000 / 0x80:
      case 0x1080 / 0x80:
      case 0x1100 / 0x80:
      case 0x1180 / 0x80:
      case 0x1200 / 0x80:
      case 0x1280 / 0x80:
      case 0x1300 / 0x80:
      case 0x1380 / 0x80:
      case 0x1400 / 0x80:
      case 0x1480 / 0x80:
      case 0x1500 / 0x80:
      case 0x1580 / 0x80:
      case 0x1600 / 0x80:
      case 0x1680 / 0x80:
      case 0x1700 / 0x80:
      case 0x1780 / 0x80:
      case 0x1800 / 0x80:
      case 0x1880 / 0x80:
      case 0x1900 / 0x80:
      case 0x1980 / 0x80: return Weird;
      case 0x2000 / 0x80:
        if (character == 0x2018 || character == 0x2019)
        {
          return QU;
        }
        return Weird;
        // FIXME: Continue bitmask switch up to 2E80.
    }

    // CJK
    if (0x2E80 <= character && character <= 0xA4CF)
    {
      if ((character & blockLast8) == 0x3000)
      {
        if (character <= 0x303F)
        {
          // This block (0x3000-0x303F) is categorically very mixed up.
          switch (character & 0x1F)
          {
            case 0x3001 & 0x1F:
            case 0x3002 & 0x1F:
            case 0x3009 & 0x1F:
            case 0x300B & 0x1F:
            case 0x300D & 0x1F:
            case 0x300F & 0x1F:
            case 0x3011 & 0x1F:
            case 0x3015 & 0x1F:
            case 0x3017 & 0x1F:
            case 0x3019 & 0x1F:
            case 0x301B & 0x1F:
            case 0x301E & 0x1F:
            case 0x301F & 0x1F: return CL;
            case 0x3008 & 0x1F:
            case 0x300A & 0x1F:
            case 0x300C & 0x1F:
            case 0x300E & 0x1F:
            case 0x3010 & 0x1F:
            case 0x3016 & 0x1F:
            case 0x3014 & 0x1F:
            case 0x3018 & 0x1F:
            case 0x301A & 0x1F:
            case 0x301D & 0x1F: return OP;
            default:            return Weird;
          }
        }
        // This block (0x3040-0x30FF) is tangled up and sensitive to 'line-break'.
        return Weird;
      }
      if ((character & blockLast4) == 0x31F0) KRYS_UNLIKELY
      { // Small Kana.
        return Weird;
      }
      if ((character & blockLast3) == 0x3248) KRYS_UNLIKELY
      {
        return AL;
      }
      if ((character & blockLast6) == 0x4DC0) KRYS_UNLIKELY
      {
        return AL;
      }
      if (character == 0xA015) KRYS_UNLIKELY
      {
        return Weird;
      }
      return ID;
    }
    // Precomposed Hangul
    if (0xAC00 <= character && character <= 0xD7AF)
    {
      return ID; // WARNING: These are actually H2 or H3.
    }
    // More CJK
    if (0xF900 <= character && character <= 0XFAFF)
    {
      return ID;
    }

    return Weird;
  }
}
