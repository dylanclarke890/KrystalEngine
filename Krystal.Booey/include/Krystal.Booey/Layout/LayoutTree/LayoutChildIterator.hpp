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
  /// @brief Iterates over the children of type @p T of a layout box.
  template <typename T>
  class LayoutChildIterator : public LayoutIterator<T>
  {
  public:
    explicit LayoutChildIterator(const ElementBox &parent) noexcept : LayoutIterator<T>(&parent)
    {
    }

    LayoutChildIterator(const ElementBox &parent, const T *current) noexcept
        : LayoutIterator<T>(&parent, current)
    {
    }

    LayoutChildIterator &operator++() noexcept
    {
      return static_cast<LayoutChildIterator &>(LayoutIterator<T>::TraverseNextSibling());
    }
  };

  /// @brief The children of type @p T of a layout box.
  template <typename T>
  class LayoutChildIteratorAdapter
  {
  private:
    const ElementBox &_parent;

  public:
    explicit LayoutChildIteratorAdapter(const ElementBox &parent) noexcept : _parent(parent)
    {
    }

    KRYS_NODISCARD LayoutChildIterator<T> begin() const noexcept
    {
      return LayoutChildIterator<T>(_parent, Traversal::FirstChild<T>(_parent));
    }

    KRYS_NODISCARD LayoutChildIterator<T> end() const noexcept
    {
      return LayoutChildIterator<T>(_parent);
    }

    KRYS_NODISCARD const T *First() const noexcept
    {
      return Traversal::FirstChild<T>(_parent);
    }
  };

  template <typename T>
  KRYS_NODISCARD inline LayoutChildIteratorAdapter<T> ChildrenOfType(const ElementBox &parent) noexcept
  {
    return LayoutChildIteratorAdapter<T>(parent);
  }
}
