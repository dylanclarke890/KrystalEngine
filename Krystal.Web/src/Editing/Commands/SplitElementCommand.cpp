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

#include "Krystal.Web/Editing/Commands/SplitElementCommand.hpp"
#include "Krystal.Web/Dom/Mixins/ChildNode.hpp"
#include "Krystal.Web/Dom/Nodes/ContainerNode.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Web/Editing/Editing.hpp"

namespace krys::web
{
  Ref<SplitElementCommand> SplitElementCommand::Create(Element &element, Node &splitPointChild) noexcept
  {
    return CreateRef<SplitElementCommand>(element, splitPointChild);
  }

  SplitElementCommand::SplitElementCommand(Element &element, Node &splitPointChild) noexcept
      : SimpleEditCommand(element.NodeDocument()), _element2(ShareRef(element)),
        _atChild(ShareRef(splitPointChild))
  {
    krys_debug_assert(_atChild->ParentNode() == _element2.get());
  }

  void SplitElementCommand::ExecuteApply() noexcept
  {
    if (_atChild->ParentNode() != _element2.get())
    {
      return;
    }

    List<Ref<Node>> children;
    for (auto *node = _element2->FirstChild(); node != _atChild.get(); node = node->NextSibling())
    {
      children.push_back(ShareRef(*node));
    }

    RefPtr<ContainerNode> parent = ShareRefPtr(_element2->ParentNode());
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }
    if (parent->InsertBefore(*_element1, _element2.get()).HasException())
    {
      return;
    }

    // Delete id attribute from the second element because the same id cannot be used for more than one
    // element
    _element2->RemoveAttribute(u8"id");

    for (auto &child : children)
    {
      (void)_element1->AppendChild(*child);
    }
  }

  void SplitElementCommand::DoApply() noexcept
  {
    _element1 = CloneElementWithoutChildren(*_element2);

    ExecuteApply();
  }

  void SplitElementCommand::DoUnapply() noexcept
  {
    RefPtr<Element> element1 = _element1;
    if (!element1 || !element1->HasEditableStyle() || !_element2->HasEditableStyle())
    {
      return;
    }

    List<Ref<Node>> children;
    for (auto *node = element1->FirstChild(); node; node = node->NextSibling())
    {
      children.push_back(ShareRef(*node));
    }

    RefPtr<Node> refChild = ShareRefPtr(_element2->FirstChild());

    for (auto &child : children)
    {
      (void)_element2->InsertBefore(*child, refChild.get());
    }

    // Recover the id attribute of the original element.
    if (auto id = element1->GetAttribute(u8"id"))
    {
      (void)_element2->SetAttribute(u8"id", krys::move(*id));
    }

    (void)dom::mixins::ChildNode::Remove(*element1);
  }

  void SplitElementCommand::DoReapply() noexcept
  {
    if (!_element1)
    {
      return;
    }

    ExecuteApply();
  }
}
