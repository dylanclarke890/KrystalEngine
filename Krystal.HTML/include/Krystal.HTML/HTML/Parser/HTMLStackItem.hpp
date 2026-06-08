#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Enums/ElementName.hpp"
#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"

namespace Krys::HTML
{
  class HTMLStackItem
  {
  private:
    ElementName _name {ElementName::Unknown};
    Namespace _namespace {Namespace::Unknown};
    RefPtr<ContainerNode> _node;
    List<IntermediaryAttribute> _attributes;

  public:
    HTMLStackItem() = default;

    // Document fragment or element for parsing context.
    explicit HTMLStackItem(Element &element) noexcept : _node(ShareRef(element))
    {
    }

    explicit HTMLStackItem(DocumentFragment &fragment) noexcept : _node(ShareRef(fragment))
    {
    }

    // Normal HTMLElementStack and HTMLFormattingElementList items.
    HTMLStackItem(Ref<Element> &&element, HTMLTokenAtom &&token) noexcept
        : _node(Krys::Move(element)), _attributes(Krys::Move(token.Attributes()))
    {
    }

    HTMLStackItem(Ref<Element> &&element, List<IntermediaryAttribute> &&attributes) noexcept
        : _node(Krys::Move(element)), _attributes(Krys::Move(attributes))
    {
    }

    KRYS_NODISCARD bool IsElement() const noexcept
    {
      return Is<Element>(_node.get());
    }

    KRYS_NODISCARD bool IsFragment() const noexcept
    {
      return Is<DocumentFragment>(_node.get());
    }

    KRYS_NODISCARD const List<IntermediaryAttribute> &Attributes() const noexcept
    {
      return _attributes;
    }

    KRYS_NODISCARD ContainerNode &Node() const noexcept
    {
      assert(_node);
      return *_node;
    }

    KRYS_NODISCARD ElementName Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD Namespace Namespace() const noexcept
    {
      return _namespace;
    }
  };
}