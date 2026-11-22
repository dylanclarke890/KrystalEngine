#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class ContainerNode;
  class Element;
}

namespace krys::boo::dom::mixins
{
  /// @see https://dom.spec.whatwg.org/#nonelementparentnode
  class NonElementParentNode
  {
  public:
    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#get-an-element-by-id
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    KRYS_NODISCARD static RefPtr<Element> GetElementById(ContainerNode &node, DOMStringView id) noexcept;

    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#get-an-element-by-id
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    KRYS_NODISCARD static RefPtr<const Element> GetElementById(const ContainerNode &node,
                                                               DOMStringView id) noexcept;
  };
}
