#include "Krystal.Booey/Editing/AppendNodeCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<AppendNodeCommand> AppendNodeCommand::Create(dom::ContainerNode &parent, dom::Node &node,
                                                   EditAction editingAction) noexcept
  {
    return CreateRef<AppendNodeCommand>(parent, node, editingAction);
  }

  AppendNodeCommand::AppendNodeCommand(dom::ContainerNode &parent, dom::Node &node,
                                       EditAction editingAction) noexcept
      : SimpleEditCommand(parent.NodeDocument(), editingAction), _parent(ShareRef(parent)),
        _node(ShareRef(node))
  {
    krys_debug_assert(!_node->ParentNode());
  }

  void AppendNodeCommand::DoApply() noexcept
  {
    if (!_parent->HasEditableStyle() && _parent->Renderer())
    {
      return;
    }

    (void)_parent->AppendChild(*_node);
  }

  void AppendNodeCommand::DoUnapply() noexcept
  {
    if (!_parent->HasEditableStyle())
    {
      return;
    }

    (void)dom::mixins::ChildNode::Remove(*_node);
  }
}
