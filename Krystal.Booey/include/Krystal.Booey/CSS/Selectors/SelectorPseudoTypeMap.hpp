#pragma once

#include "Krystal.Booey/CSS/Selectors/SelectorEnums.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  /// @brief The result of looking up a name written after a single colon: a pseudo-class, or one of the
  /// pseudo-elements that can also be written with a single colon (`:before`, `:after`, `:first-line`,
  /// `:first-letter`).
  struct PseudoClassOrCompatibilityPseudoElement
  {
    Maybe<SelectorPseudoClass> PseudoClass;
    Maybe<SelectorPseudoElement> CompatibilityPseudoElement;
  };

  /// @brief Looks up the name of a pseudo-class or single-colon pseudo-element, ignoring ASCII case.
  /// @param name The name without the leading colon.
  /// @return Both members are empty if the name is unknown.
  KRYS_NODISCARD PseudoClassOrCompatibilityPseudoElement
    FindPseudoClassAndCompatibilityElementName(CSSOMStringView name) noexcept;

  /// @brief Looks up the name of a pseudo-element, ignoring ASCII case.
  /// @param name The name without the leading colons.
  /// @return `SelectorPseudoElement::UserAgentPart` for the parts of a user agent shadow tree, or nothing if
  /// the name is unknown.
  KRYS_NODISCARD Maybe<SelectorPseudoElement> FindPseudoElementName(CSSOMStringView name) noexcept;
}
