/*
 * Copyright (c) 2009 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Editing/Commands/ReplaceNodeWithSpanCommand.hpp"
#include "Krystal.Booey/DOM/Mixins/ChildNode.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/Editing/Commands/ApplyStyleCommand.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"

namespace krys::boo
{
  namespace
  {
    void SwapInNodePreservingAttributesAndChildren(dom::Element &newNode,
                                                   dom::Element &nodeToReplace) noexcept
    {
      krys_debug_assert(nodeToReplace.IsConnected());
      RefPtr<dom::ContainerNode> parentNode = ShareRefPtr(nodeToReplace.ParentNode());

      // FIXME: Fix this to send the proper MutationRecords when MutationObservers are present.
      CloneDataFromElement(newNode, nodeToReplace);

      List<Ref<dom::Node>> children;
      for (auto *child = nodeToReplace.FirstChild(); child; child = child->NextSibling())
      {
        children.push_back(ShareRef(*child));
      }
      for (auto &child : children)
      {
        (void)newNode.AppendChild(*child);
      }

      (void)parentNode->InsertBefore(newNode, &nodeToReplace);
      (void)parentNode->RemoveChild(nodeToReplace);
    }
  }

  Ref<ReplaceNodeWithSpanCommand> ReplaceNodeWithSpanCommand::Create(dom::Element &element) noexcept
  {
    return CreateRef<ReplaceNodeWithSpanCommand>(element);
  }

  ReplaceNodeWithSpanCommand::ReplaceNodeWithSpanCommand(dom::Element &element) noexcept
      : SimpleEditCommand(element.NodeDocument()), _elementToReplace(ShareRef(element))
  {
  }

  void ReplaceNodeWithSpanCommand::DoApply() noexcept
  {
    if (!_elementToReplace->IsConnected())
    {
      return;
    }

    if (!_spanElement)
    {
      _spanElement = CreateHTMLElement(_elementToReplace->NodeDocument(), u8"span");
    }

    SwapInNodePreservingAttributesAndChildren(*_spanElement, *_elementToReplace);
  }

  void ReplaceNodeWithSpanCommand::DoUnapply() noexcept
  {
    if (!_spanElement || !_spanElement->IsConnected())
    {
      return;
    }

    SwapInNodePreservingAttributesAndChildren(*_elementToReplace, *_spanElement);
  }
}
