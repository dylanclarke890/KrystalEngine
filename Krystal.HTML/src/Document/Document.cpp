#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Document/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode), TreeScope(*this)
  {
  }

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

  DOMString Document::NodeName() const noexcept
  {
    return u8"#document";
  }

  RefPtr<Element> Document::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(IMPL): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }
}