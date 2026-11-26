#pragma once

#include "Krystal.Booey/Css/Types/CSSOMString.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::html::presentational_hints
{
  /// @brief Whether the attribute is @p localName in the null namespace.
  KRYS_NODISCARD inline bool IsAttribute(const DOMStringAtom &localName, const DOMStringAtom &namespaceURI,
                                         DOMStringView expected) noexcept
  {
    return namespaceURI == null && localName == expected;
  }

  KRYS_NODISCARD inline CSSOMString ToCSSOMString(DOMStringView value) noexcept
  {
    return CSSOMString(value);
  }
}
