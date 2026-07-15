#pragma once

#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/String/StringAtom.hpp"

namespace Krys::HTML
{
  using DOMString = utf8_string;

  using DOMStringView = utf8_stringview;

  using DOMStringAtom = StringAtom;
}