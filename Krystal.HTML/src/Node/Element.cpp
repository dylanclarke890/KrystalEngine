#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/TreeMutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
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
      return TreeMutationAlgorithms::Remove(*this, SuppressObservers(false));
    }

    return {};
  }

#pragma endregion

#pragma region NonDocumentTypeChildNode

  RawPtr<const Element> Element::PreviousElementSibling() const noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<Element> Element::PreviousElementSibling() noexcept
  {
    return TreeTraversal::PreviousElementSibling(*this);
  }

  RawPtr<const Element> Element::NextElementSibling() const noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

  RawPtr<Element> Element::NextElementSibling() noexcept
  {
    return TreeTraversal::NextElementSibling(*this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Element::Children() noexcept
  {
    return _elementRareData->Children(*this);
  }

  RawPtr<const Element> Element::FirstElementChild() const noexcept
  {
    return TreeTraversal::FirstElementChild(*this);
  }

  RawPtr<Element> Element::FirstElementChild() noexcept
  {
    return TreeTraversal::FirstElementChild(*this);
  }

  RawPtr<const Element> Element::LastElementChild() const noexcept
  {
    return TreeTraversal::LastElementChild(*this);
  }

  RawPtr<Element> Element::LastElementChild() noexcept
  {
    return TreeTraversal::LastElementChild(*this);
  }

  size_t Element::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> Element::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    auto node = TreeMutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = TreeMutationAlgorithms::PreInsert(*node.Value(), *this, FirstChild());
        result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::Append(const List<NodeOrString> &nodes) noexcept
  {
    auto node = TreeMutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = TreeMutationAlgorithms::Append(*node.Value(), *this); result.HasException())
    {
      return result.ReleaseException();
    }

    return {};
  }

  ExceptionOr<void> Element::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    auto node = TreeMutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = TreeMutationAlgorithms::PreInsert(*node.Value(), *this, nullptr); result.HasException())
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

    return TreeMutationAlgorithms::Move(node, *this, refChild);
  }

  ExceptionOr<RawPtr<Element>> Element::QuerySelector(const DOMString &selectors) noexcept
  {
    // TODO(impl): Implement this method
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> Element::QuerySelectorAll(const DOMString &selectors) noexcept
  {
    // TODO(impl): Implement this method
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion
}