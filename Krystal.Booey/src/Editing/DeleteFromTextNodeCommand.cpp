#include "Krystal.Booey/Editing/DeleteFromTextNodeCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<DeleteFromTextNodeCommand> DeleteFromTextNodeCommand::Create(dom::Text &node, uint32 offset,
                                                                   uint32 count,
                                                                   EditAction editingAction) noexcept
  {
    return CreateRef<DeleteFromTextNodeCommand>(node, offset, count, editingAction);
  }

  DeleteFromTextNodeCommand::DeleteFromTextNodeCommand(dom::Text &node, uint32 offset, uint32 count,
                                                       EditAction editingAction) noexcept
      : SimpleEditCommand(node.NodeDocument(), editingAction), _node(ShareRef(node)), _offset(offset),
        _count(count)
  {
    krys_debug_assert(_offset <= _node->Length());
    krys_debug_assert(_offset + _count <= _node->Length());
  }

  void DeleteFromTextNodeCommand::DoApply() noexcept
  {
    if (!IsEditableNode(*_node))
    {
      return;
    }

    auto result = _node->SubstringData(_offset, _count);
    if (result.HasException())
    {
      return;
    }
    _text = result.ReleaseValue();

    (void)_node->DeleteData(_offset, _count);
  }

  void DeleteFromTextNodeCommand::DoUnapply() noexcept
  {
    if (!_node->HasEditableStyle())
    {
      return;
    }

    (void)_node->InsertData(_offset, utf8_string(_text));
  }
}
