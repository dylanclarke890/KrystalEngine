#include "Krystal.Booey/Editing/RemoveNodeCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<RemoveNodeCommand>
    RemoveNodeCommand::Create(dom::Node &node,
                              ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
                              EditAction editingAction) noexcept
  {
    return CreateRef<RemoveNodeCommand>(node, shouldAssumeContentIsAlwaysEditable, editingAction);
  }

  RemoveNodeCommand::RemoveNodeCommand(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
    EditAction editingAction) noexcept
      : SimpleEditCommand(node.NodeDocument(), editingAction), _node(ShareRef(node)),
        _shouldAssumeContentIsAlwaysEditable(shouldAssumeContentIsAlwaysEditable)
  {
    krys_debug_assert(_node->ParentNode());
  }

  void RemoveNodeCommand::DoApply() noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_node->ParentNode());
    if (!parent
        || (_shouldAssumeContentIsAlwaysEditable == DoNotAssumeContentIsAlwaysEditable
            && !IsEditableNode(*parent) && parent->Renderer()))
    {
      return;
    }
    krys_debug_assert(IsEditableNode(*parent) || !parent->Renderer());

    _parent = krys::move(parent);
    _refChild = ShareRefPtr(_node->NextSibling());

    (void)dom::mixins::ChildNode::Remove(*_node);
  }

  void RemoveNodeCommand::DoUnapply() noexcept
  {
    RefPtr<dom::ContainerNode> parent = krys::move(_parent);
    RefPtr<dom::Node> refChild = krys::move(_refChild);
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    (void)parent->InsertBefore(*_node, refChild.get());
  }
}
