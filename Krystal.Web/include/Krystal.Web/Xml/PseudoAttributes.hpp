#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web::xml
{
  /// @brief A pseudo-attribute: a name and its value, as in a processing instruction's data.
  /// @see https://www.w3.org/TR/xml-stylesheet/#dt-pseudo-attribute
  struct PseudoAttribute
  {
    /// @see https://www.w3.org/TR/xml-stylesheet/#dt-name
    DOMString Name;
    /// @see https://www.w3.org/TR/xml-stylesheet/#dt-value
    DOMString Value;
  };

  /// @brief Parses @p string as pseudo-attributes (`name="value" other='value'`), in order; null if it
  /// doesn't match the `PseudoAtts` production, has a character reference to a character XML doesn't allow,
  /// or has a name twice.
  /// @see https://www.w3.org/TR/xml-stylesheet/#dt-parsing
  /// @see https://www.w3.org/TR/xml-stylesheet/#dt-parsing-result
  KRYS_NODISCARD Maybe<List<PseudoAttribute>> ParsePseudoAttributes(DOMStringView string) noexcept;
}
