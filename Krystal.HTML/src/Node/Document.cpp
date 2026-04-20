#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode), TreeScope(*this)
  {
    SetEventTargetFlag(EventTargetFlag::IsConnected);
  }

#pragma region Document

  ExceptionOr<Ref<Node>> Document::AdoptNode(Node &node) noexcept
  {
    if (node.GetNodeType() == NodeType::DOCUMENT_NODE)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (node.IsShadowRootNode()) // ShadowRoot cannot disconnect itself from the host node.
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (node.GetNodeType() == NodeType::ATTRIBUTE_NODE)
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

    node.SetTreeScopeRecursively(*this);

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

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
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

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
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

  ExceptionOr<void> Document::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    if (refChild == &node)
    {
      refChild = node.NextSibling();
    }

    return MutationAlgorithms::Move(node, *this, refChild);
  }

  ExceptionOr<RefPtr<Element>> Document::QuerySelector(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

  ExceptionOr<Ref<NodeList>> Document::QuerySelectorAll(const DOMString &selectors) noexcept
  {
    // TODO(impl): implement this when we have css parsing.
    return Exception {ExceptionCode::NotSupportedError};
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(impl): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }

#pragma endregion
}