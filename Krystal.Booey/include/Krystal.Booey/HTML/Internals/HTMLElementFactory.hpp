#pragma once

#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/Types/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class Document;
  class HTMLElement;
}

namespace krys::boo::html
{
  class CustomElementRegistry;

  class HTMLElementFactory
  {
  public:
    /// @brief An element of the element interface for @p elementName in the HTML namespace, or null if it
    /// has none (a custom element name, or an unknown element).
    /// @see https://html.spec.whatwg.org/multipage/dom.html#referenced
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(Document &document, TagName elementName) noexcept;
  };
}
