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

#include "Krystal.Booey/Editing/Commands/SplitTextNodeCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<SplitTextNodeCommand> SplitTextNodeCommand::Create(dom::Text &node, uint32 offset) noexcept
  {
    return CreateRef<SplitTextNodeCommand>(node, offset);
  }

  SplitTextNodeCommand::SplitTextNodeCommand(dom::Text &text, uint32 offset) noexcept
      : SimpleEditCommand(text.NodeDocument()), _text2(ShareRef(text)), _offset(offset)
  {
    // NOTE: Various callers rely on the fact that the original node becomes
    // the second node (i.e. the new node is inserted before the existing one).
    // That is not a fundamental dependency (i.e. it could be re-coded), but
    // rather is based on how this code happens to work.
    krys_debug_assert(_text2->Length() > 0);
    krys_debug_assert(_offset > 0);
    krys_debug_assert(_offset < _text2->Length());
  }

  // (There are no document markers to copy; see DIFFERENCES.md.)
  void SplitTextNodeCommand::DoApply() noexcept
  {
    auto *parent = _text2->ParentNode();
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    auto result = _text2->SubstringData(0, _offset);
    if (result.HasException())
    {
      return;
    }
    auto prefixText = result.ReleaseValue();
    if (prefixText.empty())
    {
      return;
    }

    _text1 = GetDocument().CreateTextNode(krys::move(prefixText));
    krys_debug_assert(_text1);

    InsertText1AndTrimText2();
  }

  void SplitTextNodeCommand::DoUnapply() noexcept
  {
    RefPtr<dom::Text> text1 = _text1;
    if (!text1 || !text1->HasEditableStyle())
    {
      return;
    }

    krys_debug_assert(&text1->NodeDocument() == &GetDocument());

    utf8_string prefixText = text1->Data();

    (void)_text2->InsertData(0, prefixText);

    (void)dom::mixins::ChildNode::Remove(*text1);
  }

  void SplitTextNodeCommand::DoReapply() noexcept
  {
    if (!_text1)
    {
      return;
    }

    auto *parent = _text2->ParentNode();
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    InsertText1AndTrimText2();
  }

  void SplitTextNodeCommand::InsertText1AndTrimText2() noexcept
  {
    if (_text2->ParentNode()->InsertBefore(*_text1, _text2.get()).HasException())
    {
      return;
    }
    (void)_text2->DeleteData(0, _offset);
  }
}
