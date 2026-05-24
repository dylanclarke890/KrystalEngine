#pragma once

#include "Krystal.HTML/Infra/StringAlgorithms.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML
{
  enum class ValidateAndExtractContext : uint8
  {
    Attribute,
    Element,
  };

  /// @see https://dom.spec.whatwg.org/#namespaces
  class NameValidation
  {
  public:
    /// @see https://dom.spec.whatwg.org/#valid-namespace-prefix
    KRYS_NODISCARD static bool IsValidNamespacePrefix(DOMStringView name) noexcept
    {
      if (name.empty())
      {
        return false;
      }

      for (const auto &ch : name)
      {
        if (::Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
        {
          return false;
        }
      }

      return true;
    }

    /// @see https://dom.spec.whatwg.org/#valid-attribute-local-name
    KRYS_NODISCARD static bool IsValidAttributeLocalName(DOMStringView name) noexcept
    {
      if (name.empty())
      {
        return false;
      }

      for (const auto &ch : name)
      {
        if (::Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '=' || ch == '>')
        {
          return false;
        }
      }

      return true;
    }

    /// @see https://dom.spec.whatwg.org/#valid-element-local-name
    KRYS_NODISCARD static bool IsValidElementLocalName(DOMStringView name) noexcept
    {
      if (name.empty())
      {
        return false;
      }

      if (::Krys::Text::IsASCIIAlpha(name[0]))
      {
        for (const auto &ch : name)
        {
          if (::Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '/' || ch == '>')
          {
            return false;
          }
        }

        return true;
      }

      auto utf32 = ::Krys::Text::ConvertToUTF32(utf8_stringview {name});
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
        if (!::Krys::Text::IsASCIIAlpha(ch) && !::Krys::Text::IsASCIIDigit(ch) && ch != '-' && ch != '.'
            && ch != ':' && ch != '_' && !(ch >= 0x0080 && ch <= 0x10FFF))
        {
          return false;
        }
      }

      return false;
    }

    /// @see https://dom.spec.whatwg.org/#valid-doctype-name
    KRYS_NODISCARD static bool IsValidDocTypeName(DOMStringView name) noexcept
    {
      for (const auto &ch : name)
      {
        if (::Krys::Text::IsASCIIWhitespace(ch) || ch == '\0' || ch == '>')
        {
          return false;
        }
      }

      return true;
    }

    /// @see https://dom.spec.whatwg.org/#validate-and-extract
    KRYS_NODISCARD static ExceptionOr<QualifiedName>
      ValidateAndExtract(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
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

      if (prefix == Namespaces::XMLPrefix && namespaceURI != Namespaces::XML)
      {
        return Exception {ExceptionCode::NamespaceError};
      }

      if (prefix == Namespaces::XMLNSPrefix && namespaceURI != Namespaces::XMLNS)
      {
        return Exception {ExceptionCode::NamespaceError};
      }

      QualifiedName qualifiedNameStruct;
      qualifiedNameStruct.NamespaceURI = namespaceURI;
      qualifiedNameStruct.Prefix = prefix;
      qualifiedNameStruct.LocalName = DOMStringAtom {localName};

      return qualifiedNameStruct;
    }

    /// @see https://dom.spec.whatwg.org/#valid-shadow-host-name
    KRYS_NODISCARD static bool IsValidShadowHostName(DOMStringView name) noexcept
    {
      if (IsValidCustomElementName(name))
      {
        return true;
      }

      static Array<DOMStringView, 18> validNames = {
        u8"article", u8"aside", u8"blockquote", u8"body", u8"div",     u8"footer",
        u8"h1",      u8"h2",    u8"h3",         u8"h4",   u8"h5",      u8"h6",
        u8"header",  u8"main",  u8"nav",        u8"p",    u8"section", u8"span"};

      if (std::ranges::contains(validNames, name))
      {
        return true;
      }

      return false;
    }

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#valid-custom-element-name
    KRYS_NODISCARD static bool IsValidCustomElementName(DOMStringView name) noexcept
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

    KRYS_NODISCARD static bool IsXMLNameStartChar(char32 ch) noexcept
    {
      return ch == ':' || ch == '_' || ::Krys::Text::IsASCIIAlpha(ch) || (ch >= 0xC0 && ch <= 0xD6)
             || (ch >= 0xD8 && ch <= 0xF6) || (ch >= 0xF8 && ch <= 0x2FF) || (ch >= 0x370 && ch <= 0x37D)
             || (ch >= 0x37F && ch <= 0x1FFF) || (ch >= 0x200C && ch <= 0x200D)
             || (ch >= 0x2070 && ch <= 0x218F) || (ch >= 0x2C00 && ch <= 0x2FEF)
             || (ch >= 0x3001 && ch <= 0xD7FF) || (ch >= 0xF900 && ch <= 0xFDCF)
             || (ch >= 0xFDF0 && ch <= 0xFFFD) || (ch >= 0x10000 && ch <= 0xEFFFF);
    }

    KRYS_NODISCARD static bool IsXMLNameChar(char32 ch) noexcept
    {
      return IsXMLNameStartChar(ch) || ch == '-' || ch == '.' || ::Krys::Text::IsASCIIDigit(ch) || ch == 0xB7
             || (ch >= 0x0300 && ch <= 0x036F) || (ch >= 0x203F && ch <= 0x2040);
    }

    /// @see https://www.w3.org/TR/xml/#NT-Name
    KRYS_NODISCARD static bool IsValidXMLName(DOMStringView name) noexcept
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
  };
}