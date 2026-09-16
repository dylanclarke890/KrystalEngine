#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::dom
{
  /// @brief Implementations of the ordered set algorithms.
  /// @see https://dom.spec.whatwg.org/#ordered-sets
  class OrderedSet
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-ordered-set-parser
    KRYS_NODISCARD static List<DOMString> Parser(DOMStringView input) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-ordered-set-serializer
    KRYS_NODISCARD static DOMString Serializer(Span<DOMString> tokens) noexcept;
  };
}