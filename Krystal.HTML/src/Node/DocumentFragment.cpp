#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.HTML/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/DOM/Mixins/ParentNode.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  DocumentFragment::DocumentFragment(Document &document, NodeFlags flags) noexcept
      : ContainerNode(document, NodeType::DOCUMENT_FRAGMENT_NODE, flags)
  {
  }

  DocumentFragment::DocumentFragment(Document &document) noexcept
      : ContainerNode(document, NodeType::DOCUMENT_FRAGMENT_NODE, NodeFlags::None)
  {
  }

#pragma region Node

  Maybe<DOMString> DocumentFragment::TextContent() const noexcept
  {
    return TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> DocumentFragment::TextContent(DOMString &&value) noexcept
  {
    return NodeAlgorithms::StringReplaceAll(Krys::Move(value), *this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> DocumentFragment::Children() noexcept
  {
    return _documentFragmentRareData->Children(*this);
  }

  RefPtr<const Element> DocumentFragment::FirstElementChild() const noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> DocumentFragment::FirstElementChild() noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> DocumentFragment::LastElementChild() const noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> DocumentFragment::LastElementChild() noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  size_t DocumentFragment::ChildElementCount() const noexcept
  {
    return Mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> DocumentFragment::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::Append(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return Mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> DocumentFragment::QuerySelector(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> DocumentFragment::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> DocumentFragment::GetElementById(DOMStringView elementId) noexcept
  {
    return Mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

  RefPtr<const Element> DocumentFragment::GetElementById(DOMStringView elementId) const noexcept
  {
    return Mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

#pragma endregion
}