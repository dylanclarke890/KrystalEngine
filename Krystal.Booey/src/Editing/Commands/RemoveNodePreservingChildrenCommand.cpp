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

#include "Krystal.Booey/Editing/Commands/RemoveNodePreservingChildrenCommand.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
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
