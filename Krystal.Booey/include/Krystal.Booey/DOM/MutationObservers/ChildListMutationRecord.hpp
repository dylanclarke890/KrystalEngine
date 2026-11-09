/*
 * Copyright (C) 2011 Google Inc. All rights reserved.
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"

namespace krys::boo::dom
{
  class ContainerNode;

  class ChildListMutationRecord final : public MutationRecord
  {
  private:
    Ref<Node> _target;
    Ref<NodeList> _addedNodes;
    Ref<NodeList> _removedNodes;
    RefPtr<Node> _previousSibling;
    RefPtr<Node> _nextSibling;

  public:
    ChildListMutationRecord(Ref<Node> &&target, Ref<NodeList> &&added, Ref<NodeList> &&removed,
                            RefPtr<Node> &&previousSibling, RefPtr<Node> &&nextSibling) noexcept;

    KRYS_NODISCARD DOMStringAtom Type() const noexcept override
    {
      return u8"childList";
    }

    KRYS_NODISCARD Ref<Node> Target() const noexcept override
    {
      return _target;
    }

    KRYS_NODISCARD RefPtr<NodeList> AddedNodes() const noexcept override
    {
      return _addedNodes;
    }

    KRYS_NODISCARD RefPtr<NodeList> RemovedNodes() const noexcept override
    {
      return _removedNodes;
    }

    KRYS_NODISCARD RefPtr<Node> PreviousSibling() const noexcept override
    {
      return _previousSibling;
    }

    KRYS_NODISCARD RefPtr<Node> NextSibling() const noexcept override
    {
      return _nextSibling;
    }
  };
}