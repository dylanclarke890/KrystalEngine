#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML
{
  namespace
  {
    KRYS_NODISCARD bool IsXMLNameStartChar(char32 ch) noexcept
    {
      return ch == ':' || ch == '_' || Krys::Text::IsASCIIAlpha(ch) || (ch >= 0xC0 && ch <= 0xD6)
             || (ch >= 0xD8 && ch <= 0xF6) || (ch >= 0xF8 && ch <= 0x2FF) || (ch >= 0x370 && ch <= 0x37D)
             || (ch >= 0x37F && ch <= 0x1FFF) || (ch >= 0x200C && ch <= 0x200D)
             || (ch >= 0x2070 && ch <= 0x218F) || (ch >= 0x2C00 && ch <= 0x2FEF)
             || (ch >= 0x3001 && ch <= 0xD7FF) || (ch >= 0xF900 && ch <= 0xFDCF)
             || (ch >= 0xFDF0 && ch <= 0xFFFD) || (ch >= 0x10000 && ch <= 0xEFFFF);
    }

    KRYS_NODISCARD bool IsXMLNameChar(char32 ch) noexcept
    {
      return IsXMLNameStartChar(ch) || ch == '-' || ch == '.' || Krys::Text::IsASCIIDigit(ch) || ch == 0xB7
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
      if (Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
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
      if (Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '=' || ch == '>')
      {
        return false;
      }
    }

    return true;
  }

  bool NameValidation::IsValidElementLocalName(DOMStringView name) noexcept
  {
    if (name.empty())
    {
      return false;
    }

    if (Krys::Text::IsASCIIAlpha(name[0]))
    {
      for (const auto &ch : name)
      {
        if (Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
        {
          return false;
        }
      }

      return true;
    }

    auto utf32 = Krys::Text::ConvertToUTF32(utf8_stringview {name});
    auto ch = utf32[0];
    if (ch != ':' && ch != '_' && !(ch >= 0x0080 && ch <= 0x10FFF))
    {
      return false;
    }

    if (utf32.size() == 1uz)
    {
      return true;
    }

    for (auto ch : utf32_stringview {utf32.begin() + 1uz, utf32.end()})
    {
      if (!Krys::Text::IsASCIIAlpha(ch) && !Krys::Text::IsASCIIDigit(ch) && ch != '-' && ch != '.'
          && ch != ':' && ch != '_' && !(ch >= 0x0080 && ch <= 0x10FFF))
      {
        return false;
      }
    }

    return false;
  }

  bool NameValidation::IsValidDocTypeName(DOMStringView name) noexcept
  {
    for (const auto &ch : name)
    {
      if (Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '>')
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
    if (namespaceURI == DOMStringAtom::Empty())
    {
      namespaceURI = DOMStringAtom::Null();
    }

    DOMStringAtom prefix = DOMStringAtom::Null();

    DOMStringAtom localName = qualifiedName;
    if (localName.View().contains(':'))
    {
      auto splitResult = StringAlgorithms::StrictlySplit(localName.View(), ':');
      prefix = splitResult[0];
      localName = splitResult[1];

      if (!IsValidNamespacePrefix(prefix.View()))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }
    }

    assert(prefix == DOMStringAtom::Null() || IsValidNamespacePrefix(prefix.View()));

    if (context == ValidateAndExtractContext::Attribute)
    {
      if (!IsValidAttributeLocalName(localName.View()))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }
    }
    else if (context == ValidateAndExtractContext::Element)
    {
      if (!IsValidElementLocalName(localName.View()))
      {
        return Exception {ExceptionCode::InvalidCharacterError};
      }
    }

    if (prefix != DOMStringAtom::Null() && namespaceURI == DOMStringAtom::Null())
    {
      return Exception {ExceptionCode::NamespaceError};
    }

    if (prefix == NamespacePrefix::XML && namespaceURI != Namespace::XML)
    {
      return Exception {ExceptionCode::NamespaceError};
    }

    if (prefix == NamespacePrefix::XMLNS && namespaceURI != Namespace::XMLNS)
    {
      return Exception {ExceptionCode::NamespaceError};
    }

    QualifiedName qualifiedNameStruct;
    qualifiedNameStruct.NamespaceURI = namespaceURI;
    qualifiedNameStruct.Prefix = prefix;
    qualifiedNameStruct.LocalName = DOMStringAtom {localName};

    return qualifiedNameStruct;
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

    if (!Krys::Text::IsASCIILower(name[0]))
    {
      return false;
    }

    if (std::ranges::any_of(name, [](char8 ch) { return Krys::Text::IsASCIIUpper(ch) || ch == '-'; }))
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

    // TODO(fix): We can't guarantee that the name is valid UTF-8, validate before converting.
    auto utf32 = ::Krys::Text::ConvertToUTF32(utf8_stringview {name});
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