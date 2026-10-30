#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Layout/LayoutTestUtils.hpp"
#include "Krystal.Booey/Graphics/Font.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineTextItem.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::tests
{
  using layout::TextUtil;
  using layout::tests::RegisterAhem;
  using layout::tests::RootBoxOf;
  using style::tests::ResolverFixture;

  namespace
  {
    /// @brief The first text box of the block with id @p id.
    KRYS_NODISCARD const layout::InlineTextBox &TextBoxOf(ResolverFixture &fixture,
                                                          dom::DOMStringView id) noexcept
    {
      auto &root = RootBoxOf(fixture, id);
      const layout::InlineTextBox *textBox = nullptr;
      for (auto *child = root.FirstChild(); child && !textBox; child = child->NextSibling())
      {
        textBox = DynamicDowncast<layout::InlineTextBox>(*child);
      }
      REQUIRE(textBox);
      return *textBox;
    }

    /// @brief The inline items of the block with id @p id.
    struct Items
    {
      layout::InlineContentCache Cache;

      Items(ResolverFixture &fixture, dom::DOMStringView id) noexcept
      {
        layout::InlineItemsBuilder(Cache, RootBoxOf(fixture, id)).Build({});
      }

      KRYS_NODISCARD const layout::InlineTextItem &operator[](size_t index) const noexcept
      {
        auto &items = Cache.GetInlineItems().Content();
        REQUIRE(index < items.size());
        return Downcast<layout::InlineTextItem>(items[index]);
      }
    };
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TextUtil: widths", "[Layout][Inline][TextUtil]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #pre { white-space: pre }
    </style><body>
    <div id=text>ab  cd</div>
    <div id=pre>ab  cd</div>
    <div id=word>abcdefghij</div>
    <div id=hebrew>aש</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("Ranges of a text box")
    {
      auto &box = TextBoxOf(fixture, u8"pre");
      auto &font = box.Style().GetFontCascade();
      CHECK(TextUtil::Width(box, font, 0, 0, 0) == 0.f);
      CHECK(TextUtil::Width(box, font, 0, 2, 0) == 20.f);
      CHECK(TextUtil::Width(box, font, 0, 6, 0) == 60.f);
      CHECK(TextUtil::Width(box, font, 0, 2, 0, TextUtil::UseTrailingWhitespaceMeasuringOptimization::No)
            == 20.f);
      CHECK(TextUtil::TrailingWhitespaceWidth(box, font, 0, 3) == 10.f);
      CHECK(TextUtil::TrailingWhitespaceWidth(box, font, 0, 4) == 10.f);
    }

    SECTION("Collapsible white space is one space wide; preserved white space is measured")
    {
      Items collapsed {fixture, u8"text"};
      CHECK(collapsed[1].IsWhitespace());
      CHECK(collapsed[1].Length() == 2);
      CHECK(TextUtil::Width(collapsed[1], collapsed[1].Style().GetFontCascade(), 0) == 10.f);

      Items preserved {fixture, u8"pre"};
      CHECK(TextUtil::Width(preserved[1], preserved[1].Style().GetFontCascade(), 0) == 20.f);
      CHECK(TextUtil::Width(preserved[0], preserved[0].Style().GetFontCascade(), 0) == 20.f);
    }

    SECTION("Breaking a word")
    {
      Items items {fixture, u8"word"};
      auto &item = items[0];
      auto &font = item.Style().GetFontCascade();
      auto check = [&](float availableWidth, size_t length, float width)
      {
        auto left = TextUtil::BreakWord(item, font, 100, availableWidth, 0);
        CHECK(left.Length == length);
        CHECK(left.LogicalWidth == width);
      };
      check(35, 3, 30);
      check(40, 4, 40);
      check(5, 0, 0);
      check(99, 9, 90);
    }

    SECTION("Fallback fonts")
    {
      auto &style = TextBoxOf(fixture, u8"text").Style();
      CHECK(TextUtil::FallbackFontsForText(u8"ab", style, TextUtil::IncludeHyphen::No).empty());
      CHECK(TextUtil::FallbackFontsForText(u8"ab", style, TextUtil::IncludeHyphen::Yes).empty());
      auto fallbackFonts = TextUtil::FallbackFontsForText(u8"a\u05E9", style, TextUtil::IncludeHyphen::No);
      REQUIRE(fallbackFonts.size() == 1);
      CHECK(fallbackFonts.begin()->get() != style.GetFontCascade().PrimaryFont().get());
    }

    SECTION("Glyph bounds")
    {
      auto &style = TextBoxOf(fixture, u8"text").Style();
      // Ahem's glyphs fill the ascent (0.8em) and descent (0.2em) exactly.
      auto simple = TextUtil::EnclosingGlyphBoundsForText(
        u8"ab", style, TextUtil::ShouldUseSimpleGlyphOverflowCodePath::Yes);
      CHECK(simple.Ascent == -8.f);
      CHECK(simple.Descent == 2.f);
      auto complex = TextUtil::EnclosingGlyphBoundsForText(
        u8"ab", style, TextUtil::ShouldUseSimpleGlyphOverflowCodePath::No);
      CHECK(complex.Ascent == -8.f);
      CHECK(complex.Descent == 2.f);
      auto empty = TextUtil::EnclosingGlyphBoundsForText(u8"", style,
                                                         TextUtil::ShouldUseSimpleGlyphOverflowCodePath::Yes);
      CHECK(empty.Ascent == 0.f);
      CHECK(empty.Descent == 0.f);
    }

    SECTION("Hyphens")
    {
      auto &style = TextBoxOf(fixture, u8"text").Style();
      CHECK(TextUtil::HyphenWidth(style) == style.GetFontCascade().Width(style.HyphenString().View()));
      CHECK(TextUtil::HyphenWidth(style) > 0.f);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TextUtil: breaking", "[Layout][Inline][TextUtil]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #none { hyphens: none }
      #keepAll { word-break: keep-all }
      #strict { line-break: strict }
    </style><body>
    <div id=normal>x</div>
    <div id=none>x</div>
    <div id=keepAll>x</div>
    <div id=strict>x</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    auto &normal = TextBoxOf(fixture, u8"normal").Style();
    auto &none = TextBoxOf(fixture, u8"none").Style();

    SECTION("Breaks between adjacent content")
    {
      CHECK(TextUtil::MayBreakInBetween(u8"ex-", normal, u8"ample", normal));
      CHECK_FALSE(TextUtil::MayBreakInBetween(u8"ab", normal, u8"cd", normal));
      CHECK(TextUtil::MayBreakInBetween(u8"\u65E5", normal, u8"\u672C", normal));
      CHECK_FALSE(TextUtil::MayBreakInBetween(u8"", normal, u8"ab", normal));
      // A trailing soft hyphen only breaks when hyphenation is allowed.
      CHECK(TextUtil::MayBreakInBetween(u8"ex\u00AD", normal, u8"ample", normal));
      CHECK_FALSE(TextUtil::MayBreakInBetween(u8"ex\u00AD", none, u8"ample", normal));
    }

    SECTION("The next breakable position follows the style")
    {
      auto next = [](utf8_stringview text, const RenderStyle &style)
      {
        auto factory = CachedLineBreakIteratorFactory {text, style.ComputedLocale().GetPlatform(),
                                                       TextUtil::LineBreakIteratorMode(style.GetLineBreak()),
                                                       TextUtil::GetContentAnalysis(style.GetWordBreak())};
        return TextUtil::FindNextBreakablePosition(factory, 0, style);
      };
      CHECK(next(u8"\u65E5\u672C \u8A9E", normal) == 3);
      CHECK(next(u8"\u65E5\u672C \u8A9E", TextBoxOf(fixture, u8"keepAll").Style()) == 6);
      CHECK(next(u8"ab cd", TextBoxOf(fixture, u8"strict").Style()) == 2);
    }

    SECTION("Iterator modes")
    {
      CHECK(TextUtil::LineBreakIteratorMode(LineBreak::Auto) == TextBreakLineBehavior::Default);
      CHECK(TextUtil::LineBreakIteratorMode(LineBreak::Anywhere) == TextBreakLineBehavior::Default);
      CHECK(TextUtil::LineBreakIteratorMode(LineBreak::Loose) == TextBreakLineBehavior::Loose);
      CHECK(TextUtil::LineBreakIteratorMode(LineBreak::Normal) == TextBreakLineBehavior::Normal);
      CHECK(TextUtil::LineBreakIteratorMode(LineBreak::Strict) == TextBreakLineBehavior::Strict);
      CHECK(TextUtil::GetContentAnalysis(WordBreak::BreakAll)
            == TextBreakIterator::ContentAnalysis::Mechanical);
      CHECK(TextUtil::GetContentAnalysis(WordBreak::AutoPhrase)
            == TextBreakIterator::ContentAnalysis::Linguistic);
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "TextUtil: white space and text properties",
                   "[Layout][Inline][TextUtil]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
      #pre { white-space: pre }
      #preLine { white-space: pre-line }
      #preWrap { white-space: pre-wrap }
      #breakSpaces { white-space: break-spaces }
      #nowrap { white-space: nowrap }
      #hang { hanging-punctuation: first last }
      #hangEnd { hanging-punctuation: allow-end }
    </style><body>
    <div id=normal>&#xE9;a</div>
    <div id=combining>e&#x301;a</div>
    <div id=pre>x</div>
    <div id=preLine>x</div>
    <div id=preWrap>x</div>
    <div id=breakSpaces>x</div>
    <div id=nowrap>x</div>
    <div id=hang>“ab”</div>
    <div id=hangEnd>ab,</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("White space")
    {
      auto check =
        [&](dom::DOMStringView id, bool preserveSpaces, bool preserveNewline, bool wraps, bool hangs)
      {
        auto &box = TextBoxOf(fixture, id);
        CHECK(TextUtil::ShouldPreserveSpacesAndTabs(box) == preserveSpaces);
        CHECK(TextUtil::ShouldPreserveNewline(box) == preserveNewline);
        CHECK(TextUtil::IsWrappingAllowed(box.Style()) == wraps);
        CHECK(TextUtil::ShouldTrailingWhitespaceHang(box.Style()) == hangs);
      };
      check(u8"normal", false, false, true, false);
      check(u8"pre", true, true, false, false);
      check(u8"preLine", false, true, true, false);
      check(u8"preWrap", true, true, true, true);
      check(u8"breakSpaces", true, true, true, false);
      check(u8"nowrap", false, false, false, false);
    }

    SECTION("User-perceived characters")
    {
      auto &precomposed = TextBoxOf(fixture, u8"normal");
      REQUIRE(precomposed.CanUseSimpleFontCodePath());
      CHECK(TextUtil::FirstUserPerceivedCharacterLength(precomposed, 0, 3) == 2);
      CHECK(TextUtil::FirstUserPerceivedCharacterLength(precomposed, 2, 1) == 1);

      auto &combining = TextBoxOf(fixture, u8"combining");
      REQUIRE_FALSE(combining.CanUseSimpleFontCodePath());
      CHECK(TextUtil::FirstUserPerceivedCharacterLength(combining, 0, 4) == 3);

      Items items {fixture, u8"combining"};
      CHECK(TextUtil::FirstUserPerceivedCharacterLength(items[0]) == 3);
    }

    SECTION("Hanging punctuation")
    {
      Items hang {fixture, u8"hang"};
      auto &style = hang[0].Style();
      CHECK(TextUtil::HasHangablePunctuationStart(hang[0], style));
      CHECK(TextUtil::HasHangablePunctuationEnd(hang[0], style));
      CHECK_FALSE(TextUtil::HasHangableStopOrCommaEnd(hang[0], style));
      // The quotation marks are three bytes long.
      auto &font = style.GetFontCascade();
      CHECK(TextUtil::HangablePunctuationStartWidth(hang[0], style)
            == TextUtil::Width(hang[0], font, 0, 3, 0));
      CHECK(TextUtil::HangablePunctuationEndWidth(hang[0], style) == TextUtil::Width(hang[0], font, 5, 8, 0));
      CHECK(TextUtil::HangableStopOrCommaEndWidth(hang[0], style) == 0.f);

      Items hangEnd {fixture, u8"hangEnd"};
      CHECK_FALSE(TextUtil::HasHangablePunctuationStart(hangEnd[0], hangEnd[0].Style()));
      CHECK(TextUtil::HasHangableStopOrCommaEnd(hangEnd[0], hangEnd[0].Style()));
      CHECK(TextUtil::HangableStopOrCommaEndWidth(hangEnd[0], hangEnd[0].Style()) == 10.f);

      Items none {fixture, u8"normal"};
      CHECK_FALSE(TextUtil::HasHangablePunctuationStart(none[0], none[0].Style()));
      CHECK(TextUtil::HangablePunctuationStartWidth(none[0], none[0].Style()) == 0.f);
    }

    SECTION("Characters and direction")
    {
      CHECK(TextUtil::LastBaseCharacterFromText(u8"ae\u0301") == U'e');
      CHECK(TextUtil::LastBaseCharacterFromText(u8"a\u05E9") == char32 {0x05E9});
      CHECK(TextUtil::LastBaseCharacterFromText(u8"\u0301") == 0);
      CHECK(TextUtil::LastBaseCharacterFromText(u8"") == 0);

      CHECK(TextUtil::DirectionForTextContent(u8"abc") == TextDirection::LTR);
      CHECK(TextUtil::DirectionForTextContent(u8"123 \u05E9") == TextDirection::RTL);
      CHECK(TextUtil::DirectionForTextContent(u8"123") == TextDirection::LTR);

      CHECK(TextUtil::EllipsisTextInInlineDirection() == StringAtom {u8"\u2026"});
      CHECK(TextUtil::EllipsisTextInInlineDirection(false) == StringAtom {u8"\u22EE"});
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "InlineTextItem", "[Layout][Inline][InlineTextItem]")
  {
    RegisterAhem();
    ResolverFixture fixture {u8R"html(<!DOCTYPE html><style>
      div { font: 10px Ahem }
    </style><body>
    <div id=text>abcdef</div>
    <div id=zwsp>a<span>&#x200B;</span>b</div>
    </body>)html"};
    (void)fixture.Document->UpdateLayout();

    SECTION("Left and right sides")
    {
      Items items {fixture, u8"text"};
      auto &item = items[0];
      CHECK(item.Width() == 60.f);

      auto left = item.Left(2);
      CHECK(left.Start() == 0);
      CHECK(left.Length() == 2);
      CHECK(left.Content() == u8"ab");
      CHECK_FALSE(left.Width());

      auto right = item.Right(2, 20.f);
      CHECK(right.Start() == 4);
      CHECK(right.End() == 6);
      CHECK(right.Content() == u8"ef");
      CHECK(right.Width() == 20.f);
      CHECK_FALSE(right.IsWhitespace());
      CHECK_FALSE(right.IsEmpty());
      CHECK(right.Direction() == TextDirection::LTR);
    }

    SECTION("Zero width spaces")
    {
      Items items {fixture, u8"zwsp"};
      CHECK_FALSE(items[0].IsZeroWidthSpaceSeparator());
      auto &zeroWidthSpace = items[2];
      CHECK(zeroWidthSpace.Length() == 3);
      CHECK(zeroWidthSpace.IsZeroWidthSpaceSeparator());
      // Zero width spaces aren't measured.
      CHECK_FALSE(zeroWidthSpace.Width());
      CHECK_FALSE(zeroWidthSpace.IsQuirkNonBreakingSpace());
    }
  }
}
