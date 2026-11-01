#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
  class HTMLElement;
}

namespace krys::boo::dom
{
  class Document;
  class Element;

  using CustomElementRegistryOrDefault = Maybe<RefPtr<html::CustomElementRegistry>>;
  constexpr auto DefaultCustomElementRegistry = null;

  class ElementFactory
  {
  public:
    /// @brief Creates an element of the interface its namespace and local name give it.
    /// @see https://dom.spec.whatwg.org/#concept-create-element
    /// @see https://dom.spec.whatwg.org/#create-an-element-internal
    /// @see https://dom.spec.whatwg.org/#concept-element-interface
    KRYS_NODISCARD static Ref<Element>
      Create(Document &document, const QualifiedName &name, DOMStringAtom is = DOMStringAtom::Null(),
             bool synchronousCustomElements = false,
             CustomElementRegistryOrDefault registry = DefaultCustomElementRegistry) noexcept;

    /// @brief Creates the HTML element @p localName, whose interface is @p T, with its namespace and local
    /// name set as creating an element sets them.
    /// @see https://dom.spec.whatwg.org/#concept-create-element
    template <typename T>
    KRYS_NODISCARD static Ref<T> CreateHTMLElement(Document &document, DOMStringAtom localName) noexcept
    {
      auto element =
        Create(document, QualifiedName {infra::Namespaces::HTML, DOMStringAtom::Null(), localName});
      return ShareRef(Downcast<T>(Downcast<html::HTMLElement>(*element)));
    }
  };
}