/*
 * Copyright (C) 2007, 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Ranges/Position.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @brief A Position iterator with constant-time increment, decrement, and several predicates on the
  /// Position it is at. Conversion to/from Position is O(n) in the offset.
  class PositionIterator
  {
  private:
    RefPtr<Node> _anchorNode;
    // If this is non-null, _nodeAfterPositionInAnchor->ParentNode() == _anchorNode;
    RefPtr<Node> _nodeAfterPositionInAnchor;
    int32 _offsetInAnchor {0};

  public:
    PositionIterator(const Position &position) noexcept;
    operator Position() const noexcept;

    void Increment() noexcept;
    void Decrement() noexcept;

    KRYS_NODISCARD Node *GetNode() const noexcept
    {
      return _anchorNode.get();
    }
    KRYS_NODISCARD int32 OffsetInLeafNode() const noexcept
    {
      return _offsetInAnchor;
    }

    KRYS_NODISCARD bool AtStart() const noexcept;
    KRYS_NODISCARD bool AtEnd() const noexcept;
    KRYS_NODISCARD bool AtStartOfNode() const noexcept;
    KRYS_NODISCARD bool AtEndOfNode() const noexcept;
    KRYS_NODISCARD bool IsCandidate() const noexcept;
  };
}
