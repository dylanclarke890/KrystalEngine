#include "Krystal.Booey/Core/TextSpacing.hpp"

namespace krys::boo
{
  using namespace text_spacing;

  bool TextSpacingTrim::ShouldTrimSpacing(const CharactersData &charactersData) const noexcept
  {
    switch (_trim)
    {
      case TrimType::SpaceAll: return false;
      case TrimType::Auto:     return false;
      case TrimType::TrimAll:
        return charactersData.CurrentCharacterClass == CharacterClass::FullWidthOpeningPunctuation
               || charactersData.CurrentCharacterClass == CharacterClass::FullWidthClosingPunctuation
               || charactersData.CurrentCharacterClass == CharacterClass::FullWidthMiddleDotPunctuation;
      default: return false;
    }
  }

  bool TextAutospace::ShouldApplySpacing(CharacterClass firstCharacterClass,
                                         CharacterClass secondCharacterClass) const noexcept
  {
    constexpr uint8 ideographAlphaMask =
      static_cast<uint8>(CharacterClass::Ideograph) | static_cast<uint8>(CharacterClass::NonIdeographLetter);
    constexpr uint8 ideographNumericMask =
      static_cast<uint8>(CharacterClass::Ideograph) | static_cast<uint8>(CharacterClass::NonIdeographNumeral);

    uint8 characterClassesMask =
      static_cast<uint8>(static_cast<uint8>(firstCharacterClass) | static_cast<uint8>(secondCharacterClass));
    if (HasIdeographAlpha() && characterClassesMask == ideographAlphaMask)
    {
      return true;
    }
    if (HasIdeographNumeric() && characterClassesMask == ideographNumericMask)
    {
      return true;
    }
    return false;
  }
}
