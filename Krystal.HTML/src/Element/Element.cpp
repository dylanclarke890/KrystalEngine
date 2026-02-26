#include "Krystal.HTML/Element/Element.hpp"
#include "Krystal.HTML/Document/ShadowRoot.hpp"
#include "Krystal.HTML/Tree/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  Element::Element(Document &document, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags | NodeFlag::IsElement)
  {
  }

  DOMString Element::NodeName() const noexcept
  {
    // TODO(IMPL): Return the qualified name
    return u8"element";
  }

  DOMString Element::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::RemoveAttributeNode(Attr &attribute) const noexcept
  {
    // TODO(IMPL): Implement this method
    return {};
  }

#pragma region ChildNode

  ExceptionOr<void> Element::Before(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> Element::After(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> Element::ReplaceWith(SmallList<NodeOrString> &&nodes) noexcept
  {
    return ExceptionOr<void>();
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    if (auto parent = ShareRefPtr(ParentNode()))
    {
      return TreeMutationAlgorithms::Remove(*this, *parent, SuppressObservers(false));
    }

    return {};
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RawPtr<Element> Element::PreviousElementSibling() const noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<Element> Element::NextElementSibling() const noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

#pragma endregion
}