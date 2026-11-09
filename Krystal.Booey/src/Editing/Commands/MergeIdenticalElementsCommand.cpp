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

#include "Krystal.Booey/Editing/Commands/MergeIdenticalElementsCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  Ref<MergeIdenticalElementsCommand> MergeIdenticalElementsCommand::Create(dom::Element &element1,
                                                                           dom::Element &element2) noexcept
  {
    return CreateRef<MergeIdenticalElementsCommand>(element1, element2);
  }

  MergeIdenticalElementsCommand::MergeIdenticalElementsCommand(dom::Element &first,
                                                               dom::Element &second) noexcept
      : SimpleEditCommand(first.NodeDocument()), _element1(ShareRef(first)), _element2(ShareRef(second))
  {
    krys_debug_assert(_element1->NextSibling() == _element2.get());
  }

  void MergeIdenticalElementsCommand::DoApply() noexcept
  {
    if (_element1->NextSibling() != _element2.get() || !_element1->HasEditableStyle()
        || !_element2->HasEditableStyle())
    {
      return;
    }

    _atChild = ShareRefPtr(_element2->FirstChild());

    List<Ref<dom::Node>> children;
    for (auto *child = _element1->FirstChild(); child; child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_element2->InsertBefore(*child, _atChild.get());
    }

    (void)dom::mixins::ChildNode::Remove(*_element1);
  }

  void MergeIdenticalElementsCommand::DoUnapply() noexcept
  {
    RefPtr<dom::Node> atChild = krys::move(_atChild);

    RefPtr<dom::ContainerNode> parent = ShareRefPtr(_element2->ParentNode());
    if (!parent || !parent->HasEditableStyle())
    {
      return;
    }

    if (parent->InsertBefore(*_element1, _element2.get()).HasException())
    {
      return;
    }

    List<Ref<dom::Node>> children;
    for (auto *child = _element2->FirstChild(); child && child != atChild.get(); child = child->NextSibling())
    {
      children.push_back(ShareRef(*child));
    }

    for (auto &child : children)
    {
      (void)_element1->AppendChild(*child);
    }
  }
}
