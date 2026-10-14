#include "Krystal.Booey/CSS/Selectors/SelectorPseudoTypeMap.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD bool IsPseudoClass(CSSOMStringView name, SelectorPseudoClass expected) noexcept
    {
      auto [pseudoClass, compatibilityPseudoElement] = FindPseudoClassAndCompatibilityElementName(name);
      return pseudoClass == expected && !compatibilityPseudoElement;
    }

    KRYS_NODISCARD bool IsCompatibilityPseudoElement(CSSOMStringView name,
                                                     SelectorPseudoElement expected) noexcept
    {
      auto [pseudoClass, compatibilityPseudoElement] = FindPseudoClassAndCompatibilityElementName(name);
      return !pseudoClass && compatibilityPseudoElement == expected;
    }

    KRYS_NODISCARD bool IsUnknownAfterSingleColon(CSSOMStringView name) noexcept
    {
      auto [pseudoClass, compatibilityPseudoElement] = FindPseudoClassAndCompatibilityElementName(name);
      return !pseudoClass && !compatibilityPseudoElement;
    }
  }

  TEST_CASE("SelectorEnums(traits)", "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    STATIC_CHECK(EnumTraits<SelectorPseudoClass>::DistinctValues == 63uz);
    STATIC_CHECK(EnumTraits<SelectorPseudoElement>::DistinctValues == 21uz);
    STATIC_CHECK(BitCount<SelectorPseudoClass>() == 6u);
    STATIC_CHECK(BitCount<SelectorPseudoElement>() == 5u);
  }

  TEST_CASE("FindPseudoClassAndCompatibilityElementName(pseudo-classes)",
            "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(IsPseudoClass(u8"hover", SelectorPseudoClass::Hover));
    CHECK(IsPseudoClass(u8"nth-last-of-type", SelectorPseudoClass::NthLastOfType));
    CHECK(IsPseudoClass(u8"is", SelectorPseudoClass::Is));
    CHECK(IsPseudoClass(u8"where", SelectorPseudoClass::Where));
    CHECK(IsPseudoClass(u8"active-view-transition-type", SelectorPseudoClass::ActiveViewTransitionType));
    CHECK(IsPseudoClass(u8"popover-open", SelectorPseudoClass::PopoverOpen));
    CHECK(IsPseudoClass(u8"volume-locked", SelectorPseudoClass::VolumeLocked));
  }

  TEST_CASE("FindPseudoClassAndCompatibilityElementName(case-insensitive)",
            "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(IsPseudoClass(u8"HOVER", SelectorPseudoClass::Hover));
    CHECK(IsPseudoClass(u8"Focus-Visible", SelectorPseudoClass::FocusVisible));
    CHECK(IsCompatibilityPseudoElement(u8"BEFORE", SelectorPseudoElement::Before));
  }

  TEST_CASE("FindPseudoClassAndCompatibilityElementName(compatibility pseudo-elements)",
            "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(IsCompatibilityPseudoElement(u8"before", SelectorPseudoElement::Before));
    CHECK(IsCompatibilityPseudoElement(u8"after", SelectorPseudoElement::After));
    CHECK(IsCompatibilityPseudoElement(u8"first-line", SelectorPseudoElement::FirstLine));
    CHECK(IsCompatibilityPseudoElement(u8"first-letter", SelectorPseudoElement::FirstLetter));

    // Other pseudo-elements need two colons.
    CHECK(IsUnknownAfterSingleColon(u8"marker"));
    CHECK(IsUnknownAfterSingleColon(u8"selection"));
    CHECK(IsUnknownAfterSingleColon(u8"placeholder"));
  }

  TEST_CASE("FindPseudoClassAndCompatibilityElementName(unsupported)",
            "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(IsUnknownAfterSingleColon(u8""));
    CHECK(IsUnknownAfterSingleColon(u8"not-a-pseudo-class"));
    CHECK(IsUnknownAfterSingleColon(u8"hover-"));
    CHECK(IsUnknownAfterSingleColon(u8"hövér"));
    CHECK(IsUnknownAfterSingleColon(CSSOMStringView(u8"hover\0", 6uz)));

    // Prefixed, non-standard and legacy alias names are not supported.
    CHECK(IsUnknownAfterSingleColon(u8"matches"));
    CHECK(IsUnknownAfterSingleColon(u8"-webkit-any"));
    CHECK(IsUnknownAfterSingleColon(u8"-webkit-any-link"));
    CHECK(IsUnknownAfterSingleColon(u8"-webkit-autofill"));
    CHECK(IsUnknownAfterSingleColon(u8"-webkit-full-screen"));
    CHECK(IsUnknownAfterSingleColon(u8"-internal-html-document"));
    CHECK(IsUnknownAfterSingleColon(u8"window-inactive"));
    CHECK(IsUnknownAfterSingleColon(u8"horizontal"));
    CHECK(IsUnknownAfterSingleColon(u8"immersive"));
  }

  TEST_CASE("FindPseudoElementName", "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(FindPseudoElementName(u8"before") == SelectorPseudoElement::Before);
    CHECK(FindPseudoElementName(u8"marker") == SelectorPseudoElement::Marker);
    CHECK(FindPseudoElementName(u8"view-transition-image-pair")
          == SelectorPseudoElement::ViewTransitionImagePair);
    CHECK(FindPseudoElementName(u8"Highlight") == SelectorPseudoElement::Highlight);
    CHECK(FindPseudoElementName(u8"cue") == SelectorPseudoElement::Cue);
  }

  TEST_CASE("FindPseudoElementName(user agent parts)", "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK(FindPseudoElementName(u8"placeholder") == SelectorPseudoElement::UserAgentPart);
    CHECK(FindPseudoElementName(u8"PLACEHOLDER") == SelectorPseudoElement::UserAgentPart);
    CHECK(FindPseudoElementName(u8"file-selector-button") == SelectorPseudoElement::UserAgentPart);
    CHECK(FindPseudoElementName(u8"details-content") == SelectorPseudoElement::UserAgentPart);
    CHECK(FindPseudoElementName(u8"thumb") == SelectorPseudoElement::UserAgentPart);
    CHECK(FindPseudoElementName(u8"track") == SelectorPseudoElement::UserAgentPart);
  }

  TEST_CASE("FindPseudoElementName(unsupported)", "[CSS][Selectors][SelectorPseudoTypeMap]")
  {
    CHECK_FALSE(FindPseudoElementName(u8""));
    CHECK_FALSE(FindPseudoElementName(u8"hover"));
    CHECK_FALSE(FindPseudoElementName(u8"not-a-pseudo-element"));
    CHECK_FALSE(FindPseudoElementName(CSSOMStringView(u8"before\0", 7uz)));

    // Prefixed and legacy alias names are not supported.
    CHECK_FALSE(FindPseudoElementName(u8"-webkit-input-placeholder"));
    CHECK_FALSE(FindPseudoElementName(u8"-webkit-file-upload-button"));
    CHECK_FALSE(FindPseudoElementName(u8"-webkit-scrollbar"));
    CHECK_FALSE(FindPseudoElementName(u8"-webkit-slider-thumb"));
    CHECK_FALSE(FindPseudoElementName(u8"-internal-writing-suggestions"));
  }
}
