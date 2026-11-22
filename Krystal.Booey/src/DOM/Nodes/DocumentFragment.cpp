#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"

namespace krys::boo
{
  DocumentFragment::DocumentFragment(Document &document, dom::NodeFlags flags) noexcept
      : ContainerNode(document, dom::NodeType::DOCUMENT_FRAGMENT_NODE, flags)
  {
  }

  Element *DocumentFragment::Host() const noexcept
  {
    if (auto host = _host.lock())
    {
      return host.get();
    }

    return nullptr;
  }

  void DocumentFragment::SetHost(Element &host) noexcept
  {
    _host = CreateWeakPtr(&host);
  }

  DocumentFragment::DocumentFragment(Document &document) noexcept
      : ContainerNode(document, dom::NodeType::DOCUMENT_FRAGMENT_NODE, {})
  {
  }

#pragma region Node

  Maybe<DOMString> DocumentFragment::TextContent() const noexcept
  {
    return dom::TextAlgorithms::DescendantTextContent(*this);
  }

  ExceptionOr<void> DocumentFragment::TextContent(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::NodeAlgorithms::StringReplaceAll(krys::move(value), *this);
  }

#pragma endregion

#pragma region ParentNode

  Ref<dom::HTMLCollection> DocumentFragment::Children() noexcept
  {
    return EnsureNodeRareData().Children(*this);
  }

  RefPtr<const Element> DocumentFragment::FirstElementChild() const noexcept
  {
    return dom::mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> DocumentFragment::FirstElementChild() noexcept
  {
    return dom::mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> DocumentFragment::LastElementChild() const noexcept
  {
    return dom::mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> DocumentFragment::LastElementChild() noexcept
  {
    return dom::mixins::ParentNode::LastElementChild(*this);
  }

  size_t DocumentFragment::ChildElementCount() const noexcept
  {
    return dom::mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> DocumentFragment::Prepend(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::Append(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::ReplaceChildren(const List<dom::NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> DocumentFragment::MoveBefore(Node &node, Node *refChild) noexcept
  {
    html::CEReactionsScope ceReactions;
    return dom::mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> DocumentFragment::QuerySelector(DOMStringView selectors) noexcept
  {
    return dom::mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<dom::NodeList>> DocumentFragment::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return dom::mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> DocumentFragment::GetElementById(DOMStringView elementId) noexcept
  {
    return dom::mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

  RefPtr<const Element> DocumentFragment::GetElementById(DOMStringView elementId) const noexcept
  {
    return dom::mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

#pragma endregion
}
