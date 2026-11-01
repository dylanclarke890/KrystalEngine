#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/Enums/TagName.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class HTMLElement;

  class HTMLElementFactory
  {
  public:
    /// @brief An element of the element interface for @p elementName in the HTML namespace, or null if it
    /// has none (a custom element name, or an unknown element).
    /// @see https://html.spec.whatwg.org/#referenced
    KRYS_NODISCARD static RefPtr<HTMLElement> TryCreate(dom::Document &document,
                                                        TagName elementName) noexcept;
  };
}