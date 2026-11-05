#include "Krystal.Booey/Editing/Commands/InsertIntoTextNodeCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<InsertIntoTextNodeCommand> InsertIntoTextNodeCommand::Create(dom::Text &node, uint32 offset,
                                                                   utf8_stringview text,
                                                                   AllowPasswordEcho allowPasswordEcho,
                                                                   EditAction editingAction) noexcept
  {
    return CreateRef<InsertIntoTextNodeCommand>(node, offset, text, allowPasswordEcho, editingAction);
  }

  InsertIntoTextNodeCommand::InsertIntoTextNodeCommand(dom::Text &node, uint32 offset, utf8_stringview text,
                                                       AllowPasswordEcho allowPasswordEcho,
                                                       EditAction editingAction) noexcept
      : SimpleEditCommand(node.NodeDocument(), editingAction), _node(ShareRef(node)), _offset(offset),
        _text(text), _allowPasswordEcho(allowPasswordEcho)
  {
    krys_debug_assert(_offset <= _node->Length());
    krys_debug_assert(!_text.empty());
  }

  // FIXME: Password echo (revealing the last typed character of a password for a moment).
  void InsertIntoTextNodeCommand::DoApply() noexcept
  {
    if (!_node->HasEditableStyle())
    {
      return;
    }

    (void)_node->InsertData(_offset, _text);
  }

  void InsertIntoTextNodeCommand::DoReapply() noexcept
  {
    if (!_node->HasEditableStyle())
    {
      return;
    }

    (void)_node->InsertData(_offset, _text);
  }

  void InsertIntoTextNodeCommand::DoUnapply() noexcept
  {
    if (!_node->HasEditableStyle())
    {
      return;
    }

    (void)_node->DeleteData(_offset, _text.size());
  }
}
