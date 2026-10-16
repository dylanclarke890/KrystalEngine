#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD FontCascade Cascade(const char8_t *family, Kerning kerning = Kerning::Auto,
                                       TextRenderingMode textRendering = TextRenderingMode::Auto)
    {
      (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                  / "Ahem.ttf");
      FontCascadeDescription description;
      description.SetFamilies({StringAtom {family}});
      description.SetComputedSize(20);
      description.SetKerning(kerning);
      description.SetTextRenderingMode(textRendering);
      FontCascade cascade {krys::move(description)};
      cascade.Update();
      return cascade;
    }

    KRYS_NODISCARD TextRun Rtl(utf8_stringview text)
    {
      return TextRun {text, 0, 0, ExpansionBehavior::DefaultBehavior(), TextDirection::RTL};
    }

    // Liberation Sans at 20px: A and V are 1366/2048em, and the pair kerns by -152/2048em.
    constexpr float AdvanceOfA = 13.33984f;
    constexpr float KernedAV = 25.19531f;
  }

  TEST_CASE("ComplexTextController: shaping", "[Graphics][ComplexTextController]")
  {
    auto liberation = Cascade(u8"Liberation Sans");

    SECTION("Fonts have HarfBuzz fonts")
    {
      auto *font = liberation.PrimaryFont()->HarfBuzzFont();
      CHECK(font);
      CHECK(liberation.PrimaryFont()->HarfBuzzFont() == font);
    }

    SECTION("Kerning")
    {
      TextRun run {u8"AV"};
      ComplexTextController controller {liberation, run};
      CHECK(controller.TotalAdvance().Width() == Catch::Approx(KernedAV));
      // Text is shaped by default (kerning is on), so widths are kerned.
      CHECK(liberation.Width(run) == Catch::Approx(KernedAV));

      // Without kerning, shaping turns kerning off.
      auto noKerning = Cascade(u8"Liberation Sans", Kerning::NoShift);
      ComplexTextController unkerned {noKerning, run};
      CHECK(unkerned.TotalAdvance().Width() == Catch::Approx(2 * AdvanceOfA));

      // Text rendered for speed isn't shaped: the simple path measures it.
      auto fast = Cascade(u8"Liberation Sans", Kerning::Auto, TextRenderingMode::OptimizeSpeed);
      CHECK(fast.Width(run) == Catch::Approx(2 * AdvanceOfA));
    }

    SECTION("Combining characters are composed")
    {
      TextRun run {u8"e\u0301x"};
      CHECK(liberation.GetCodePath(run) == FontCascade::CodePath::Complex);
      ComplexTextController controller {liberation, run};
      GlyphBuffer buffer;
      controller.Advance(run.Length(), &buffer);
      REQUIRE(buffer.Size() == 2);
      CHECK(buffer.GlyphAt(0) == liberation.PrimaryFont()->GlyphForCharacter(U'\u00E9'));
      CHECK(buffer.UncheckedStringOffsetAt(0) == 0);
      CHECK(buffer.UncheckedStringOffsetAt(1) == 3);

      float before = 0;
      float after = 0;
      auto acute = liberation.WidthOfTextRange(run, 0, 3, before, after);
      CHECK(acute == Catch::Approx(liberation.Width(u8"\u00E9")));
      CHECK(before == 0);
      CHECK(after == Catch::Approx(10.f));

      // Positions inside the cluster go to its edges.
      CHECK(liberation.OffsetForPosition(run, 5, false) == 0);
      CHECK(liberation.OffsetForPosition(run, 8, true) == 3);
      CHECK(liberation.OffsetForPosition(run, 1'000, true) == 4);
    }

    SECTION("RTL text")
    {
      // Hebrew shin, lamed, vav, final mem.
      auto run = Rtl(u8"\u05E9\u05DC\u05D5\u05DD");
      ComplexTextController controller {liberation, run};
      auto total = controller.TotalAdvance().Width();
      CHECK(total == Catch::Approx(43.95508f));

      // Glyphs are emitted in string order.
      GlyphBuffer buffer;
      controller.Advance(run.Length(), &buffer);
      REQUIRE(buffer.Size() == 4);
      CHECK(buffer.UncheckedStringOffsetAt(0) == 0);
      CHECK(buffer.UncheckedStringOffsetAt(3) == 6);

      // The run starts on the right.
      CHECK(liberation.OffsetForPosition(run, 1, true) == 8);
      CHECK(liberation.OffsetForPosition(run, total - 1, true) == 0);

      auto rects = liberation.CharacterSelectionRectsForText(run, LayoutRect {0, 0, 0, 20}, 0, 2);
      REQUIRE(rects.size() == 2);
      auto shin =
        liberation.PrimaryFont()->WidthForGlyph(liberation.PrimaryFont()->GlyphForCharacter(U'\u05E9'));
      CHECK(rects[0].X().ToFloat() == Catch::Approx(total - shin).margin(0.02f));
      CHECK(rects[0].Width() == LayoutUnit::FromFloatCeil(shin));
      // The second byte of shin adds nothing.
      CHECK(rects[1].Width() == 0);

      auto advances = ComplexTextController::GlyphAdvancesForTextRun(liberation, run);
      REQUIRE(advances.size() == 8);
      CHECK(advances[0] == Catch::Approx(shin));
      CHECK(advances[1] == 0);
      CHECK(advances[0] + advances[2] + advances[4] + advances[6] == Catch::Approx(total));
    }

    SECTION("Marks don't advance")
    {
      // Shin with qamats, then lamed.
      auto run = Rtl(u8"\u05E9\u05B8\u05DC");
      CHECK(liberation.GetCodePath(run) == FontCascade::CodePath::Complex);
      ComplexTextController controller {liberation, run};
      auto &font = *liberation.PrimaryFont();
      CHECK(controller.TotalAdvance().Width()
            == Catch::Approx(font.WidthForGlyph(font.GlyphForCharacter(U'\u05E9'))
                             + font.WidthForGlyph(font.GlyphForCharacter(U'\u05DC'))));
    }

    SECTION("Clusters a family lacks fall back as a whole")
    {
      auto ahem = Cascade(u8"Ahem");
      TextRun run {u8"a\u0301"};
      HashSet<RefPtr<const Font>> fallbackFonts;
      // Shaped widths don't report their fallback fonts; advancing through the glyphs does.
      (void)ahem.Width(run, &fallbackFonts);
      CHECK(fallbackFonts.empty());
      ComplexTextController controller {ahem, run};
      controller.Advance(run.Length(), nullptr, GlyphIterationStyle::IncludePartialGlyphs, &fallbackFonts);
      REQUIRE(fallbackFonts.size() == 1);
      CHECK((*fallbackFonts.begin())->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      auto font = ahem.FontForCombiningCharacterSequence(u8"a\u0301");
      REQUIRE(font);
      CHECK(font->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      // A single character keeps the family's font.
      CHECK(ahem.FontForCombiningCharacterSequence(u8"a").get() == ahem.PrimaryFont().get());
    }

    SECTION("Glyph bounds")
    {
      auto ahem = Cascade(u8"Ahem");
      auto [ascent, descent] = ComplexTextController::EnclosingGlyphBoundsForTextRun(ahem, TextRun {u8"ab"});
      CHECK(ascent == Catch::Approx(-16.f));
      CHECK(descent == Catch::Approx(4.f));

      GlyphOverflow overflow;
      overflow.ComputeBounds = true;
      CHECK(ahem.Width(TextRun {u8"ab"}, nullptr, &overflow) == 40);
      CHECK(overflow.Top.ToFloat() == Catch::Approx(16.f).margin(0.02f));
      CHECK(overflow.Left == 0);
      CHECK(overflow.Right == 0);
    }
  }

  TEST_CASE("ComplexTextController: given runs", "[Graphics][ComplexTextController]")
  {
    auto liberation = Cascade(u8"Liberation Sans");
    auto &font = *liberation.PrimaryFont();

    // One glyph (a ligature, say) for two two-byte characters.
    utf8_stringview text = u8"\u00E9\u00E9";
    TextRun run {text};
    Array<FloatSize, 1> advances {FloatSize {10, 0}};
    Array<FloatPoint, 1> origins {FloatPoint {}};
    Array<Glyph, 1> glyphs {1};
    Array<uint32, 1> indices {0};
    List<Ref<ComplexTextController::ComplexTextRun>> runs;
    runs.push_back(ComplexTextController::ComplexTextRun::Create(advances, origins, glyphs, indices,
                                                                 FloatSize {}, font, text, 0, 0, 4, true));
    ComplexTextController controller {liberation, run, runs};
    CHECK(controller.TotalAdvance().Width() == Catch::Approx(10.f + font.SyntheticBoldOffset()));

    SECTION("A ligature's advance is divided between its characters")
    {
      controller.Advance(1);
      CHECK(controller.RunWidthSoFar() == Catch::Approx(5.f));
      controller.Advance(2);
      CHECK(controller.RunWidthSoFar() == Catch::Approx(5.f));
      controller.Advance(3);
      CHECK(controller.RunWidthSoFar() == Catch::Approx(10.f));
      controller.Advance(4);
      CHECK(controller.RunWidthSoFar() == Catch::Approx(10.f));
      // Going back starts again.
      controller.Advance(1, nullptr, GlyphIterationStyle::ByWholeGlyphs);
      CHECK(controller.RunWidthSoFar() == Catch::Approx(10.f));
    }

    SECTION("Positions inside a ligature")
    {
      CHECK(controller.OffsetForPosition(2, false) == 0);
      CHECK(controller.OffsetForPosition(7, false) == 2);
      CHECK(controller.OffsetForPosition(7, true) == 2);
      CHECK(controller.OffsetForPosition(9, true) == 4);
      CHECK(controller.OffsetForPosition(-1, true) == 0);
    }
  }
}
