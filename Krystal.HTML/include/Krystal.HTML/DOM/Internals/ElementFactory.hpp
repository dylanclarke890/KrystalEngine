#pragma once

#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class CustomElementRegistry;
  class Document;
  class Element;

  using CustomElementRegistryOrDefault = Maybe<RefPtr<CustomElementRegistry>>;
  constexpr inline auto DefaultCustomElementRegistry = Null;

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