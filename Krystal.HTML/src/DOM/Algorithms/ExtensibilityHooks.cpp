#include "Krystal.HTML/DOM/Algorithms/ExtensibilityHooks.hpp"
#include "Krystal.HTML/Constants/Attributes.hpp"
#include "Krystal.HTML/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
{
  void ExtensibilityHooks::NodeInserted(Node &insertedNode) noexcept
  {
    auto *parent = insertedNode.ParentNode();
    if (parent == nullptr)
    {
      return;
    }

    insertedNode.AddRef();

    if (parent->IsConnected())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsConnected);
    }

    if (parent->IsInShadowTree())
    {
      insertedNode.SetNodeFlag(NodeFlags::IsInShadowTree);
    }

    insertedNode.OnInsert();
  }

  void ExtensibilityHooks::NodePostConnection(Node &connectedNode) noexcept
  {
    connectedNode.OnPostConnection();
  }

  void ExtensibilityHooks::NodeChildrenChanged(ContainerNode &node) noexcept
  {
    node.OnChildrenChanged();
  }

  void ExtensibilityHooks::NodeMoved(Node &movedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept
  {
    auto *parent = movedNode.ParentNode();
    assert(parent != nullptr);

    // NOTE: a node can be moved from a shadow tree to a light tree and vice versa but only as long as
    // they share the same shadow-including root (i.e. they're in the same document). This means that
    // `IsConnected` will be the same for the node before and after the move but `IsInShadowTree` may be
    // different.

    if (parent->IsInShadowTree())
    {
      movedNode.SetNodeFlag(NodeFlags::IsInShadowTree);
    }
    else
    {
      movedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);
    }

    movedNode.OnMove(isSubtreeRoot, oldAncestor);
  }

  void ExtensibilityHooks::NodeRemoved(Node &removedNode, bool isSubtreeRoot,
                                       ContainerNode &oldAncestor) noexcept
  {
    removedNode.ClearNodeFlag(NodeFlags::IsConnected);
    removedNode.ClearNodeFlag(NodeFlags::IsInShadowTree);

    removedNode.OnRemove(isSubtreeRoot, oldAncestor);

    if (removedNode.ParentNode() == nullptr)
    {
      removedNode.SubRef();
    }
  }

  void ExtensibilityHooks::NodeCloned(Node &node, Node &copy, bool subtree) noexcept
  {
    node.OnClone(copy, subtree);
  }

  void ExtensibilityHooks::NodeAdopted(Node &node, Document &oldDocument) noexcept
  {
    node.OnAdopt(oldDocument);
  }

  void ExtensibilityHooks::ElementAttributeChanged(Element &element, DOMStringAtom localName,
                                                   Maybe<DOMStringView> oldValue, Maybe<DOMStringView> value,
                                                   DOMStringAtom namespaceURI) noexcept
  {
    // NOTE: we currently don't need this as we just use the value stored in the attribute. Later, when
    // we optimize we can use this hook to update Element's member variable to skip needing to lookup the
    // attribute. This will be particularly important for a faster implementation of GetElementById.
    // if (localName == u8"id")
    //{
    //   if (namespaceURI == DOMStringAtom::Null())
    //   {
    //     if (value.has_value() && !value->empty())
    //     {
    //       element.Id(DOMString(*value));
    //     }
    //     else
    //     {
    //       element.Id(u8"");
    //     }
    //   }
    // }

    // TODO(fix): the below logic needs reviewing and a better way of doing this needs to be found.

    if (localName == Attributes::Class)
    {
      if (namespaceURI == DOMStringAtom::Null())
      {
        auto &domTokenList = element.ClassList();
        domTokenList._tokens.clear();
        if (value.has_value())
        {
          auto tokens = OrderedSet::Parser(*value);
          domTokenList._tokens.append(tokens.begin(), tokens.end());
        }
      }
    }
    else if (localName == Attributes::Name)
    {
      if (Is<HTMLSlotElement>(element) && namespaceURI == DOMStringAtom::Null())
      {
        if (value == oldValue)
        {
          return;
        }

        if (!value.has_value() && oldValue.has_value() && oldValue->empty())
        {
          return;
        }

        if (value.has_value() && value->empty() && !oldValue.has_value())
        {
          return;
        }

        auto &slotElement = Downcast<HTMLSlotElement>(element);
        if (!value.has_value() || value->empty())
        {
          slotElement.Name(u8"");
        }
        else
        {
          slotElement.Name(DOMString(*value));
        }

        SlotAlgorithms::AssignSlottablesForTree(TreeQueries::Root(slotElement));
      }
    }
    else if (localName == Attributes::Slot)
    {
      if (namespaceURI == DOMStringAtom::Null())
      {
        if (value == oldValue)
        {
          return;
        }

        if (!value.has_value() && oldValue.has_value() && oldValue->empty())
        {
          return;
        }

        if (value.has_value() && value->empty() && !oldValue.has_value())
        {
          return;
        }

        if (!value.has_value() || value->empty())
        {
          element._slottableName = u8"";
        }
        else
        {
          element._slottableName = DOMString(*value);
        }

        if (auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(element))
        {
          SlotAlgorithms::AssignSlottables(*assignedSlot);
        }

        SlotAlgorithms::AssignSlot(element);
      }
    }
  }
}