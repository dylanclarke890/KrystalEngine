#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/Enums/CustomElementState.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
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
      CreateElement(Document &document, const QualifiedName &name, DOMStringAtom is = DOMStringAtom::Null(),
                    bool synchronousCustomElements = false,
                    CustomElementRegistryOrDefault registry = DefaultCustomElementRegistry) noexcept;

    /// @see https://dom.spec.whatwg.org/#create-an-element-internal
    KRYS_NODISCARD static Ref<Element>
      CreateElementInternal(Document &document, const QualifiedName &name, CustomElementState state,
                            DOMStringAtom is, RefPtr<CustomElementRegistry> &&registry) noexcept;
  };
}