/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/LayoutTree/LayoutIterator.hpp"

namespace krys::boo::layout
{
  /// @brief Iterates over the descendants of type @p T of a layout box, in tree order.
  template <typename T>
  class LayoutDescendantIterator : public LayoutIterator<T>
  {
  public:
    explicit LayoutDescendantIterator(const ElementBox &root) noexcept : LayoutIterator<T>(&root)
    {
    }

    LayoutDescendantIterator(const ElementBox &root, const T *current) noexcept
        : LayoutIterator<T>(&root, current)
    {
    }

    LayoutDescendantIterator &operator++() noexcept
    {
      return static_cast<LayoutDescendantIterator &>(LayoutIterator<T>::TraverseNext());
    }
  };

  /// @brief The descendants of type @p T of a layout box.
  template <typename T>
  class LayoutDescendantIteratorAdapter
  {
  private:
    const ElementBox &_root;

  public:
    explicit LayoutDescendantIteratorAdapter(const ElementBox &root) noexcept : _root(root)
    {
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> begin() noexcept
    {
      return LayoutDescendantIterator<T>(_root, Traversal::FirstWithin<T>(_root));
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> end() noexcept
    {
      return LayoutDescendantIterator<T>(_root);
    }

    KRYS_NODISCARD LayoutDescendantIterator<T> At(const T &current) noexcept
    {
      return LayoutDescendantIterator<T>(_root, &current);
    }
  };

  template <typename T>
  KRYS_NODISCARD inline LayoutDescendantIteratorAdapter<T> DescendantsOfType(const ElementBox &root) noexcept
  {
    return LayoutDescendantIteratorAdapter<T>(root);
  }
}
