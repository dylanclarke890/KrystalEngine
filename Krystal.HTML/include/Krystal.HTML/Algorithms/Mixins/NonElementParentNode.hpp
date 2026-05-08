#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class Element;
  class ContainerNode;
}

namespace Krys::HTML::Mixins
{
  class NonElementParentNode
  {
  public:
    KRYS_NODISCARD static RawPtr<Element> GetElementById(ContainerNode &node, DOMStringView id) noexcept;
    KRYS_NODISCARD static RawPtr<const Element> GetElementById(const ContainerNode &node,
                                                               DOMStringView id) noexcept;
  };
}