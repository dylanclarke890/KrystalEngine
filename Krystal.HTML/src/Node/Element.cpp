#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/ChildNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SlotAssignmentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/ElementRareData.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"

namespace Krys::HTML
{
  Element::Element(Document &document, NodeFlag nodeFlags) noexcept
      : ContainerNode(document, NodeType::ELEMENT_NODE, nodeFlags | NodeFlag::IsElement)
  {
  }

#pragma region Element

  ExceptionOr<void> Element::RemoveAttributeNode(Attr &attribute) const noexcept
  {
    // TODO(IMPL): Implement this method
    return {};
  }

#pragma endregion

#pragma region Node

  DOMString Element::NodeName() const noexcept
  {
    // TODO(IMPL): Return the qualified name
    return u8"element";
  }

  DOMString Element::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

  ExceptionOr<void> Element::SetTextContent(DOMString &&value) noexcept
  {
    return ExceptionOr<void>();
  }

#pragma endregion

#pragma region ChildNode

  ExceptionOr<void> Element::Before(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::Before(*this, nodes);
  }

  ExceptionOr<void> Element::After(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::After(*this, nodes);
  }

  ExceptionOr<void> Element::ReplaceWith(const List<NodeOrString> &nodes) noexcept
  {
    return ChildNodeAlgorithms::ReplaceWith(*this, nodes);
  }

  ExceptionOr<void> Element::Remove() noexcept
  {
    return ChildNodeAlgorithms::Remove(*this);
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RefPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::PreviousElementSibling(*this));
  }

  RefPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

  RefPtr<Element> Element::NextElementSibling() noexcept
  {
    return ShareRefPtr(TreeTraversal::NextElementSibling(*this));
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return _elementRareData->Children(*this);
  }

  RefPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<Element> Element::FirstElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<const Element> Element::LastElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  RefPtr<Element> Element::LastElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::PreInsert(*node.Value(), *this, FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::Append(*node.Value(), *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::ReplaceAll(node.Value().get(), *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    if (&node == refChild)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, *this, refChild);
  }

  ExceptionOr<RefPtr<Element>> Element::QuerySelector(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion

#pragma region Slottable

  RawPtr<HTMLSlotElement> Element::AssignedSlot() noexcept
  {
    return SlotAssignmentAlgorithms::FindSlot(*this, true);
  }

#pragma endregion
}