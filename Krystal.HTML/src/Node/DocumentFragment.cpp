#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"

namespace Krys::HTML
{
  DocumentFragment::DocumentFragment(Document &document, NodeFlag flags) noexcept
      : ContainerNode(document, NodeType::DOCUMENT_FRAGMENT_NODE, flags)
  {
  }

#pragma region Node

  DOMString DocumentFragment::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

  ExceptionOr<void> DocumentFragment::SetTextContent(DOMString &&value) noexcept
  {
    return {};
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> DocumentFragment::Children() noexcept
  {
    return _documentFragmentRareData->Children(*this);
  }

  RefPtr<const Element> DocumentFragment::FirstElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<Element> DocumentFragment::FirstElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<const Element> DocumentFragment::LastElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  RefPtr<Element> DocumentFragment::LastElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  size_t DocumentFragment::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> DocumentFragment::Prepend(const List<NodeOrString> &nodes) noexcept
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

  ExceptionOr<void> DocumentFragment::Append(const List<NodeOrString> &nodes) noexcept
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

  ExceptionOr<void> DocumentFragment::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    auto node = MutationAlgorithms::ConvertNodesIntoNode(nodes, NodeDocument());
    if (node.HasException())
    {
      return node.ReleaseException();
    }

    if (auto result = MutationAlgorithms::EnsurePreInsertValidity(*node.Value(), *this, nullptr);
        result.HasException())
    {
      return result.ReleaseException();
    }

    return MutationAlgorithms::ReplaceAll(node.Value().get(), *this);
  }

  ExceptionOr<void> DocumentFragment::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    if (refChild == &node)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, *this, refChild);
  }

  ExceptionOr<RefPtr<Element>> DocumentFragment::QuerySelector(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> DocumentFragment::QuerySelectorAll(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> DocumentFragment::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(impl): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }

#pragma endregion
}