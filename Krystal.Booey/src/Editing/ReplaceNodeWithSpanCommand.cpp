#include "Krystal.Booey/Editing/ReplaceNodeWithSpanCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  namespace
  {
    void SwapInNodePreservingAttributesAndChildren(dom::Element &newNode,
                                                   dom::Element &nodeToReplace) noexcept
    {
      krys_debug_assert(nodeToReplace.IsConnected());
      RefPtr<dom::ContainerNode> parentNode = ShareRefPtr(nodeToReplace.ParentNode());

      // FIXME: Fix this to send the proper MutationRecords when MutationObservers are present.
      CloneDataFromElement(newNode, nodeToReplace);

      List<Ref<dom::Node>> children;
      for (auto *child = nodeToReplace.FirstChild(); child; child = child->NextSibling())
      {
        children.push_back(ShareRef(*child));
      }
      for (auto &child : children)
      {
        (void)newNode.AppendChild(*child);
      }

      (void)parentNode->InsertBefore(newNode, &nodeToReplace);
      (void)parentNode->RemoveChild(nodeToReplace);
    }
  }

  Ref<ReplaceNodeWithSpanCommand> ReplaceNodeWithSpanCommand::Create(dom::Element &element) noexcept
  {
    return CreateRef<ReplaceNodeWithSpanCommand>(element);
  }

  ReplaceNodeWithSpanCommand::ReplaceNodeWithSpanCommand(dom::Element &element) noexcept
      : SimpleEditCommand(element.NodeDocument()), _elementToReplace(ShareRef(element))
  {
  }

  void ReplaceNodeWithSpanCommand::DoApply() noexcept
  {
    if (!_elementToReplace->IsConnected())
    {
      return;
    }

    if (!_spanElement)
    {
      _spanElement = CreateHTMLElement(_elementToReplace->NodeDocument(), u8"span");
    }

    SwapInNodePreservingAttributesAndChildren(*_spanElement, *_elementToReplace);
  }

  void ReplaceNodeWithSpanCommand::DoUnapply() noexcept
  {
    if (!_spanElement || !_spanElement->IsConnected())
    {
      return;
    }

    SwapInNodePreservingAttributesAndChildren(*_elementToReplace, *_spanElement);
  }
}
