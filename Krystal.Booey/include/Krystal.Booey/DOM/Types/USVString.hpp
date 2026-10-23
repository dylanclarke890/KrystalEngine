#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"

namespace krys::boo::dom
{
  /// @brief A string of Unicode scalar values. Booey's strings are UTF-8, which holds nothing else (it can't
  /// encode a lone surrogate), so it's a `DOMString` kept apart only where the IDL types it apart.
  /// @see https://webidl.spec.whatwg.org/#idl-USVString
  class USVString : public DOMString
  {
  public:
    using DOMString::DOMString;

    USVString() noexcept = default;

    explicit USVString(DOMString &&string) noexcept : DOMString(krys::move(string))
    {
    }

    explicit USVString(const DOMString &string) noexcept : DOMString(string)
    {
    }

    explicit USVString(DOMStringView string) noexcept : DOMString(string)
    {
    }
  };
}
