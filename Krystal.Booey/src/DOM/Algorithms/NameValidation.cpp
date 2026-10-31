#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Text/Encodings/Decode.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD bool IsXMLNameStartChar(char32 ch) noexcept
    {
      return ch == ':' || ch == '_' || krys::text::IsASCIIAlpha(ch) || (ch >= 0xC0 && ch <= 0xD6)
             || (ch >= 0xD8 && ch <= 0xF6) || (ch >= 0xF8 && ch <= 0x2FF) || (ch >= 0x370 && ch <= 0x37D)
             || (ch >= 0x37F && ch <= 0x1FFF) || (ch >= 0x200C && ch <= 0x200D)
             || (ch >= 0x2070 && ch <= 0x218F) || (ch >= 0x2C00 && ch <= 0x2FEF)
             || (ch >= 0x3001 && ch <= 0xD7FF) || (ch >= 0xF900 && ch <= 0xFDCF)
             || (ch >= 0xFDF0 && ch <= 0xFFFD) || (ch >= 0x10000 && ch <= 0xEFFFF);
    }

    KRYS_NODISCARD bool IsXMLNameChar(char32 ch) noexcept
    {
      return IsXMLNameStartChar(ch) || ch == '-' || ch == '.' || krys::text::IsASCIIDigit(ch) || ch == 0xB7
             || (ch >= 0x0300 && ch <= 0x036F) || (ch >= 0x203F && ch <= 0x2040);
    }
  }

  bool NameValidation::IsValidNamespacePrefix(DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return false;
    }

    for (const auto &ch : name)
    {
      if (krys::text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
      {
        return false;
      }
    }

    return true;
  }

  bool NameValidation::IsValidAttributeLocalName(DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return false;
    }

    for (const auto &ch : name)
    {
      if (krys::text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '=' || ch == '>')
      {
        return false;
      }
    }

    return true;
  }

  bool NameValidation::IsValidElementLocalName(DOMStringView name) noexcept
  {
    // 1. If name's length is 0, then return false.
    if (name.empty())
    {
      return false;
    }

    // 2. If name's 0th code point is an ASCII alpha:
    if (krys::text::IsASCIIAlpha(name[0]))
    {
      // 2a. If name contains ASCII whitespace, U+0000 NULL, U+002F (/), or U+003E (>), then return false.
      for (const auto &ch : name)
      {
        if (krys::text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
        {
          return false;
        }
      }

      // 2b. Return true.
      return true;
    }

    // FIXME: We need to iterate over codepoints instead of decoding the entire string at once
    auto decodeResult = krys::text::Decode<krys::text::UTF8>(name);
    krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);

    // 3. If name's 0th code point is not U+003A (:), U+005F (_), or in the range U+0080 to U+10FFFF,
    // inclusive, then return false.
    auto &utf32 = decodeResult.Output;
    auto first = utf32[0];
    if (first != ':' && first != '_' && !(first >= 0x0080 && first <= 0x10FFFF))
    {
      return false;
    }

    // 4. If name's subsequent code points, if any, are not ASCII alphas, ASCII digits, U+002D (-), U+002E
    // (.), U+003A (:), U+005F (_), or in the range U+0080 to U+10FFFF, inclusive, then return false.
    for (auto ch : utf32_stringview {utf32.begin() + 1uz, utf32.end()})
    {
      if (!krys::text::IsASCIIAlpha(ch) && !krys::text::IsASCIIDigit(ch) && ch != '-' && ch != '.'
          && ch != ':' && ch != '_' && !(ch >= 0x0080 && ch <= 0x10FFFF))
      {
        return false;
      }
    }

    // 5. Return true.
    return true;
  }

  bool NameValidation::IsValidDocTypeName(DOMStringView name) noexcept
  {
    for (const auto &ch : name)
    {
      if (krys::text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '>')
      {
        return false;
      }
    }

    return true;
  }

  ExceptionOr<QualifiedName> NameValidation::ValidateAndExtract(DOMStringAtom namespaceURI,
                                                                DOMStringAtom qualifiedName,
                                                                ValidateAndExtractContext context) noexcept
  {
    // 1. If namespace is the empty string, then set it to null.
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    // 2. Let prefix be null.
    DOMStringAtom prefix = DOMStringAtom::Null();

    // 3. Let localName be qualifiedName.
    DOMStringAtom localName = qualifiedName;

    // 4. If qualifiedName contains a U+003A (:):
    if (auto colon = qualifiedName.View().find(u8':'); colon != DOMStringView::npos)
    {
      // 4a. Set prefix to the part of qualifiedName before the first U+003A (:).
      prefix = DOMStringAtom(qualifiedName.View().substr(0, colon));

      // 4b. Set localName to the part of qualifiedName after the first U+003A (:).
      localName = DOMStringAtom(qualifiedName.View().substr(colon + 1));

      // 4c. If prefix is not a valid namespace prefix, then throw an "InvalidCharacterError" DOMException.
      if (!IsValidNamespacePrefix(prefix.View()))
      {
        return ExceptionCode::InvalidCharacterError;
      }
    }

    // 5. Assert: prefix is either null or a valid namespace prefix.
    krys_debug_assert(prefix == DOMStringAtom::Null() || IsValidNamespacePrefix(prefix.View()));

    // 6. If context is "attribute" and localName is not a valid attribute local name, then throw an
    // "InvalidCharacterError" DOMException.
    if (context == ValidateAndExtractContext::Attribute && !IsValidAttributeLocalName(localName.View()))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 7. If context is "element" and localName is not a valid element local name, then throw an
    // "InvalidCharacterError" DOMException.
    if (context == ValidateAndExtractContext::Element && !IsValidElementLocalName(localName.View()))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    // 8. If prefix is non-null and namespace is null, then throw a "NamespaceError" DOMException.
    if (prefix != DOMStringAtom::Null() && namespaceURI == DOMStringAtom::Null())
    {
      return ExceptionCode::NamespaceError;
    }

    // 9. If prefix is "xml" and namespace is not the XML namespace, then throw a "NamespaceError"
    // DOMException.
    if (prefix == infra::NamespacePrefixes::XML && namespaceURI != infra::Namespaces::XML)
    {
      return ExceptionCode::NamespaceError;
    }

    // 10. If either qualifiedName or prefix is "xmlns" and namespace is not the XMLNS namespace, then throw a
    // "NamespaceError" DOMException.
    bool namedXMLNS =
      qualifiedName == infra::NamespacePrefixes::XMLNS || prefix == infra::NamespacePrefixes::XMLNS;
    if (namedXMLNS && namespaceURI != infra::Namespaces::XMLNS)
    {
      return ExceptionCode::NamespaceError;
    }

    // 11. If namespace is the XMLNS namespace and neither qualifiedName nor prefix is "xmlns", then throw a
    // "NamespaceError" DOMException.
    if (namespaceURI == infra::Namespaces::XMLNS && !namedXMLNS)
    {
      return ExceptionCode::NamespaceError;
    }

    // 12. Return (namespace, prefix, localName).
    return QualifiedName {namespaceURI, prefix, localName};
  }

  bool NameValidation::IsValidShadowHostName(DOMStringView name) noexcept
  {
    if (IsValidCustomElementName(name))
    {
      return true;
    }

    static Array<DOMStringView, 18> validNames = {u8"article", u8"aside",   u8"blockquote", u8"body", u8"div",
                                                  u8"footer",  u8"h1",      u8"h2",         u8"h3",   u8"h4",
                                                  u8"h5",      u8"h6",      u8"header",     u8"main", u8"nav",
                                                  u8"p",       u8"section", u8"span"};

    if (std::ranges::contains(validNames, name))
    {
      return true;
    }

    return false;
  }

  bool NameValidation::IsValidCustomElementName(DOMStringView name) noexcept
  {
    if (!IsValidElementLocalName(name))
    {
      return false;
    }

    if (!krys::text::IsASCIILower(name[0]))
    {
      return false;
    }

    if (std::ranges::any_of(name, [](char8 ch) { return krys::text::IsASCIIUpper(ch); }))
    {
      return false;
    }

    if (!std::ranges::contains(name, u8'-'))
    {
      return false;
    }

    static Array<DOMStringView, 8> reservedNames = {
      u8"annotation-xml", u8"color-profile",    u8"font-face",      u8"font-face-src",
      u8"font-face-uri",  u8"font-face-format", u8"font-face-name", u8"missing-glyph"};

    if (std::ranges::contains(reservedNames, name))
    {
      return false;
    }

    return true;
  }

  bool NameValidation::IsValidXMLName(DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return false;
    }

    // FIXME: We can't guarantee that the name is valid UTF-8, validate before converting.
    auto decodeResult = krys::text::Decode<krys::text::UTF8>(name);
    krys_debug_assert(decodeResult.Error == krys::text::DecodeError::None);
    auto &utf32 = decodeResult.Output;
    if (!IsXMLNameStartChar(utf32[0]))
    {
      return false;
    }

    for (auto ch : utf32_stringview {utf32.begin() + 1uz, utf32.end()})
    {
      if (!IsXMLNameChar(ch))
      {
        return false;
      }
    }

    return true;
  }
}