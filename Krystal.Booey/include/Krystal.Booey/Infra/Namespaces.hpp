#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::infra
{
  namespace Namespaces
  {
    constexpr utf8_stringview XML = u8"http://www.w3.org/XML/1998/namespace";
    constexpr utf8_stringview XMLNS = u8"http://www.w3.org/2000/xmlns/";
    constexpr utf8_stringview HTML = u8"http://www.w3.org/1999/xhtml";
    constexpr utf8_stringview SVG = u8"http://www.w3.org/2000/svg";
    constexpr utf8_stringview MathML = u8"http://www.w3.org/1998/Math/MathML";
    constexpr utf8_stringview XLink = u8"http://www.w3.org/1999/xlink";
  }

  namespace NamespacePrefixes
  {
    constexpr utf8_stringview XML = u8"xml";
    constexpr utf8_stringview XMLNS = u8"xmlns";
    constexpr utf8_stringview HTML = u8"html";
    constexpr utf8_stringview SVG = u8"svg";
    constexpr utf8_stringview MathML = u8"mathml";
    constexpr utf8_stringview XLink = u8"xlink";
  }
}