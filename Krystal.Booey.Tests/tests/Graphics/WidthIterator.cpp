#include "Krystal.Booey/Graphics/WidthIterator.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/FontDatabase.hpp"
#include "Krystal.Booey/Graphics/GlyphBuffer.hpp"
#include "Krystal.Booey/Graphics/TextRun.hpp"
#include "Krystal.Booey/Graphics/WidthCache.hpp"

namespace krys::boo::tests
{
  namespace
  {
    KRYS_NODISCARD FontCascade Ahem()
    {
      (void)FontDatabase::Singleton().AddFontFile(std::filesystem::path {KRYS_BOOEY_TESTS_DATA_DIR} / "fonts"
                                                  / "Ahem.ttf");
      FontCascadeDescription description;
      description.SetFamilies({StringAtom {u8"Ahem"}});
      description.SetComputedSize(16);
      FontCascade cascade {krys::move(description)};
      cascade.Update();
      return cascade;
    }
  }

  TEST_CASE("WidthIterator", "[Graphics][WidthIterator]")
  {
    auto cascade = Ahem();

    SECTION("Glyphs are added per character, with byte offsets")
    {
      TextRun run {u8"a\u4E00b"};
      WidthIterator it {cascade, run};
      GlyphBuffer buffer;
      it.Advance(1, buffer);
      CHECK(it.CurrentCharacterIndex() == 1);
      CHECK(it.RunWidthSoFar() == 16);
      // Advancing into a character lays out the whole character.
      it.Advance(2, buffer);
      CHECK(it.CurrentCharacterIndex() == 4);
      it.Advance(run.Length(), buffer);
      it.Finalize(buffer);
      CHECK(it.RunWidthSoFar() == 48);
      REQUIRE(buffer.Size() == 3);
      CHECK(buffer.UncheckedStringOffsetAt(0) == 0);
      CHECK(buffer.UncheckedStringOffsetAt(1) == 1);
      CHECK(buffer.UncheckedStringOffsetAt(2) == 4);
      CHECK(buffer.GlyphAt(0) == cascade.PrimaryFont()->GlyphForCharacter(U'a'));
      CHECK(&buffer.FontAt(0) == cascade.PrimaryFont().get());
    }

    SECTION("Fallback fonts are collected")
    {
      TextRun run {u8"a\u0416b"};
      HashSet<RefPtr<const Font>> fallbackFonts;
      WidthIterator it {cascade, run, &fallbackFonts};
      GlyphBuffer buffer;
      it.Advance(run.Length(), buffer);
      it.Finalize(buffer);
      REQUIRE(fallbackFonts.size() == 1);
      CHECK((*fallbackFonts.begin())->PlatformData().Face() == &FontDatabase::Singleton().BundledFace());
      CHECK(&buffer.FontAt(1) == fallbackFonts.begin()->get());
    }

    SECTION("Advancing one character at a time")
    {
      TextRun run {u8"a\u4E00"};
      WidthIterator it {cascade, run};
      GlyphBuffer buffer;
      float width = 0;
      CHECK(it.AdvanceOneCharacter(width, buffer));
      CHECK(width == 16);
      CHECK(it.AdvanceOneCharacter(width, buffer));
      CHECK(width == 16);
      CHECK(it.CurrentCharacterIndex() == 4);
      CHECK_FALSE(it.AdvanceOneCharacter(width, buffer));
    }

    SECTION("Tabs are invisible")
    {
      TextRun run {u8"a\tb"};
      run.SetTabSize(true, TabSize {8});
      WidthIterator it {cascade, run};
      GlyphBuffer buffer;
      it.Advance(run.Length(), buffer);
      REQUIRE(buffer.Size() == 3);
      CHECK(buffer.GlyphAt(1) == DeletedGlyph);
      CHECK(Width(buffer.AdvanceAt(1)) == 112);
    }

    SECTION("RTL expansion is added before the run")
    {
      // The space is the run's last character, so in RTL its expansion is on the run's left.
      TextRun run {u8"a ", 0, 10,
                   ExpansionBehavior {ExpansionBehavior::Behavior::Allow, ExpansionBehavior::Behavior::Allow},
                   TextDirection::RTL};
      WidthIterator it {cascade, run};
      GlyphBuffer buffer;
      it.Advance(run.Length(), buffer);
      it.Finalize(buffer);
      CHECK(it.RunWidthSoFar() == 42);
      CHECK(Width(buffer.InitialAdvance()) == 10);
    }

    SECTION("Glyph bounds")
    {
      TextRun run {u8"ab"};
      WidthIterator it {cascade, run, nullptr, true};
      GlyphBuffer buffer;
      it.Advance(run.Length(), buffer);
      CHECK(it.MinGlyphBoundingBoxY() == Catch::Approx(-12.8f));
      CHECK(it.MaxGlyphBoundingBoxY() == Catch::Approx(3.2f));
      CHECK(it.FirstGlyphOverflow() == 0);
      CHECK(it.LastGlyphOverflow() == 0);
    }

    SECTION("Characters for simplified measuring")
    {
      CHECK(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(U'a', true));
      CHECK(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(U'\n', false));
      CHECK_FALSE(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(U'\t', false));
      CHECK_FALSE(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(text::ZeroWidthJoiner, true));
      CHECK_FALSE(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(text::HiraganaLetterSmallA, true));
      CHECK_FALSE(WidthIterator::CharacterCanUseSimplifiedTextMeasuring(U'\u0001', true));
    }
  }

