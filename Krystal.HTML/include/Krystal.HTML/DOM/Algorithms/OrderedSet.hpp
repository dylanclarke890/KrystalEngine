#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Lib/Types/Span.hpp"

namespace Krys::HTML
{
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