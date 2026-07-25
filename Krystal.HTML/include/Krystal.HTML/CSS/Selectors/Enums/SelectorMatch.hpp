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
    /// @brief Matches an element when it has a specific attribute, regardless of the value of that attribute.
    AttributeExists,
    /// @brief Matches an element when it has a specific attribute with a specific value.
    AttributeEquals,
    /// @brief Matches an element when it has an attribute whose value either exactly matches or begins with a
    /// specific value immediately followed by "-" (U+002D).
    AttributeDash,
    /// @brief Matches an element when it has an attribute whose value is a whitespace-separated list of
    /// words, one of which is exactly a specific value.
    AttributeIncludes,
    /// @brief Matches an element when it has an attribute whose value starts with a specific value.
    AttributePrefix,
    /// @brief Matches an element when it has an attribute whose value ends with a specific value.
    AttributeSuffix,
    /// @brief Matches an element when it has an attribute whose value contains a specific value.
    AttributeSubstring,
    /// @brief Matches a nesting selector of the element.
    NestingParent,
    /// @brief Matches the :has() scope of the element.
    HasScope,
    /// @brief The selector is invalid or not supported. This is used for forgiving parsing of selectors that
    /// are not supported by the browser but need to be preserved for serialization.
    ForgivingUnknown,
    /// @brief The selector is invalid or not supported but contains a nesting combinator (&). This is used
    /// for forgiving parsing of selectors that are not supported by the browser but need to be preserved for
    /// serialization.
    ForgivingUnknownNestContaining,
  };
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(Krys::HTML::SelectorMatch, 14uz);