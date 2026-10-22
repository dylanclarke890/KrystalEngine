#include "Krystal.Booey/Editing/RemoveNodePreservingChildrenCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  RemoveNodePreservingChildrenCommand::RemoveNodePreservingChildrenCommand(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
    EditAction editingAction) noexcept
      : CompositeEditCommand(node.NodeDocument(), editingAction), _node(ShareRef(node)),
        _shouldAssumeContentIsAlwaysEditable(shouldAssumeContentIsAlwaysEditable)
  {
  }

  Ref<RemoveNodePreservingChildrenCommand> RemoveNodePreservingChildrenCommand::Create(
    dom::Node &node, ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
    EditAction editingAction) noexcept
  {
    return CreateRef<RemoveNodePreservingChildrenCommand>(node, shouldAssumeContentIsAlwaysEditable,
                                                          editingAction);
  }

  void RemoveNodePreservingChildrenCommand::DoApply() noexcept
  {
    List<Ref<dom::Node>> children;
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_node->ParentNode());
    if (!parent
        || (_shouldAssumeContentIsAlwaysEditable == DoNotAssumeContentIsAlwaysEditable
            && !IsEditableNode(*parent)))
    {
      return;
    }

    for (auto *child = _node->FirstChild(); child; child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      RemoveNode(*child, _shouldAssumeContentIsAlwaysEditable);
      (void)InsertNodeBefore(*child, *_node, _shouldAssumeContentIsAlwaysEditable);
    }
    RemoveNode(*_node, _shouldAssumeContentIsAlwaysEditable);
  }
}
