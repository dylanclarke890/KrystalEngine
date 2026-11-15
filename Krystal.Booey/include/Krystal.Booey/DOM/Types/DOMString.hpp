#pragma once

#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo::dom
{
  using DOMString = utf8_string;

  using DOMStringView = utf8_stringview;

  using DOMStringAtom = StringAtom;

  inline const StringAtom &StarAtom() noexcept
  {
    static StringAtom starAtom = u8"*";
    return starAtom;
  }
}