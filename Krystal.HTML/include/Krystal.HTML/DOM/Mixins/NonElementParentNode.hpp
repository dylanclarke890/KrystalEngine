#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"

namespace Krys::HTML
{
  class Element;
  class ContainerNode;
}

namespace Krys::HTML::Mixins
{
  /// @see https://dom.spec.whatwg.org/#nonelementparentnode
  class NonElementParentNode
  {
  public:
    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    KRYS_NODISCARD static RefPtr<Element> GetElementById(ContainerNode &node, DOMStringView id) noexcept;

    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    KRYS_NODISCARD static RefPtr<const Element> GetElementById(const ContainerNode &node,
                                                               DOMStringView id) noexcept;
  };
}