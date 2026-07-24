#pragma once

#include "Krystal.Lib/Core/Enum.hpp"

namespace Krys::HTML
{
  enum class SelectorMatch : uint8
  {
    None = 0,
    /// @brief Matches the type (i.e. tag name) of the element. Includes the universal selector `*`.
    Type,
    /// @brief Matches the ID of the element.
    Id,
    /// @brief Matches the class of the element.
    Class,
    /// @brief Matches a pseudo-class of the element.
    PseudoClass,
    /// @brief Matches a pseudo-element of the element.
    PseudoElement,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SelectorMatch, 6uz);