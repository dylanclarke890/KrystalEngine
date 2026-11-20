#include "Krystal.Text/TextBreakIterator.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  namespace
  {
    const StringAtom NoLocale = null;

    KRYS_NODISCARD CachedTextBreakIterator LineBreaks(
      utf8_stringview text, utf8_stringview priorContext = {},
      TextBreakLineBehavior behavior = TextBreakLineBehavior::Default, const StringAtom &locale = NoLocale)
    {
      return CachedTextBreakIterator {text, priorContext, TextBreakIterator::LineMode {behavior}, locale};
    }
  }

  TEST_CASE("TextBreakIterator: line breaks", "[Core][TextBreakIterator]")
  {
    SECTION("Offsets are bytes")
    {
      auto iterator = LineBreaks(u8"Hello world foo");
      CHECK(iterator.Following(0) == 6u);
      CHECK(iterator.Following(6) == 12u);
      CHECK(iterator.Following(12) == 15u);
      CHECK_FALSE(iterator.Following(15));
      CHECK(iterator.Preceding(11) == 6u);
      CHECK_FALSE(iterator.Preceding(0));
      CHECK(iterator.IsBoundary(6));
      CHECK_FALSE(iterator.IsBoundary(3));

      // "héllo " is seven bytes.
      auto accented = LineBreaks(u8"h\u00E9llo w\u00F6rld");
      CHECK(accented.Following(0) == 7u);
      CHECK(accented.Preceding(12) == 7u);
    }

    SECTION("The prior context decides whether the text starts with a break opportunity")
    {
      CHECK_FALSE(LineBreaks(u8"abc", u8"x").IsBoundary(0));
      CHECK(LineBreaks(u8"abc", u8"x ").IsBoundary(0));
      // A prior context isn't part of the text.
      auto iterator = LineBreaks(u8"ab cd", u8"\u00E9 ");
      CHECK(iterator.Following(0) == 3u);
      CHECK(iterator.Preceding(4) == 3u);
      CHECK(iterator.Preceding(1).value_or(0) == 0u);
    }

    SECTION("Strictness")
    {
      // A small kana can only start a line under loose or normal line breaking.
      auto text = u8"\u3042\u3041";
      const StringAtom japanese {u8"ja"};
      CHECK_FALSE(LineBreaks(text, {}, TextBreakLineBehavior::Strict, japanese).IsBoundary(3));
      CHECK(LineBreaks(text, {}, TextBreakLineBehavior::Loose, japanese).IsBoundary(3));
    }

    SECTION("Moved iterators keep their text")
    {
      auto iterator = LineBreaks(u8"ab cd", u8"x ");
      auto moved = krys::move(iterator);
      CHECK(moved.Following(0) == 3u);

      auto other = LineBreaks(u8"efgh ij");
      other = krys::move(moved);
      CHECK(other.Following(0) == 3u);
    }

    SECTION("Released iterators are reused for new text")
    {
      {
        auto first = LineBreaks(u8"one two", u8"q ");
        CHECK(first.Following(0) == 4u);
      }
      auto second = LineBreaks(u8"three four");
      CHECK(second.Following(0) == 6u);
      CHECK(second.IsBoundary(0));
    }
  }

  TEST_CASE("TextBreakIterator: grapheme clusters", "[Core][TextBreakIterator]")
  {
    SECTION("Caret and character modes")
    {
      // e + COMBINING ACUTE ACCENT (two bytes) is one cluster.
      utf8_stringview text = u8"e\u0301x";
      CachedTextBreakIterator character {text, {}, TextBreakIterator::CharacterMode {}, NoLocale};
      CHECK(character.Following(0) == 3u);
      CHECK_FALSE(character.IsBoundary(1));
      CachedTextBreakIterator caret {text, {}, TextBreakIterator::CaretMode {}, NoLocale};
      CHECK(caret.Preceding(4) == 3u);
    }

    SECTION("Counting clusters")
    {
      CHECK(NumGraphemeClusters(u8"") == 0u);
      CHECK(NumGraphemeClusters(u8"abc") == 3u);
      CHECK(NumGraphemeClusters(u8"a\r\nb") == 3u);
      CHECK(NumGraphemeClusters(u8"e\u0301") == 1u);
      // A flag is a pair of regional indicators.
      CHECK(NumGraphemeClusters(u8"\U0001F1EB\U0001F1F7!") == 2u);

      CHECK(NumCodeUnitsInGraphemeClusters(u8"a\r\nbc", 2) == 3u);
      CHECK(NumCodeUnitsInGraphemeClusters(u8"abc", 5) == 3u);
      CHECK(NumCodeUnitsInGraphemeClusters(u8"ae\u0301b", 2) == 4u);
      CHECK(NumCodeUnitsInGraphemeClusters(u8"ae\u0301b", 9) == 5u);
    }

    SECTION("Non-shared iterators")
    {
      NonSharedCharacterBreakIterator iterator {u8"\u00E9a"};
      REQUIRE(iterator);
      CHECK(iterator.Next() == 2u);
      CHECK(iterator.Next() == 3u);
      CHECK_FALSE(iterator.Next());

      NonSharedSentenceBreakIterator sentences {u8"One. Two."};
      REQUIRE(sentences);
      CHECK(sentences.Next() == 5u);
    }
  }

  TEST_CASE("TextBreakIterator: words", "[Core][TextBreakIterator]")
  {
    auto iterator = WordBreakIterator(u8"hello, world");
    REQUIRE(iterator);
    CHECK(iterator.First() == 0u);
    CHECK(iterator.Next() == 5u);
    CHECK(iterator.IsWordBreak());
    CHECK(iterator.Next() == 6u);
    CHECK_FALSE(iterator.IsWordBreak());
    CHECK(iterator.Current() == 6u);
    CHECK(iterator.IsBoundary(7));
    CHECK_FALSE(iterator.IsBoundary(9));
    CHECK(iterator.Following(7) == 12u);
    CHECK(iterator.Preceding(9) == 7u);
    CHECK(iterator.Last() == 12u);
    CHECK(iterator.Previous() == 7u);
    CHECK_FALSE(iterator.Following(12));

    auto sentences = SentenceBreakIterator(u8"A b. C d.");
    REQUIRE(sentences);
    CHECK(sentences.Next() == 5u);
  }

  TEST_CASE("CachedLineBreakIteratorFactory", "[Core][TextBreakIterator]")
  {
    SECTION("Prior context")
    {
      CachedLineBreakIteratorFactory::PriorContext context;
      CHECK(context.GetLength() == 0u);
      CHECK(context.Characters().empty());

      context.Update(U'a');
      CHECK(context.GetLength() == 1u);
      CHECK(context.LastCharacter() == U'a');
      CHECK(context.Characters() == u8"a");

      context.Update(U'\u00E9');
      CHECK(context.GetLength() == 2u);
      CHECK(context.SecondToLastCharacter() == U'a');
      CHECK(context.Characters() == u8"a\u00E9");

      context.Set({U'\U0001F600', U' '});
      CHECK(context.Characters() == u8"\U0001F600 ");

      context.Reset();
      CHECK(context.GetLength() == 0u);
      CHECK(context == CachedLineBreakIteratorFactory::PriorContext {});
    }

    SECTION("Iterators follow the prior context")
    {
      CachedLineBreakIteratorFactory factory {u8"abc def"};
      CHECK(factory.StringView() == u8"abc def");
      CHECK(factory.Mode() == TextBreakLineBehavior::Default);
      // Without a prior context, the text starts at a boundary.
      CHECK(factory.Get().IsBoundary(0));

      factory.GetPriorContext().Set({0, U'x'});
      CHECK_FALSE(factory.Get().IsBoundary(0));

      factory.GetPriorContext().Set({U'x', U' '});
      CHECK(factory.Get().IsBoundary(0));
      CHECK(factory.Get().Following(0) == 4u);

      factory.ResetStringAndReleaseIterator(u8"gh ij", null, TextBreakLineBehavior::Strict);
      CHECK(factory.Mode() == TextBreakLineBehavior::Strict);
      CHECK(factory.Get().Following(0) == 3u);
    }
  }
}
