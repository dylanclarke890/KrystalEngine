/*
 * Copyright (C) 2015-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Node;
}

namespace krys::boo
{
  class RenderElement;
  class RenderObject;

  /// @brief Where a node's new renderer goes in the render tree: a parent, and the sibling to insert it
  /// before.
  class RenderTreePosition
  {
  private:
    RenderElement *_parent;
    RenderObject *_nextSibling {nullptr};
    bool _hasValidNextSibling {false};

  public:
    explicit RenderTreePosition(RenderElement &parent) noexcept : _parent(&parent)
    {
    }

    RenderTreePosition(RenderElement &parent, RenderObject *nextSibling) noexcept
        : _parent(&parent), _nextSibling(nextSibling), _hasValidNextSibling(true)
    {
    }

    KRYS_NODISCARD RenderElement &Parent() const noexcept
    {
      return *_parent;
    }

    KRYS_NODISCARD RenderObject *NextSibling() const noexcept
    {
      krys_debug_assert(_hasValidNextSibling);
      return _nextSibling;
    }

    void ComputeNextSibling(const dom::Node &node) noexcept;

    void MoveToLastChild() noexcept
    {
      _nextSibling = nullptr;
      _hasValidNextSibling = true;
    }

    void InvalidateNextSibling() noexcept
    {
      _hasValidNextSibling = false;
    }

    void InvalidateNextSibling(const RenderObject &siblingRenderer) noexcept;

    KRYS_NODISCARD RenderObject *NextSiblingRenderer(const dom::Node &node) const noexcept;
  };
}
