#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web::html::presentational_hints
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
