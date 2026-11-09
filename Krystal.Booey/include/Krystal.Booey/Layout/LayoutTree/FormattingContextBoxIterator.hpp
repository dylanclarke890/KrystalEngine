/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/LayoutTree/LayoutDescendantIterator.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief Walks the boxes of a formatting context: the root's descendants, without descending into the
  /// boxes that establish formatting contexts of their own.
  class FormattingContextBoxIterator : public LayoutDescendantIterator<Box>
  {
  public:
    explicit FormattingContextBoxIterator(const ElementBox &root) noexcept
        : LayoutDescendantIterator<Box>(root)
    {
    }

    enum FirstTag
    {
      First
    };

    FormattingContextBoxIterator(const ElementBox &root, FirstTag) noexcept
        : LayoutDescendantIterator<Box>(root, Traversal::FirstWithin<Box>(root))
    {
    }

    FormattingContextBoxIterator &operator++() noexcept
    {
      if (Get().EstablishesFormattingContext())
      {
        TraverseNextSkippingChildren();
      }
      else
      {
        TraverseNext();
      }
      return *this;
    }
  };

  /// @brief The boxes of a formatting context.
  class FormattingContextBoxIteratorAdapter
  {
  private:
    const ElementBox &_root;

  public:
    explicit FormattingContextBoxIteratorAdapter(const ElementBox &root) noexcept : _root(root)
    {
    }

    KRYS_NODISCARD FormattingContextBoxIterator begin() const noexcept
    {
      return {_root, FormattingContextBoxIterator::First};
    }
    KRYS_NODISCARD FormattingContextBoxIterator end() const noexcept
    {
      return FormattingContextBoxIterator(_root);
    }
  };

  KRYS_NODISCARD inline FormattingContextBoxIteratorAdapter
    FormattingContextBoxes(const ElementBox &root) noexcept
  {
    krys_debug_assert(root.EstablishesFormattingContext());
    return FormattingContextBoxIteratorAdapter(root);
  }
}
