#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"

namespace krys::boo::tests
{
  namespace
  {
    void RegisterAhem()
    {
      (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                  / "Ahem.ttf");
    }

    /// @brief A 16px Ahem cascade: every Ahem glyph is a 1em square, so most characters are 16px wide.
    KRYS_NODISCARD FontCascade Ahem(float size = 16)
    {
      RegisterAhem();
      FontCascadeDescription description;
      description.SetFamilies({StringAtom {u8"Ahem"}});
      description.SetComputedSize(size);
      FontCascade cascade {krys::move(description)};
      cascade.Update();
      return cascade;
    }
  }

  TEST_CASE("FontCascade: widths", "[Graphics][FontCascade]")
  {
    auto cascade = Ahem();

    SECTION("Text")
    {
      CHECK(cascade.Width(u8"") == 0);
      CHECK(cascade.Width(u8"abc") == 48);
      CHECK(cascade.WidthOfSpaceString() == 16);
      // Offsets are UTF-8 bytes, but widths are per character.
      CHECK(cascade.Width(u8"a\u4E00b") == 48);
    }

    SECTION("Letter and word spacing")
    {
      cascade.SetLetterSpacing(2);
      CHECK(cascade.Width(u8"abc") == 54);
      cascade.SetLetterSpacing(0);
      cascade.SetWordSpacing(4);
      CHECK(cascade.Width(u8"a b") == 52);
      // A leading space gets no word spacing.
      CHECK(cascade.Width(u8" b") == 32);
    }

    SECTION("Expansion")
    {
      TextRun run {u8"a b c", 0, 10};
      CHECK(cascade.Width(run) == 90);
      TextRun rtl {u8"a b c", 0, 10, ExpansionBehavior::DefaultBehavior(), TextDirection::RTL};
      CHECK(cascade.Width(rtl) == 90);
    }

    SECTION("Tabs")
    {
      TextRun run {u8"a\tb"};
      // Without tab stops, a tab is as wide as its glyph.
      CHECK(cascade.Width(run) == 48);
      run.SetTabSize(true, TabSize {8});
      // The tab reaches the next tab stop: 8 spaces from the start of the line.
      CHECK(cascade.Width(run) == 144);
      run.SetXPos(100);
      CHECK(cascade.Width(run) == 16 + (128 - 116) + 16);
      // A tab narrower than half a space reaches the next tab stop instead.
      CHECK(
        cascade.TabWidth(*cascade.PrimaryFont(), TabSize {8}, 120, Font::SyntheticBoldInclusion::Incorporate)
        == 8);
      CHECK(
        cascade.TabWidth(*cascade.PrimaryFont(), TabSize {8}, 124, Font::SyntheticBoldInclusion::Incorporate)
        == 132);
    }

    SECTION("Invisible and control characters")
    {
      // Default ignorable characters are dropped.
      CHECK(cascade.Width(u8"a\u200Bb") == 32);
      // Newlines are drawn as spaces.
      CHECK(cascade.Width(u8"a\nb") == 48);
      // Other control characters are drawn as the missing glyph.
      auto notdef = cascade.PrimaryFont()->WidthForGlyph(0);
      CHECK(cascade.Width(u8"a\u0001b") == Catch::Approx(32 + notdef));
    }

    SECTION("Small caps are synthesized when the font has none")
    {
      auto description = cascade.GetFontDescription();
      description.SetVariantCaps(FontVariantCaps::Small);
      FontCascade smallCaps {krys::move(description)};
      smallCaps.Update();
      CHECK(smallCaps.Width(u8"A") == 16);
      CHECK(smallCaps.Width(u8"a") == Catch::Approx(11.2f));
      CHECK(smallCaps.Width(u8"Aa") == Catch::Approx(27.2f));
    }

    SECTION("text-autospace")
    {
      auto description = cascade.GetFontDescription();
      description.SetTextAutospace(TextAutospace {TextAutospace::Options {TextAutospaceType::Normal}});
      FontCascade autospace {krys::move(description)};
      autospace.Update();
      auto spacing = TextAutospace::TextAutospaceSize(*autospace.PrimaryFont());
      CHECK(spacing == Catch::Approx(2.f));
      CHECK(autospace.Width(u8"a\u4E00") == Catch::Approx(32 + spacing));
      CHECK(autospace.Width(u8"ab") == 32);
    }

    SECTION("Ranges")
    {
      TextRun run {u8"a\u4E00bc"};
      float before = 0;
      float after = 0;
      CHECK(cascade.WidthOfTextRange(run, 1, 4, before, after) == 16);
      CHECK(before == 16);
      CHECK(after == 32);
      CHECK(cascade.WidthForCharacterInRun(run, 1) == 16);
    }

    SECTION("Fixed pitch")
    {
      CHECK(cascade.WidthForSimpleTextWithFixedPitch(u8"a\u00E9b", true) == 48);
      cascade.SetWordSpacing(4);
      CHECK(cascade.WidthForSimpleTextWithFixedPitch(u8"a b\n", false) == 52);
      CHECK(cascade.WidthForTextUsingSimplifiedMeasuring(u8"abc") == 48);
    }

    SECTION("Glyph overflow")
    {
      GlyphOverflow overflow;
      overflow.ComputeBounds = true;
      CHECK(cascade.Width(TextRun {u8"ab"}, nullptr, &overflow) == 32);
      // Ahem's glyphs fill the ascent (0.8em) and descent (0.2em) exactly.
      CHECK(overflow.Top.ToFloat() == Catch::Approx(12.8f).margin(0.02f));
      CHECK(overflow.Bottom.ToFloat() == Catch::Approx(3.2f).margin(0.02f));
      CHECK(overflow.Left == 0);
      CHECK(overflow.Right == 0);
    }
  }

