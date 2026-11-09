/*
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
