#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"

namespace Krys::HTML
{
  class HTMLStackItem
  {
  private:
    RefPtr<ContainerNode> _node;

  public:
    HTMLStackItem() = default;

    // Normal HTMLElementStack and HTMLFormattingElementList items.
    HTMLStackItem(Ref<Element> && element, HTMLTokenAtom &&)
    {
    }
    HTMLStackItem(Ref<Element> &&, List<Attribute> &&);

    // Document fragment or element for parsing context.
    explicit HTMLStackItem(Element &);
    explicit HTMLStackItem(DocumentFragment &);
  };
}