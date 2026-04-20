#pragma once

#include "Krystal.HTML/Algorithms/StringAlgorithms.hpp"
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Namespaces.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Text/ASCII.hpp"
#include "Krystal.Text/StringConversion.hpp"

namespace Krys::HTML
{
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
    KRYS_NODISCARD static ExceptionOr<QualifiedName> ValidateAndExtract(const DOMString &namespaceUri,
                                                                        const DOMString &qualifiedName,
                                                                        const DOMString &context) noexcept
    {
      Maybe<DOMString> namespaceURI;
      if (!namespaceUri.empty())
      {
        namespaceURI = namespaceUri;
      }

      Maybe<DOMString> prefix;

      DOMString localName = qualifiedName;
      if (qualifiedName.contains(':'))
      {
        auto splitResult = StringAlgorithms::StrictlySplit(qualifiedName, ':');
        prefix = splitResult[0];
        localName = splitResult[1];

        if (!IsValidNamespacePrefix(prefix.value()))
        {
          return Exception {ExceptionCode::InvalidCharacterError};
        }
      }

      assert(!prefix.has_value() || IsValidNamespacePrefix(prefix.value()));

      // TODO(FIX, PERF): use an enum for context instead of string comparison.
      if (context == u8"attribute")
      {
        if (!IsValidAttributeLocalName(localName))
        {
          return Exception {ExceptionCode::InvalidCharacterError};
        }
      }
      else if (context == u8"element")
      {
        if (!IsValidElementLocalName(localName))
        {
          return Exception {ExceptionCode::InvalidCharacterError};
        }
      }

      if (prefix.has_value() && !namespaceURI.has_value())
      {
        return Exception {ExceptionCode::NamespaceError};
      }

      if (prefix == u8"xml" && namespaceURI != ::Krys::HTML::Namespaces::XML)
      {
        return Exception {ExceptionCode::NamespaceError};
      }

      if (prefix == u8"xmlns" && namespaceURI != ::Krys::HTML::Namespaces::XMLNS)
      {
        return Exception {ExceptionCode::NamespaceError};
      }

      QualifiedName qualifiedNameStruct;
      qualifiedNameStruct.NamespaceURI = DOMStringAtom {namespaceURI.value_or(u8"")};
      qualifiedNameStruct.Prefix = DOMStringAtom {prefix.value_or(u8"")};
      qualifiedNameStruct.LocalName = DOMStringAtom {localName};

      return qualifiedNameStruct;
    }
  };
}