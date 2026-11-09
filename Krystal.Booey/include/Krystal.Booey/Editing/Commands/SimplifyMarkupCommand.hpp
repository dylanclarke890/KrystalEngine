/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Editing/Commands/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Removes the inline elements (and the redundant `div`s) in a run of inserted nodes that don't
  /// change how it looks.
  class SimplifyMarkupCommand final : public CompositeEditCommand
  {
  private:
    RefPtr<dom::Node> _firstNode;
    RefPtr<dom::Node> _nodeAfterLast;

  public:
    SimplifyMarkupCommand(dom::Document &document, dom::Node *firstNode, dom::Node *nodeAfterLast) noexcept;

    KRYS_NODISCARD static Ref<SimplifyMarkupCommand> Create(dom::Document &document, dom::Node *firstNode,
                                                            dom::Node *nodeAfterLast) noexcept;

  private:
    void DoApply() noexcept override;
    int32 PruneSubsequentAncestorsToRemove(List<Ref<dom::Node>> &nodesToRemove,
                                           size_t startNodeIndex) noexcept;
  };
}
