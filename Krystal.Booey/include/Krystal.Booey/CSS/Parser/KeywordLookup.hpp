#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::css
{
  /// @brief Looks up a property by name, ignoring ASCII case.
  /// @return The property, or PropertyId::Invalid if the name is empty, too long, contains non-ASCII
  /// characters or isn't a known property.
  KRYS_NODISCARD css::PropertyId PropertyIdFromName(CSSOMStringView name) noexcept;

  /// @brief Looks up a value keyword by name, ignoring ASCII case. Legacy `-apple-` keywords are treated
  /// as their `-webkit-` equivalents.
  /// @return The keyword, or ValueId::Invalid if the name is empty, too long, contains non-ASCII characters
  /// or isn't a known keyword.
  KRYS_NODISCARD css::ValueId ValueIdFromName(CSSOMStringView name) noexcept;
}
