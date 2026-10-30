#pragma once

#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html::presentational_hints
{
  /// @brief Whether the attribute is @p localName in the null namespace.
  KRYS_NODISCARD inline bool IsAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                         dom::DOMStringView expected) noexcept
  {
    return namespaceURI == dom::DOMStringAtom::Null() && localName == expected;
  }

  KRYS_NODISCARD inline css::CSSOMString ToCSSOMString(dom::DOMStringView value) noexcept
  {
    return css::CSSOMString(value);
  }
}
