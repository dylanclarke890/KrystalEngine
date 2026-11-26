#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/QualifiedName.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Infra/Namespaces.hpp"

namespace krys::web
{
  class Document;
  class Element;
  class HTMLElement;
}

namespace krys::web::html
{
  class CustomElementRegistry;
}

namespace krys::web::dom
{
  /// @brief A registry (null for none), or "default" when empty. A plain pointer: creating an element only
  /// reads it, and the element takes its own reference.
  using CustomElementRegistryOrDefault = Maybe<html::CustomElementRegistry *>;
  constexpr auto DefaultCustomElementRegistry = null;

  class ElementFactory
  {
  public:
    /// @brief Creates an element of the interface its namespace and local name give it.
    /// @see https://dom.spec.whatwg.org/#concept-create-element
    /// @see https://dom.spec.whatwg.org/#create-an-element-internal
    /// @see https://dom.spec.whatwg.org/#concept-element-interface
    KRYS_NODISCARD static Ref<Element>
      Create(Document &document, const QualifiedName &name, DOMStringAtom is = null,
             bool synchronousCustomElements = false,
             CustomElementRegistryOrDefault registry = DefaultCustomElementRegistry) noexcept;

    /// @brief Creates the HTML element @p localName, whose interface is @p T, with its namespace and local
    /// name set as creating an element sets them.
    /// @see https://dom.spec.whatwg.org/#concept-create-element
    template <typename T>
    KRYS_NODISCARD static Ref<T> CreateHTMLElement(Document &document, DOMStringAtom localName) noexcept
    {
      auto element = Create(document, QualifiedName {infra::namespaces::HTML, null, krys::move(localName)});
      return ShareRef(Downcast<T>(Downcast<HTMLElement>(*element)));
    }
  };
}
