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
    CHECK(GetCharacterClass(U'\u00E9') == CharacterClass::NonIdeographLetter);
    CHECK(GetCharacterClass(U'7') == CharacterClass::NonIdeographNumeral);
    CHECK(GetCharacterClass(U'\u0663') == CharacterClass::NonIdeographNumeral);
    CHECK(GetCharacterClass(U'\u6F22') == CharacterClass::Ideograph);
    CHECK(GetCharacterClass(U'\u3042') == CharacterClass::Ideograph);
    CHECK(GetCharacterClass(U'\u300C') == CharacterClass::FullWidthOpeningPunctuation);
    CHECK(GetCharacterClass(U'\u300D') == CharacterClass::FullWidthClosingPunctuation);
    CHECK(GetCharacterClass(U'\u2018') == CharacterClass::FullWidthOpeningPunctuation);
    CHECK(GetCharacterClass(U'\u201D') == CharacterClass::FullWidthClosingPunctuation);
    CHECK(GetCharacterClass(U'\u30FB') == CharacterClass::FullWidthMiddleDotPunctuation);
    // Fullwidth Latin letters aren't spaced.
    CHECK(GetCharacterClass(U'\uFF21') == CharacterClass::Undefined);
    CHECK(GetCharacterClass(U' ') == CharacterClass::Undefined);

    CHECK(text_spacing::IsIdeograph(U'\u4E00'));
    CHECK(text_spacing::IsIdeograph(U'\u30FC'));
    CHECK(text_spacing::IsIdeograph(U'\u31C0'));
    CHECK_FALSE(text_spacing::IsIdeograph(U'\u30FB'));
    CHECK_FALSE(text_spacing::IsIdeograph(U'a'));
  }

  TEST_CASE("TextSpacing: autospace between characters", "[Core][TextSpacing]")
  {
    TextAutospace normal {{TextAutospaceType::Normal}};
    CHECK(normal.ShouldApplySpacing(U'\u6F22', U'a'));
    CHECK(normal.ShouldApplySpacing(U'1', U'\u6F22'));
    CHECK_FALSE(normal.ShouldApplySpacing(U'a', U'b'));

    TextAutospace numeric {{TextAutospaceType::IdeographNumeric}};
    CHECK(numeric.ShouldApplySpacing(U'\u6F22', U'1'));
    CHECK_FALSE(numeric.ShouldApplySpacing(U'\u6F22', U'a'));

    CHECK_FALSE(TextAutospace {}.ShouldApplySpacing(U'\u6F22', U'a'));
  }

  TEST_CASE("CharacterProperties", "[Core][CharacterProperties]")
  {
    CHECK(IsEmojiGroupCandidate(U'\U0001F600'));
    CHECK_FALSE(IsEmojiGroupCandidate(U'a'));
    CHECK(IsEmojiFitzpatrickModifier(U'\U0001F3FD'));
    CHECK(IsVariationSelector(U'\uFE0F'));
    CHECK(IsEmojiKeycapBase(U'#'));
    CHECK(IsEmojiRegionalIndicator(U'\U0001F1EB'));
    CHECK(IsEmojiWithPresentationByDefault(U'\U0001F600'));
    CHECK_FALSE(IsEmojiWithPresentationByDefault(U'\u00A9'));
    CHECK(IsEmojiModifierBase(U'\U0001F44D'));
    CHECK(IsDefaultIgnorableCodePoint(U'\u200B'));
    CHECK(IsControlCharacter(U'\n'));
    CHECK(IsPrivateUseAreaCharacter(U'\uE000'));
    CHECK(IsPunctuation(U','));
    CHECK_FALSE(IsPunctuation(U'a'));
    CHECK(IsOfScriptType(U'\u0628', USCRIPT_ARABIC));
    CHECK(IsEastAsianFullWidth(U'\uFF21'));
    CHECK_FALSE(IsEastAsianFullWidth(U'\u6F22'));
    CHECK(IsCJKSymbolOrPunctuation(U'\u3001'));
    CHECK(IsCombiningMark(U'\u0301'));
    CHECK(IsCurrencySymbol(U'\u20AC'));
  }

  TEST_CASE("UnicodeHelpers: base text direction", "[Core][UnicodeHelpers]")
  {
    CHECK(BaseTextDirection(u8"abc") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"\u00A9 2025") == TextDirection::LTR);
    CHECK(BaseTextDirection(u8"\u05E9\u05DC\u05D5\u05DD abc") == TextDirection::RTL);
    CHECK(BaseTextDirection(u8"123 \u0645\u0631\u062D\u0628\u0627") == TextDirection::RTL);
    CHECK(BaseTextDirection(u8"\u20AC 1") == null);
  }

  TEST_CASE("Hyphenation", "[Core][Hyphenation]")
  {
    CHECK_FALSE(CanHyphenate(StringAtom {u8"en"}));
    CHECK(EnoughWidthForHyphenation(21, 16));
    CHECK_FALSE(EnoughWidthForHyphenation(20, 16));
  }
}
