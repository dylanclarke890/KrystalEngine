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
      // TODO(impl):
      return false;
    }

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#valid-custom-element-name
    KRYS_NODISCARD static bool IsValidCustomElementName(DOMStringView name) noexcept
    {
      // TODO(impl):
      return false;
    }
  };
}