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
    List<IntermediaryAttribute> _attributes;

  public:
    HTMLStackItem() = default;

    // Normal HTMLElementStack and HTMLFormattingElementList items.
    HTMLStackItem(Ref<Element> &&element, HTMLTokenAtom &&token) noexcept
        : _node(Krys::Move(element)), _attributes(Krys::Move(token.Attributes()))
    {
    }

    HTMLStackItem(Ref<Element> &&element, List<IntermediaryAttribute> &&attributes) noexcept
        : _node(Krys::Move(element)), _attributes(Krys::Move(attributes))
    {
    }

    // Document fragment or element for parsing context.
    explicit HTMLStackItem(Element &element) noexcept : _node(ShareRef(element))
    {
    }

    explicit HTMLStackItem(DocumentFragment &fragment) noexcept : _node(ShareRef(fragment))
    {
    }

    KRYS_NODISCARD ContainerNode &Node() const noexcept
    {
      assert(_node);
      return *_node;
    }
  };
}