#pragma once

#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  /// @brief A string of Unicode scalar values. Krystal.Web's strings are UTF-8, which holds nothing else (it can't
  /// encode a lone surrogate), so it's a `DOMString` kept apart only where the IDL types it apart.
  /// @see https://webidl.spec.whatwg.org/#idl-USVString
  class USVString : public DOMString
  {
  public:
    using DOMString::DOMString;

    USVString() noexcept = default;

    explicit USVString(DOMString string) noexcept : DOMString(krys::move(string))
    {
    }

    explicit USVString(DOMStringView string) noexcept : DOMString(string)
    {
    }
  };
}
