#include "Krystal.Booey/Rendering/BreakablePositions.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using LineBreakRules = BreakablePositions::LineBreakRules;
    using WordBreakBehavior = BreakablePositions::WordBreakBehavior;
    using NoBreakSpaceBehavior = BreakablePositions::NoBreakSpaceBehavior;

    template <LineBreakRules TRules = LineBreakRules::Normal,
              WordBreakBehavior TWords = WordBreakBehavior::Normal,
              NoBreakSpaceBehavior TSpaces = NoBreakSpaceBehavior::Normal>
    KRYS_NODISCARD uint32 NextBreak(utf8_stringview text, size_t startPosition) noexcept
    {
      auto factory = CachedLineBreakIteratorFactory {text};
      return BreakablePositions::Next<TRules, TWords, TSpaces>(factory, startPosition);
    }

    /// @brief The break opportunities of @p text, found by asking for the next one after each (the end of
    /// the text only when nothing breaks before it).
    template <LineBreakRules TRules = LineBreakRules::Normal,
              WordBreakBehavior TWords = WordBreakBehavior::Normal>
    KRYS_NODISCARD List<uint32> AllBreaks(utf8_stringview text) noexcept
    {
      List<uint32> breaks;
      auto factory = CachedLineBreakIteratorFactory {text};
      size_t position = 0;
      while (position < text.size())
      {
        auto next = BreakablePositions::Next<TRules, TWords, NoBreakSpaceBehavior::Normal>(factory, position);
        breaks.push_back(next);
        if (next >= text.size())
        {
          break;
        }
        // Continue after the character at the break.
        position = next + text::DecodeUTF8(Span<const char8>(text.data() + next, text.size() - next)).Length;
      }
      return breaks;
    }
  }

  TEST_CASE("BreakablePositions: spaces and the ASCII table", "[Rendering][BreakablePositions]")
  {
    SECTION("Spaces are breakable")
    {
      CHECK(NextBreak(u8"hello world", 0) == 5);
      CHECK(NextBreak(u8"hello world", 5) == 5);
      CHECK(NextBreak(u8"hello world", 6) == 11);
      CHECK(NextBreak(u8"a\tb\nc", 0) == 1);
      CHECK(NextBreak(u8"a\tb\nc", 2) == 3);
    }

    SECTION("No break before the first character without prior context")
    {
      CHECK(NextBreak(u8"", 0) == 0);
      CHECK(NextBreak(u8"x", 0) == 1);
      CHECK(NextBreak(u8"\u65E5", 0) == 3);
    }

    SECTION("A hyphen before a digit is a minus sign unless it follows an alphanumeric")
    {
      CHECK(NextBreak(u8"ABCD-1234", 0) == 5);
      CHECK(NextBreak(u8"1234-5678", 0) == 5);
      CHECK(NextBreak(u8"a -1", 2) == 4);
    }

    SECTION("Hyphens and slashes")
    {
      CHECK(AllBreaks(u8"well-known") == List<uint32> {5, 10});
      CHECK(NextBreak<LineBreakRules::Special>(u8"well-known", 0) == 5);
    }
  }

  TEST_CASE("BreakablePositions: prior context", "[Rendering][BreakablePositions]")
  {
    SECTION("The prior context allows a break before the first character")
    {
      auto factory = CachedLineBreakIteratorFactory {u8"ample"};
      factory.GetPriorContext().Set({u'x', u'-'});
      CHECK(BreakablePositions::Next(factory, 0) == 0);

      auto unbreakable = CachedLineBreakIteratorFactory {u8"ample"};
      unbreakable.GetPriorContext().Set({u'e', u'x'});
      CHECK(BreakablePositions::Next(unbreakable, 0) == 5);
    }

    SECTION("The character before the one before the start comes from the prior context")
    {
      // "A-1": the hyphen follows a letter, so a line can break before the digit.
      auto factory = CachedLineBreakIteratorFactory {u8"-1"};
      factory.GetPriorContext().Set({u' ', u'A'});
      CHECK(BreakablePositions::Next(factory, 1) == 1);

      // " -1": the hyphen is a minus sign.
      auto minus = CachedLineBreakIteratorFactory {u8"-1"};
      minus.GetPriorContext().Set({u'A', u' '});
      CHECK(BreakablePositions::Next(minus, 1) == 2);
    }
  }

  TEST_CASE("BreakablePositions: non-ASCII text", "[Rendering][BreakablePositions]")
  {
    SECTION("Offsets are bytes")
    {
      CHECK(NextBreak(u8"caf\u00E9 au lait", 0) == 5);
      CHECK(NextBreak(u8"caf\u00E9 au lait", 6) == 8);
      CHECK(NextBreak(u8"\u00FCber alles", 0) == 5);
    }

    SECTION("Ideographs break between each other")
    {
      CHECK(AllBreaks(u8"\u65E5\u672C\u8A9E") == List<uint32> {3, 6});
      CHECK(AllBreaks<LineBreakRules::Special>(u8"\u65E5\u672C\u8A9E") == List<uint32> {3, 6});
    }

    SECTION("Keep-all only breaks at spaces")
    {
      CHECK(NextBreak<LineBreakRules::Special, WordBreakBehavior::KeepAll>(u8"\u65E5\u672C \u8A9E", 0) == 6);
      CHECK(NextBreak<LineBreakRules::Special, WordBreakBehavior::KeepAll>(u8"\u65E5\u672C\u3000\u8A9E", 0)
            == 9);
      CHECK(NextBreak<LineBreakRules::Special, WordBreakBehavior::KeepAll>(u8"ab\u200Bcd", 0) == 2);
      CHECK(NextBreak<LineBreakRules::Special, WordBreakBehavior::KeepAll>(u8"\u65E5\u672C\u8A9E", 0) == 9);
    }

    SECTION("Break-all breaks between letters")
    {
      CHECK(AllBreaks<LineBreakRules::Special, WordBreakBehavior::BreakAll>(u8"\u0430\u0431\u0432")
            == List<uint32> {2, 4});
      CHECK(AllBreaks<LineBreakRules::Special>(u8"\u0430\u0431\u0432") == List<uint32> {6});
    }

    SECTION("Supplementary characters go to ICU")
    {
      CHECK(AllBreaks(u8"\U00020000\U00020001") == List<uint32> {4});
      CHECK(AllBreaks(u8"a\U0001F600b") == List<uint32> {1, 5});
    }

    SECTION("No-break spaces")
    {
      CHECK(NextBreak(u8"a\u00A0b", 0) == 4);
      CHECK(NextBreak<LineBreakRules::Normal, WordBreakBehavior::Normal, NoBreakSpaceBehavior::Break>(
              u8"a\u00A0b", 0)
            == 1);
    }
  }

  TEST_CASE("BreakablePositions: IsBreakable", "[Rendering][BreakablePositions]")
  {
    auto factory = CachedLineBreakIteratorFactory {u8"hello world"};

    SECTION("Finds the next breakable position")
    {
      Maybe<uint32> nextBreakable;
      CHECK(BreakablePositions::IsBreakable(factory, 5, nextBreakable, false, true, false, false));
      CHECK_FALSE(BreakablePositions::IsBreakable(factory, 3, nextBreakable, false, true, false, false));
      CHECK_FALSE(BreakablePositions::IsBreakable(factory, 3, nextBreakable, false, false, false, false));
      CHECK(BreakablePositions::IsBreakable(factory, 5, nextBreakable, false, false, true, false));
    }

    SECTION("Uses the known next breakable position")
    {
      Maybe<uint32> nextBreakable = 8;
      CHECK(BreakablePositions::IsBreakable(factory, 8, nextBreakable, false, true, false, false));
      CHECK_FALSE(BreakablePositions::IsBreakable(factory, 5, nextBreakable, false, true, false, false));
    }

    SECTION("Break anywhere")
    {
      Maybe<uint32> nextBreakable;
      CHECK_FALSE(BreakablePositions::IsBreakable(factory, 2, nextBreakable, false, true, false, true));
      // The end of the text is the last character boundary.
      CHECK(BreakablePositions::IsBreakable(factory, 11, nextBreakable, false, true, false, true));
    }
  }
}
