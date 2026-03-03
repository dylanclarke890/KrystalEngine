#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode), TreeScope(*this)
  {
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

  Ref<HTMLCollection> Document::Children() const noexcept
  {
    return _documentRareData->Children(*this);
  }

  KRYS_NODISCARD RawPtr<Element> Document::FirstElementChild() const noexcept
  {
    return TreeTraversal::FirstElementChild(*this);
  }

  KRYS_NODISCARD RawPtr<Element> Document::LastElementChild() const noexcept
  {
    return TreeTraversal::LastElementChild(*this);
  }

  KRYS_NODISCARD size_t Document::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(IMPL): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }

#pragma endregion
}