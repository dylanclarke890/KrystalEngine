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

#include "Krystal.Booey/Editing/Commands/InsertNodeBeforeCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<InsertNodeBeforeCommand>
    InsertNodeBeforeCommand::Create(dom::Node &childToInsert, dom::Node &childToInsertBefore,
                                    ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
                                    EditAction editingAction) noexcept
  {
    return CreateRef<InsertNodeBeforeCommand>(childToInsert, childToInsertBefore,
                                              shouldAssumeContentIsAlwaysEditable, editingAction);
  }

  InsertNodeBeforeCommand::InsertNodeBeforeCommand(
    dom::Node &insertChild, dom::Node &refChild,
    ShouldAssumeContentIsAlwaysEditable shouldAssumeContentIsAlwaysEditable,
    EditAction editingAction) noexcept
      : SimpleEditCommand(refChild.NodeDocument(), editingAction), _insertChild(ShareRef(insertChild)),
        _refChild(ShareRef(refChild)),
        _shouldAssumeContentIsAlwaysEditable(shouldAssumeContentIsAlwaysEditable)
  {
    krys_debug_assert(!_insertChild->ParentNode());
    krys_debug_assert(_refChild->ParentNode());
    krys_debug_assert(_refChild->ParentNode()->HasEditableStyle() || !_refChild->ParentNode()->Renderer());
  }

  void InsertNodeBeforeCommand::DoApply() noexcept
  {
    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_refChild->ParentNode());
    if (!parent
        || (_shouldAssumeContentIsAlwaysEditable == DoNotAssumeContentIsAlwaysEditable
            && !IsEditableNode(*parent)))
    {
      return;
    }
    krys_debug_assert(IsEditableNode(*parent));

    (void)parent->InsertBefore(*_insertChild, _refChild.get());
  }

  void InsertNodeBeforeCommand::DoUnapply() noexcept
  {
    if (!IsEditableNode(*_insertChild))
    {
      return;
    }

    (void)dom::mixins::ChildNode::Remove(*_insertChild);
  }
}
