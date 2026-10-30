#include "Krystal.Booey/Editing/InsertIntoTextNodeCommand.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/ApplyStyleCommand.hpp"
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

    (void)_node->InsertData(_offset, utf8_string(_text));
  }

  void InsertIntoTextNodeCommand::DoReapply() noexcept
  {
    if (!_node->HasEditableStyle())
    {
      return;
    }

    (void)_node->InsertData(_offset, utf8_string(_text));
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