  TEST_CASE("FontCascade: positions and selection", "[Graphics][FontCascade]")
  {
    auto cascade = Ahem();

    SECTION("Offsets for positions")
    {
      TextRun run {u8"a\u4E00bc"};
      CHECK(cascade.OffsetForPosition(run, 20, false) == 1);
      CHECK(cascade.OffsetForPosition(run, 20, true) == 1);
      CHECK(cascade.OffsetForPosition(run, 26, true) == 4);
      CHECK(cascade.OffsetForPosition(run, 40, false) == 4);
      CHECK(cascade.OffsetForPosition(run, 1'000, false) == 6);

      TextRun rtl {u8"abcd", 0, 0, ExpansionBehavior::DefaultBehavior(), TextDirection::RTL};
      // In RTL, the first character is on the right.
      CHECK(cascade.OffsetForPosition(rtl, 60, false) == 0);
      CHECK(cascade.OffsetForPosition(rtl, 4, false) == 3);
    }

    SECTION("Selection rects")
    {
      TextRun run {u8"abcd"};
      LayoutRect rect {0, 0, 0, 20};
      cascade.AdjustSelectionRectForText(false, run, rect, 1, 3);
      CHECK(rect == LayoutRect {16, 0, 32, 20});

      TextRun rtl {u8"abcd", 0, 0, ExpansionBehavior::DefaultBehavior(), TextDirection::RTL};
      LayoutRect rtlRect {0, 0, 0, 20};
      cascade.AdjustSelectionRectForText(false, rtl, rtlRect, 0, 1);
      CHECK(rtlRect == LayoutRect {48, 0, 16, 20});

      LayoutRect fixedPitch {0, 0, 0, 20};
      cascade.AdjustSelectionRectForText(true, run, fixedPitch, 1, 3);
      CHECK(fixedPitch == LayoutRect {16, 0, 32, 20});

      TextRun wide {u8"a\u4E00"};
      auto rects = cascade.CharacterSelectionRectsForText(wide, LayoutRect {0, 0, 0, 20}, 0, null);
      REQUIRE(rects.size() == 4);
      CHECK(rects[0] == LayoutRect {0, 0, 16, 20});
      CHECK(rects[1] == LayoutRect {16, 0, 16, 20});
      // A character's continuation bytes have empty rects at its end.
      CHECK(rects[2] == LayoutRect {32, 0, 0, 20});
      CHECK(rects[3] == LayoutRect {32, 0, 0, 20});
    }
  }

  TEST_CASE("FontCascade: text properties", "[Graphics][FontCascade]")
  {
    SECTION("Code paths")
    {
      CHECK(FontCascade::CharacterRangeCodePath(u8"abc") == FontCascade::CodePath::Simple);
      CHECK(FontCascade::CharacterRangeCodePath(u8"e\u0301") == FontCascade::CodePath::Complex);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\u1E00")
            == FontCascade::CodePath::SimpleWithGlyphOverflow);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\u0627") == FontCascade::CodePath::Complex);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\U0001F1E6") == FontCascade::CodePath::Complex);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\U00011000") == FontCascade::CodePath::Complex);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\u4E00\U0001F600") == FontCascade::CodePath::Simple);
      CHECK(FontCascade::CharacterRangeCodePath(u8"\U0001F468\u200D\U0001F469")
            == FontCascade::CodePath::Complex);
      CHECK(FontCascade::CharacterRangeCodePath(u8"a\u200Db") == FontCascade::CodePath::Simple);

      auto cascade = Ahem();
      TextRun run {u8"e\u0301"};
      CHECK(cascade.GetCodePath(run) == FontCascade::CodePath::Complex);
      run.SetCharacterScanForCodePath(false);
      CHECK(cascade.GetCodePath(run) == FontCascade::CodePath::Simple);
      FontCascade::SetCodePath(FontCascade::CodePath::Complex);
      CHECK(cascade.GetCodePath(TextRun {u8"abc"}) == FontCascade::CodePath::Complex);
      FontCascade::SetCodePath(FontCascade::CodePath::Auto);
    }

    SECTION("Expansion opportunities")
    {
      auto count = [](utf8_stringview text, ExpansionBehavior behavior = ExpansionBehavior::DefaultBehavior(),
                      TextDirection direction = TextDirection::LTR)
      {
        return FontCascade::ExpansionOpportunityCount(text, direction, behavior);
      };

      CHECK(count(u8"a b c") == std::pair<uint32, bool> {2, false});
      CHECK(count(u8"a b ") == std::pair<uint32, bool> {2, true});
      // A trailing opportunity can be forbidden.
      CHECK(count(u8"a b ", ExpansionBehavior {ExpansionBehavior::Behavior::Forbid,
                                               ExpansionBehavior::Behavior::Forbid})
            == std::pair<uint32, bool> {1, false});
      CHECK(count(u8"ab",
                  ExpansionBehavior {ExpansionBehavior::Behavior::Force, ExpansionBehavior::Behavior::Force})
            == std::pair<uint32, bool> {2, true});
      CHECK(count(u8" ab", ExpansionBehavior::DefaultBehavior(), TextDirection::RTL)
            == std::pair<uint32, bool> {1, true});

      CHECK_FALSE(FontCascade::LeftExpansionOpportunity(u8"\u4E00", TextDirection::LTR));
      CHECK(FontCascade::RightExpansionOpportunity(u8"a ", TextDirection::LTR));
      CHECK_FALSE(FontCascade::RightExpansionOpportunity(u8"a ", TextDirection::RTL));
      CHECK(FontCascade::RightExpansionOpportunity(u8" \u4E00", TextDirection::RTL));
      CHECK_FALSE(FontCascade::RightExpansionOpportunity(u8"", TextDirection::LTR));
    }

    SECTION("Text emphasis")
    {
      CHECK(FontCascade::CanReceiveTextEmphasis(U'a'));
      CHECK(FontCascade::CanReceiveTextEmphasis(U'#'));
      CHECK_FALSE(FontCascade::CanReceiveTextEmphasis(U' '));
      CHECK_FALSE(FontCascade::CanReceiveTextEmphasis(U','));
      CHECK_FALSE(FontCascade::CanReceiveTextEmphasis(text::EthiopicWordspace));

      auto cascade = Ahem();
      CHECK(cascade.EmphasisMarkHeight(StringAtom {u8""}) == 0);
      // Emphasis marks use a half size font.
      CHECK(cascade.FloatEmphasisMarkHeight(StringAtom {u8"x"}) == Catch::Approx(8.f));
      CHECK(cascade.EmphasisMarkAscent(StringAtom {u8"x"}) + cascade.EmphasisMarkDescent(StringAtom {u8"x"})
            == cascade.EmphasisMarkHeight(StringAtom {u8"x"}));
    }

    SECTION("Simplified text measuring")
    {
      auto cascade = Ahem();
      auto &primaryFont = *cascade.PrimaryFont();
      CHECK(cascade.CanUseSimplifiedTextMeasuring(U'a', AutoVariant, true, primaryFont));
      // The second query is cached.
      CHECK(cascade.CanUseSimplifiedTextMeasuring(U'a', AutoVariant, true, primaryFont));
      // Tabs are control characters, so even collapsed ones can't.
      CHECK_FALSE(cascade.CanUseSimplifiedTextMeasuring(U'\t', AutoVariant, true, primaryFont));
      CHECK_FALSE(cascade.CanUseSimplifiedTextMeasuring(U'\t', AutoVariant, false, primaryFont));
      CHECK_FALSE(cascade.CanUseSimplifiedTextMeasuring(U'\u00A0', AutoVariant, true, primaryFont));
      CHECK_FALSE(cascade.CanUseSimplifiedTextMeasuring(U'\u3042', AutoVariant, true, primaryFont));
      // Characters from a fallback font can't.
      CHECK_FALSE(cascade.CanUseSimplifiedTextMeasuring(U'\u0416', AutoVariant, true, primaryFont));
    }

    SECTION("Average character width")
    {
      auto cascade = Ahem();
      float width = 0;
      CHECK(cascade.HasValidAverageCharWidth());
      CHECK(cascade.FastAverageCharWidthIfAvailable(width));
      CHECK(width == std::round(cascade.PrimaryFont()->AvgCharWidth()));

      FontCascadeDescription description;
      description.SetFamilies({StringAtom {u8"Times"}});
      CHECK_FALSE(FontCascade {krys::move(description)}.HasValidAverageCharWidth());
    }

    SECTION("Capitalization")
    {
      CHECK(Capitalized(U'a') == U'A');
      CHECK_FALSE(Capitalized(U'A'));
      CHECK_FALSE(Capitalized(U'\u0301'));

      auto cascade = Ahem();
      auto *font = cascade.PrimaryFont().get();
      CHECK_FALSE(ShouldSynthesizeSmallCaps(false, font, U'a', U'A', FontVariantCaps::Normal, false));
      CHECK(ShouldSynthesizeSmallCaps(false, font, U'a', U'A', FontVariantCaps::Small, false));
      CHECK_FALSE(ShouldSynthesizeSmallCaps(true, font, U'a', U'A', FontVariantCaps::Small, false));
      CHECK_FALSE(ShouldSynthesizeSmallCaps(false, font, U'A', null, FontVariantCaps::Small, false));
      CHECK(ShouldSynthesizeSmallCaps(false, font, U'A', null, FontVariantCaps::AllSmall, true));
      CHECK_FALSE(ShouldSynthesizeSmallCaps(false, font, U' ', null, FontVariantCaps::AllSmall, true));
    }
  }

  TEST_CASE("FontCascade: where glyphs cross a line", "[Graphics][FontCascade]")
  {
    auto cascade = Ahem();

    // Each Ahem glyph is a 16px square from 12.8px above the baseline to 3.2px below it, so a line 1px below
    // the baseline crosses every glyph but the space.
    TextRun run {utf8_stringview {u8"XX X"}};
    auto segments =
      cascade.LineSegmentsForIntersectionsWithRect(run, FloatPoint(10, 20), FloatRect(10, 21, 64, 1));
    REQUIRE(segments.size() == 3u);
    CHECK(segments[0].Begin == Catch::Approx(0.f));
    CHECK(segments[0].End == Catch::Approx(16.f));
    CHECK(segments[1].Begin == Catch::Approx(16.f));
    CHECK(segments[1].End == Catch::Approx(32.f));
    CHECK(segments[2].Begin == Catch::Approx(48.f));
    CHECK(segments[2].End == Catch::Approx(64.f));

    // A line below the glyphs crosses nothing.
    CHECK(cascade.LineSegmentsForIntersectionsWithRect(run, FloatPoint(10, 20), FloatRect(10, 30, 64, 1))
            .empty());
  }
}