  TEST_CASE("WidthCache", "[Graphics][WidthCache]")
  {
    WidthCache cache;

    SECTION("Words are sampled, and hits sample the next words")
    {
      // The cache starts by sampling one word in 21.
      for (int32 i = 0; i < 20; ++i)
      {
        CHECK_FALSE(cache.Add(u8"word", 5));
      }
      auto *entry = cache.Add(u8"word", 5);
      REQUIRE(entry);
      CHECK(*entry == 5);
      for (int32 i = 0; i < 20; ++i)
      {
        CHECK_FALSE(cache.Add(u8"word", 7));
      }
      // A hit returns the cached width, and makes the cache sample every word.
      entry = cache.Add(u8"word", 7);
      REQUIRE(entry);
      CHECK(*entry == 5);
      CHECK(cache.Add(u8"\u4E00", 9));
      CHECK(*cache.Add(u8"\u4E00", 3) == 9);

      cache.Clear();
      CHECK(*cache.Add(u8"word", 7) == 7);
    }

    SECTION("Empty and long words aren't cached")
    {
      CHECK_FALSE(cache.Add(u8"", 1));
      CHECK_FALSE(cache.Add(u8"seventeen letters", 1));
    }

    SECTION("Runs are only cached when the cache helps and the width doesn't depend on context")
    {
      TextRun run {u8"word"};
      CHECK_FALSE(cache.Add(run, 1, false, false, false, nullptr));
      CHECK_FALSE(cache.Add(run, 1, true, true, false, nullptr));
      GlyphOverflow overflow;
      CHECK_FALSE(cache.Add(run, 1, true, false, false, &overflow));
      TextRun tabs {u8"word"};
      tabs.SetTabSize(true, TabSize {8});
      CHECK_FALSE(cache.Add(tabs, 1, true, false, false, nullptr));
      // Once an ideograph is seen, text-spacing runs aren't cached.
      CHECK_FALSE(cache.Add(TextRun {u8"\u4E00"}, 1, true, false, true, nullptr));
      CHECK_FALSE(cache.Add(run, 1, true, false, true, nullptr));
    }
  }

  TEST_CASE("GlyphBuffer and TextRun", "[Graphics][GlyphBuffer]")
  {
    auto cascade = Ahem();
    auto &font = *cascade.PrimaryFont();

    SECTION("Glyph buffers")
    {
      GlyphBuffer buffer;
      CHECK(buffer.IsEmpty());
      buffer.Add(1, font, 10, 0);
      buffer.Add(2, font, 20, 1);
      buffer.Add(3, font, MakeGlyphBufferAdvance(30, 0), 2, FloatPoint {1, 2});
      CHECK(buffer.Size() == 3);
      CHECK(buffer.CheckedStringOffsetAt(2, 3) == 2u);
      CHECK_FALSE(buffer.CheckedStringOffsetAt(2, 2));
      CHECK_FALSE(buffer.IsFlattened());

      buffer.Reverse(0, 3);
      CHECK(buffer.GlyphAt(0) == 3);
      CHECK(buffer.GlyphAt(2) == 1);

      buffer.ExpandAdvance(0, 5);
      CHECK(Width(buffer.AdvanceAt(0)) == 35);
      buffer.ExpandAdvanceToLogicalRight(1, 1);
      CHECK(X(buffer.OriginAt(1)) == 1);

      buffer.MakeHole(1, 2, &font);
      CHECK(buffer.Size() == 5);
      CHECK(buffer.GlyphAt(1) == 0xFFFF);
      buffer.Remove(1, 2);
      CHECK(buffer.Size() == 3);

      buffer.DeleteGlyphWithoutAffectingSize(2);
      CHECK(buffer.GlyphAt(2) == DeletedGlyph);
      CHECK(Width(buffer.AdvanceAt(2)) == 0);

      buffer.ExpandInitialAdvance(4);
      buffer.Flatten();
      CHECK(buffer.IsFlattened());
      // The first glyph's origin moves into the initial advance.
      CHECK(Width(buffer.InitialAdvance()) == 5);

      buffer.Shrink(1);
      CHECK(buffer.Size() == 1);
      buffer.Clear();
      CHECK(buffer.IsEmpty());
    }

    SECTION("Text runs")
    {
      TextRun run {u8"a\u4E00b"};
      CHECK(run.Length() == 5);
      CHECK(run.CharacterAt(1) == U'\u4E00');
      // A continuation byte isn't a character.
      CHECK(run.CharacterAt(2) == text::UnicodeReplacementChar);
      auto sub = run.SubRun(1, 3);
      CHECK(sub.Text() == u8"\u4E00");
      CHECK(sub != run);
      sub.SetText(run.Text());
      CHECK(sub == run);
      CHECK(run.Ltr());
      run.SetDirection(TextDirection::RTL);
      CHECK(run.Rtl());
      CHECK(run.ExpansionBehavior() == ExpansionBehavior::DefaultBehavior());
      CHECK(run.HorizontalGlyphStretch() == 1);
      CHECK(run.CharacterScanForCodePath());
      CHECK_FALSE(run.SpacingDisabled());
      run.DisableSpacing();
      CHECK(run.SpacingDisabled());
    }
  }
}
