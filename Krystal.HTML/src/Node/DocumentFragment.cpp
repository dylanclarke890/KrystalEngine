#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NonElementParentNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/ParentNodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

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
    return MutationAlgorithms::StringReplaceAll(Krys::Move(value), *this);
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
    return ParentNodeAlgorithms::Prepend(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::Append(const List<NodeOrString> &nodes) noexcept
  {
    return ParentNodeAlgorithms::Append(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return ParentNodeAlgorithms::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return ParentNodeAlgorithms::MoveBefore(*this, node, refChild);
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

  RefPtr<Element> DocumentFragment::GetElementById(DOMStringView elementId) noexcept
  {
    return ShareRefPtr(NonElementParentNodeAlgorithms::GetElementById(*this, elementId));
  }

  RefPtr<const Element> DocumentFragment::GetElementById(DOMStringView elementId) const noexcept
  {
    return ShareRefPtr(NonElementParentNodeAlgorithms::GetElementById(*this, elementId));
  }

#pragma endregion
}