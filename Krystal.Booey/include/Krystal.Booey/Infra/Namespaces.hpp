#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::infra
{
  /// @see https://infra.spec.whatwg.org/#namespaces
  namespace Namespaces
  {
    /// @see https://infra.spec.whatwg.org/#xml-namespace
    constexpr utf8_stringview XML = u8"http://www.w3.org/XML/1998/namespace";
    /// @see https://infra.spec.whatwg.org/#xmlns-namespace
    constexpr utf8_stringview XMLNS = u8"http://www.w3.org/2000/xmlns/";
    /// @see https://infra.spec.whatwg.org/#html-namespace
    constexpr utf8_stringview HTML = u8"http://www.w3.org/1999/xhtml";
    /// @see https://infra.spec.whatwg.org/#svg-namespace
    constexpr utf8_stringview SVG = u8"http://www.w3.org/2000/svg";
    /// @see https://infra.spec.whatwg.org/#mathml-namespace
    constexpr utf8_stringview MathML = u8"http://www.w3.org/1998/Math/MathML";
    /// @see https://infra.spec.whatwg.org/#xlink-namespace
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