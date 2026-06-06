#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"

namespace Krys::HTML
{
  namespace Namespace
  {
    inline DOMStringAtom XML = u8"http://www.w3.org/XML/1998/namespace";
    inline DOMStringAtom XMLNS = u8"http://www.w3.org/2000/xmlns/";
    inline DOMStringAtom HTML = u8"http://www.w3.org/1999/xhtml";
    inline DOMStringAtom SVG = u8"http://www.w3.org/2000/svg";
    inline DOMStringAtom MathML = u8"http://www.w3.org/1998/Math/MathML";
    inline DOMStringAtom XLink = u8"http://www.w3.org/1999/xlink";
  }

  namespace NamespacePrefix
  {
    inline DOMStringAtom XML = u8"xml";
    inline DOMStringAtom XMLNS = u8"xmlns";
    inline DOMStringAtom HTML = u8"html";
    inline DOMStringAtom SVG = u8"svg";
    inline DOMStringAtom MathML = u8"mathml";
    inline DOMStringAtom XLink = u8"xlink";
  }
}