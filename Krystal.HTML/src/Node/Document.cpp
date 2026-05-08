#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/Algorithms/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document() noexcept : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode)
  {
    SetEventTargetFlag(EventTargetFlag::IsConnected);
  }

#pragma region Document

  ExceptionOr<Ref<Node>> Document::AdoptNode(Node &node) noexcept
  {
    if (node.NodeType() == NodeType::DOCUMENT_NODE)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (node.IsShadowRootNode()) // ShadowRoot cannot disconnect itself from the host node.
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (node.NodeType() == NodeType::ATTRIBUTE_NODE)
    {
      auto &attr = Downcast<Attr>(node);
      RefPtr<Element> element = ShareRefPtr(attr.OwnerElement());
      if (element != nullptr)
      {
        if (auto result = element->RemoveAttributeNode(attr); result.HasException())
        {
          return result.ReleaseException();
        }
      }
    }
    else
    {
      if (auto oldParent = ShareRefPtr(node.ParentNode()))
      {
        if (auto result = oldParent->RemoveChild(node); result.HasException())
        {
          return {result.ReleaseException()};
        }
      }

      assert(!node.IsConnected());
      assert(!node.ParentNode());
    }

    return ShareRef<Node>(node);
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Document::Children() noexcept
  {
    return _documentRareData->Children(*this);
  }

  RefPtr<const Element> Document::FirstElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<Element> Document::FirstElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::FirstElementChild(*this));
  }

  RefPtr<const Element> Document::LastElementChild() const noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  RefPtr<Element> Document::LastElementChild() noexcept
  {
    return ShareRefPtr(TreeTraversal::LastElementChild(*this));
  }

  size_t Document::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

  ExceptionOr<void> Document::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Document::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return Mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Document::QuerySelector(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Document::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(DOMStringView elementId) noexcept
  {
    return ShareRefPtr(Mixins::NonElementParentNode::GetElementById(*this, elementId));
  }

  RefPtr<const Element> Document::GetElementById(DOMStringView elementId) const noexcept
  {
    return ShareRefPtr(Mixins::NonElementParentNode::GetElementById(*this, elementId));
  }

#pragma endregion
}