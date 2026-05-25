#pragma once

#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.HTML/Types/DOMString.hpp"
#include "Krystal.HTML/Types/DOMStringAtom.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

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
#pragma region Section 1.4 - https://dom.spec.whatwg.org/#namespaces

    /// @see https://dom.spec.whatwg.org/#valid-namespace-prefix
    KRYS_NODISCARD static bool IsValidNamespacePrefix(DOMStringView name) noexcept;

    /// @see https://dom.spec.whatwg.org/#valid-attribute-local-name
    KRYS_NODISCARD static bool IsValidAttributeLocalName(DOMStringView name) noexcept;

    /// @see https://dom.spec.whatwg.org/#valid-element-local-name
    KRYS_NODISCARD static bool IsValidElementLocalName(DOMStringView name) noexcept;

    /// @see https://dom.spec.whatwg.org/#valid-doctype-name
    KRYS_NODISCARD static bool IsValidDocTypeName(DOMStringView name) noexcept;

    /// @see https://dom.spec.whatwg.org/#validate-and-extract
    KRYS_NODISCARD static ExceptionOr<QualifiedName>
      ValidateAndExtract(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                         ValidateAndExtractContext context) noexcept;

#pragma endregion

    /// @see https://dom.spec.whatwg.org/#valid-shadow-host-name
    KRYS_NODISCARD static bool IsValidShadowHostName(DOMStringView name) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#valid-custom-element-name
    KRYS_NODISCARD static bool IsValidCustomElementName(DOMStringView name) noexcept;

    /// @see https://www.w3.org/TR/xml/#NT-Name
    KRYS_NODISCARD static bool IsValidXMLName(DOMStringView name) noexcept;
  };
}