/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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
  /// @brief Iterates over the ancestors of type @p T (which may be const) of a renderer.
  template <typename T>
  class RenderAncestorIterator : public RenderIterator<T>
  {
  public:
    RenderAncestorIterator() noexcept : RenderIterator<T>(nullptr)
    {
    }

    explicit RenderAncestorIterator(T *current) noexcept : RenderIterator<T>(nullptr, current)
    {
    }

    RenderAncestorIterator &operator++() noexcept
    {
      return static_cast<RenderAncestorIterator &>(RenderIterator<T>::TraverseAncestor());
    }
  };

  template <typename T>
  using RenderAncestorConstIterator = RenderAncestorIterator<const T>;

  /// @brief The ancestors of type @p T (which may be const) of a renderer, from the first.
  template <typename T>
  class RenderAncestorIteratorAdapter
  {
  private:
    T *_first;

  public:
    explicit RenderAncestorIteratorAdapter(T *first) noexcept : _first(first)
    {
    }

    KRYS_NODISCARD RenderAncestorIterator<T> begin() const noexcept
    {
      return RenderAncestorIterator<T>(_first);
    }

    KRYS_NODISCARD RenderAncestorIterator<T> end() const noexcept
    {
      return RenderAncestorIterator<T>();
    }

    KRYS_NODISCARD T *First() const noexcept
    {
      return _first;
    }
  };

  template <typename T>
  using RenderAncestorConstIteratorAdapter = RenderAncestorIteratorAdapter<const T>;

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorIteratorAdapter<T> AncestorsOfType(RenderObject &descendant) noexcept
  {
    T *first = RenderTraversal::FindAncestorOfType<T>(descendant);
    return RenderAncestorIteratorAdapter<T>(first);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorConstIteratorAdapter<T>
    AncestorsOfType(const RenderObject &descendant) noexcept
  {
    const T *first = RenderTraversal::FindAncestorOfType<const T>(descendant);
    return RenderAncestorConstIteratorAdapter<T>(first);
  }

  /// @brief @p first if it's a @p T, then its ancestors of type @p T.
  template <typename T>
  KRYS_NODISCARD inline RenderAncestorIteratorAdapter<T> LineageOfType(RenderObject &first) noexcept
  {
    if (IsRendererOfType<T>(first))
    {
      return RenderAncestorIteratorAdapter<T>(static_cast<T *>(&first));
    }
    return AncestorsOfType<T>(first);
  }

  template <typename T>
  KRYS_NODISCARD inline RenderAncestorConstIteratorAdapter<T>
    LineageOfType(const RenderObject &first) noexcept
  {
    if (IsRendererOfType<T>(first))
    {
      return RenderAncestorConstIteratorAdapter<T>(static_cast<const T *>(&first));
    }
    return AncestorsOfType<T>(first);
  }
}
