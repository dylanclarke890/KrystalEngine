#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief An element name in a sanitizer configuration. A string is an element in the HTML namespace.
  /// @note The IDL's `(DOMString or SanitizerElementNamespace)` is this dictionary, made from a string as
  /// canonicalizing one would.
  /// @see https://html.spec.whatwg.org/#dictdef-sanitizerelementnamespace
  struct SanitizerElementNamespace
  {
    dom::DOMString Name;
    /// @brief The namespace; null (`DOMStringAtom::Null()`) for no namespace.
    dom::DOMStringAtom Namespace;

    SanitizerElementNamespace(dom::DOMString name, dom::DOMStringAtom namespaceURI =
                                                     dom::DOMStringAtom(infra::Namespaces::HTML)) noexcept
        : Name(krys::move(name)), Namespace(namespaceURI)
    {
    }

    SanitizerElementNamespace(const char8_t *name) noexcept : SanitizerElementNamespace(dom::DOMString(name))
    {
    }

    /// @brief Whether the two name the same element (the equality of their name and namespace members).
    KRYS_NODISCARD bool HasSameName(const SanitizerElementNamespace &other) const noexcept
    {
      return Name == other.Name && Namespace == other.Namespace;
    }
  };

  /// @brief An attribute name in a sanitizer configuration. A string is an attribute in no namespace.
  /// @see https://html.spec.whatwg.org/#dictdef-sanitizerattributenamespace
  struct SanitizerAttributeNamespace
  {
    dom::DOMString Name;
    /// @brief The namespace; null (`DOMStringAtom::Null()`) for no namespace.
    dom::DOMStringAtom Namespace;

    SanitizerAttributeNamespace(dom::DOMString name,
                                dom::DOMStringAtom namespaceURI = dom::DOMStringAtom::Null()) noexcept
        : Name(krys::move(name)), Namespace(namespaceURI)
    {
    }

    SanitizerAttributeNamespace(const char8_t *name) noexcept
        : SanitizerAttributeNamespace(dom::DOMString(name))
    {
    }

    KRYS_NODISCARD bool operator==(const SanitizerAttributeNamespace &) const noexcept = default;
  };

  /// @brief An element allowed by a sanitizer configuration, with the attributes allowed or removed on it.
  /// @see https://html.spec.whatwg.org/#dictdef-sanitizerelementnamespacewithattributes
  struct SanitizerElementNamespaceWithAttributes : SanitizerElementNamespace
  {
    Maybe<List<SanitizerAttributeNamespace>> Attributes;
    Maybe<List<SanitizerAttributeNamespace>> RemoveAttributes;

    using SanitizerElementNamespace::SanitizerElementNamespace;

    SanitizerElementNamespaceWithAttributes(const SanitizerElementNamespace &name) noexcept
        : SanitizerElementNamespace(name)
    {
    }

    /// @brief The equality of the dictionaries: the same name, namespace and attribute lists.
    KRYS_NODISCARD bool operator==(const SanitizerElementNamespaceWithAttributes &other) const noexcept
    {
      return HasSameName(other) && Attributes == other.Attributes
             && RemoveAttributes == other.RemoveAttributes;
    }
  };

  /// @brief A processing instruction target in a sanitizer configuration.
  /// @see https://html.spec.whatwg.org/#dictdef-sanitizerprocessinginstruction
  struct SanitizerProcessingInstruction
  {
    dom::DOMString Target;

    SanitizerProcessingInstruction(dom::DOMString target) noexcept : Target(krys::move(target))
    {
    }

    SanitizerProcessingInstruction(const char8_t *target) noexcept : Target(target)
    {
    }

    KRYS_NODISCARD bool operator==(const SanitizerProcessingInstruction &) const noexcept = default;
  };

  /// @brief Which elements, attributes, comments and processing instructions sanitizing keeps. A member that
  /// isn't given is null.
  /// @see https://html.spec.whatwg.org/#dictdef-sanitizerconfig
  struct SanitizerConfig
  {
    Maybe<List<SanitizerElementNamespaceWithAttributes>> Elements;
    Maybe<List<SanitizerElementNamespace>> RemoveElements;
    Maybe<List<SanitizerElementNamespace>> ReplaceWithChildrenElements;
    Maybe<List<SanitizerProcessingInstruction>> ProcessingInstructions;
    Maybe<List<SanitizerProcessingInstruction>> RemoveProcessingInstructions;
    Maybe<List<SanitizerAttributeNamespace>> Attributes;
    Maybe<List<SanitizerAttributeNamespace>> RemoveAttributes;
    Maybe<bool> Comments;
    Maybe<bool> DataAttributes;
    Maybe<bool> JavascriptURLs;
  };

  /// @see https://html.spec.whatwg.org/#enumdef-sanitizerpresets
  enum class SanitizerPresets : uint8
  {
    Default,
  };
}
