#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo::dom
{
  class Document;
  class Element;

  using CustomElementRegistryOrDefault = Maybe<RefPtr<html::CustomElementRegistry>>;
  constexpr inline auto DefaultCustomElementRegistry = null;

  class ElementFactory
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-create-element
    KRYS_NODISCARD static Ref<Element>
      Create(Document &document, const QualifiedName &name, DOMStringAtom is = DOMStringAtom::Null(),
             bool synchronousCustomElements = false,
             CustomElementRegistryOrDefault registry = DefaultCustomElementRegistry) noexcept;
  };
}