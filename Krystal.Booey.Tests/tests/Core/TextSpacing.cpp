#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Core/CharacterProperties.hpp"
#include "Krystal.Booey/Core/Hyphenation.hpp"
#include "Krystal.Booey/Core/UnicodeHelpers.hpp"

namespace krys::boo::tests
{
  using text_spacing::CharacterClass;
  using text_spacing::GetCharacterClass;

  TEST_CASE("TextSpacing: character classes", "[Core][TextSpacing]")
  {
    CHECK(GetCharacterClass(U'a') == CharacterClass::NonIdeographLetter);
    CHECK(GetCharacterClass(U'é') == CharacterClass::NonIdeographLetter);
    CHECK(GetCharacterClass(U'7') == CharacterClass::NonIdeographNumeral);
    CHECK(GetCharacterClass(U'٣') == CharacterClass::NonIdeographNumeral);
    CHECK(GetCharacterClass(U'漢') == CharacterClass::Ideograph);
    CHECK(GetCharacterClass(U'あ') == CharacterClass::Ideograph);
    CHECK(GetCharacterClass(U'「') == CharacterClass::FullWidthOpeningPunctuation);
    CHECK(GetCharacterClass(U'」') == CharacterClass::FullWidthClosingPunctuation);
    CHECK(GetCharacterClass(U'‘') == CharacterClass::FullWidthOpeningPunctuation);
    CHECK(GetCharacterClass(U'”') == CharacterClass::FullWidthClosingPunctuation);
    CHECK(GetCharacterClass(U'・') == CharacterClass::FullWidthMiddleDotPunctuation);
    // Fullwidth Latin letters aren't spaced.
    CHECK(GetCharacterClass(U'Ａ') == CharacterClass::Undefined);
    CHECK(GetCharacterClass(U' ') == CharacterClass::Undefined);

    CHECK(text_spacing::IsIdeograph(U'一'));
    CHECK(text_spacing::IsIdeograph(U'ー'));
    CHECK(text_spacing::IsIdeograph(U'㇀'));
    CHECK_FALSE(text_spacing::IsIdeograph(U'・'));
    CHECK_FALSE(text_spacing::IsIdeograph(U'a'));
  }

  TEST_CASE("TextSpacing: autospace between characters", "[Core][TextSpacing]")
  {
    TextAutospace normal {{TextAutospaceType::Normal}};
    CHECK(normal.ShouldApplySpacing(U'漢', U'a'));
    CHECK(normal.ShouldApplySpacing(U'1', U'漢'));
    CHECK_FALSE(normal.ShouldApplySpacing(U'a', U'b'));

    TextAutospace numeric {{TextAutospaceType::IdeographNumeric}};
    CHECK(numeric.ShouldApplySpacing(U'漢', U'1'));
    CHECK_FALSE(numeric.ShouldApplySpacing(U'漢', U'a'));

    CHECK_FALSE(TextAutospace {}.ShouldApplySpacing(U'漢', U'a'));
  }

  TEST_CASE("CharacterProperties", "[Core][CharacterProperties]")
  {
    CHECK(IsEmojiGroupCandidate(U'\U0001F600'));
    CHECK_FALSE(IsEmojiGroupCandidate(U'a'));
    CHECK(IsEmojiFitzpatrickModifier(U'\U0001F3FD'));
    CHECK(IsVariationSelector(U'️'));
    CHECK(IsEmojiKeycapBase(U'#'));
    CHECK(IsEmojiRegionalIndicator(U'\U0001F1EB'));
    CHECK(IsEmojiWithPresentationByDefault(U'\U0001F600'));
    CHECK_FALSE(IsEmojiWithPresentationByDefault(U'©'));
    CHECK(IsEmojiModifierBase(U'\U0001F44D'));
    CHECK(IsDefaultIgnorableCodePoint(U'​'));
    CHECK(IsControlCharacter(U'\n'));
    CHECK(IsPrivateUseAreaCharacter(U''));
    CHECK(IsPunctuation(U','));
    CHECK_FALSE(IsPunctuation(U'a'));
    CHECK(IsOfScriptType(U'ب', USCRIPT_ARABIC));
    CHECK(IsEastAsianFullWidth(U'Ａ'));
    CHECK_FALSE(IsEastAsianFullWidth(U'漢'));
    CHECK(IsCJKSymbolOrPunctuation(U'、'));
    CHECK(IsCombiningMark(U'́'));
    CHECK(IsCurrencySymbol(U'€'));
  }

  TEST_CASE("UnicodeHelpers: base text direction", "[Core][UnicodeHelpers]")
  {
    CHECK(BaseTextDirection(u8"abc") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"© 2025") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"שלום abc") == TextDirection::RTL);
    CHECK(BaseTextDirection(u8"123 مرحبا") == TextDirection::RTL);
    CHECK(BaseTextDirection(u8"€ 1") == null);
  }

  TEST_CASE("Hyphenation", "[Core][Hyphenation]")
  {
    CHECK_FALSE(CanHyphenate(StringAtom {u8"en"}));
    CHECK(EnoughWidthForHyphenation(21, 16));
    CHECK_FALSE(EnoughWidthForHyphenation(20, 16));
  }
}
