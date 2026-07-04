#pragma once

#include "Krystal.HTML/DOM/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/HTML/Enums/Namespace.hpp"
#include "Krystal.HTML/HTML/Enums/TagName.hpp"
#include "Krystal.HTML/HTML/Parser/HTMLTokenAtom.hpp"

namespace Krys::HTML
{
  class HTMLStackItem
  {
  private:
    TagName _name {TagName::Unknown};
    Namespace _namespace {Namespace::Unknown};
    RefPtr<ContainerNode> _node;
    ParsedAttributeList _attributes;

  public:
    HTMLStackItem() = default;

#pragma region HTMLElementStack/ActiveFormattingList Constructors

    HTMLStackItem(TagName tagName, Namespace tagNamespace, Element &element,
                  ParsedAttributeList &&attributes) noexcept
        : _name(tagName), _namespace(tagNamespace), _node(ShareRef(element)),
          _attributes(Krys::Move(attributes))
    {
    }

    HTMLStackItem(TagName tagName, Namespace tagNamespace, Element &element,
                  const ParsedAttributeList &attributes) noexcept
        : _name(tagName), _namespace(tagNamespace), _node(ShareRef(element)), _attributes(attributes)
    {
    }

#pragma endregion

#pragma region Fragment Constructors

    HTMLStackItem(Element &element) noexcept
        : _name(element._qualifiedName.TagName()), _namespace(element._qualifiedName.Namespace()),
          _node(ShareRefPtr(&element))
    {
    }

    HTMLStackItem(DocumentFragment &fragment) noexcept : _node(ShareRefPtr(&fragment))
    {
    }

#pragma endregion

    KRYS_NODISCARD bool IsElement() const noexcept
    {
      return Is<Element>(_node.get());
    }

    KRYS_NODISCARD bool IsDocumentFragment() const noexcept
    {
      return Is<DocumentFragment>(_node.get());
    }

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return _node == nullptr;
    }

    KRYS_NODISCARD ContainerNode &Node() const noexcept
    {
      assert(_node);
      return *_node;
    }

    KRYS_NODISCARD Element &AsElement() const noexcept
    {
      assert(IsElement());
      return Downcast<Element>(*_node);
    }

    KRYS_NODISCARD DocumentFragment &AsDocumentFragment() const noexcept
    {
      assert(IsDocumentFragment());
      return Downcast<DocumentFragment>(*_node);
    }

    KRYS_NODISCARD TagName TagName() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD Namespace Namespace() const noexcept
    {
      return _namespace;
    }

    KRYS_NODISCARD const ParsedAttributeList &Attributes() const noexcept
    {
      assert(IsElement());
      return _attributes;
    }

    /// @brief Replaces the DOM element reference stored in this item while keeping the tag metadata.
    void UpdateElement(Element &newElement) noexcept
    {
      _node = ShareRef(newElement);
    }
  };
}