#include "Krystal.Text/CharacterProperties.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Text/Locale.hpp"

namespace krys::text::tests
{
  TEST_CASE("CharacterProperties: general category", "[Text][CharacterProperties]")
  {
    CHECK(GetGeneralCategory(U'a') == GeneralCategory::LowercaseLetter);
    CHECK(GetGeneralCategory(U'(') == GeneralCategory::OpenPunctuation);
    CHECK(GetGeneralCategory(U'́') == GeneralCategory::NonspacingMark);
    CHECK(IsInGeneralCategory(U'。', PunctuationCategories));
    CHECK_FALSE(IsInGeneralCategory(U'a', PunctuationCategories | MarkCategories));
    CHECK(GetEastAsianWidth(U'あ') == EastAsianWidth::Wide);
    CHECK(GetEastAsianWidth(U'a') == EastAsianWidth::Narrow);
  }

  TEST_CASE("CharacterProperties: other properties", "[Text][CharacterProperties]")
  {
    CHECK(IsPrintable(U'a'));
    CHECK(IsPrintable(U' '));
    CHECK_FALSE(IsPrintable(U'\u0007'));

    CHECK(IsWhiteSpace(U' '));
    CHECK(IsWhiteSpace(U'　'));
    CHECK_FALSE(IsWhiteSpace(U'a'));

    CHECK(IsInCJKBlock(U'日'));
    CHECK(IsInCJKBlock(U'あ'));
    CHECK(IsInCJKBlock(U'가'));
    CHECK_FALSE(IsInCJKBlock(U'a'));
  }

  TEST_CASE("CharacterProperties: FullWidthForm", "[Text][CharacterProperties]")
  {
    CHECK(FullWidthForm(U'a') == U'ａ');
    CHECK(FullWidthForm(U' ') == U'　');
    // Half-width katakana have narrow decompositions to their full-width forms.
    CHECK(FullWidthForm(U'ｱ') == U'ア');
    CHECK(FullWidthForm(U'日') == U'日');
  }

  TEST_CASE("Locale: default locale", "[Text][Locale]")
  {
    auto previous = DefaultLocale();
    SetDefaultLocale(u8"fr_FR");
    CHECK(DefaultLocale() == u8"fr_FR");
    SetDefaultLocale(previous);
    CHECK(DefaultLocale() == previous);
  }
}
