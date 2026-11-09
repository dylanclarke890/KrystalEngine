/*
 * Copyright (C) 2013-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Iterators/RenderIterator.hpp"

namespace krys::boo
{
  /// @brief Iterates over the children of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderChildIterator : public RenderIterator<T>
  {
  public:
    explicit RenderChildIterator(const RenderElement &parent) noexcept : RenderIterator<T>(&parent)
    {
    }

    RenderChildIterator(const RenderElement &parent, T *current) noexcept
        : RenderIterator<T>(&parent, current)
    {
    }

    RenderChildIterator &operator++() noexcept
    {
      return static_cast<RenderChildIterator &>(RenderIterator<T>::TraverseNextSibling());
    }
  };

  template <typename T>
  using RenderChildConstIterator = RenderChildIterator<const T>;

  /// @brief The children of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderChildIteratorAdapter
  {
  private:
    RenderElementFor<T> *_parent;

  public:
    explicit RenderChildIteratorAdapter(RenderElementFor<T> &parent) noexcept : _parent(&parent)
    {
    }

    KRYS_NODISCARD RenderChildIterator<T> begin() const noexcept
    {
      return RenderChildIterator<T>(*_parent, RenderTraversal::FirstChild<T>(*_parent));
    }

    KRYS_NODISCARD RenderChildIterator<T> end() const noexcept
    {
      return RenderChildIterator<T>(*_parent);
    }

    KRYS_NODISCARD T *First() const noexcept
    {
      return RenderTraversal::FirstChild<T>(*_parent);
    }

    KRYS_NODISCARD T *Last() const noexcept
    {
      return RenderTraversal::LastChild<T>(*_parent);
    }
  };

  template <typename T>
  using RenderChildConstIteratorAdapter = RenderChildIteratorAdapter<const T>;

  template <typename T>
  KRYS_NODISCARD inline RenderChildIteratorAdapter<T> ChildrenOfType(RenderElement &parent) noexcept
  {
    return RenderChildIteratorAdapter<T>(parent);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderChildConstIteratorAdapter<T>
    ChildrenOfType(const RenderElement &parent) noexcept
  {
    return RenderChildConstIteratorAdapter<T>(parent);
  }
}
