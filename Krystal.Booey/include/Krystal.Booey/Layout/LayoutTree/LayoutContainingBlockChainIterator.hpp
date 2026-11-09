/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief Walks a box's containing blocks outwards.
  class LayoutContainingBlockChainIterator
  {
  private:
    const ElementBox *_current {nullptr};

  public:
    LayoutContainingBlockChainIterator() noexcept = default;
    explicit LayoutContainingBlockChainIterator(const ElementBox *current) noexcept : _current(current)
    {
    }

    KRYS_NODISCARD const ElementBox &operator*() const noexcept
    {
      return *_current;
    }
    KRYS_NODISCARD const ElementBox *operator->() const noexcept
    {
      return _current;
    }

    LayoutContainingBlockChainIterator &operator++() noexcept
    {
      krys_debug_assert(_current);
      _current =
        _current->IsInitialContainingBlock() ? nullptr : &FormattingContext::ContainingBlock(*_current);
      return *this;
    }

    KRYS_NODISCARD friend bool operator==(LayoutContainingBlockChainIterator,
                                          LayoutContainingBlockChainIterator) noexcept = default;
  };

  /// @brief A containing block and its containing blocks, up to (but not including) an optional ancestor.
  class LayoutContainingBlockChainIteratorAdapter
  {
  private:
    const ElementBox &_containingBlock;
    const ElementBox *_stayWithin {nullptr};

  public:
    explicit LayoutContainingBlockChainIteratorAdapter(const ElementBox &containingBlock,
                                                       const ElementBox *stayWithin = nullptr) noexcept
        : _containingBlock(containingBlock), _stayWithin(stayWithin)
    {
    }

    KRYS_NODISCARD LayoutContainingBlockChainIterator begin() const noexcept
    {
      return LayoutContainingBlockChainIterator(&_containingBlock);
    }
    KRYS_NODISCARD LayoutContainingBlockChainIterator end() const noexcept
    {
      return LayoutContainingBlockChainIterator(_stayWithin);
    }
  };

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChain(const Box &layoutBox) noexcept
  {
    return LayoutContainingBlockChainIteratorAdapter(FormattingContext::ContainingBlock(layoutBox));
  }

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChain(const Box &layoutBox, const ElementBox &stayWithin) noexcept
  {
    krys_debug_assert(layoutBox.IsDescendantOf(stayWithin));
    return LayoutContainingBlockChainIteratorAdapter(FormattingContext::ContainingBlock(layoutBox),
                                                     &stayWithin);
  }

  KRYS_NODISCARD inline LayoutContainingBlockChainIteratorAdapter
    ContainingBlockChainWithinFormattingContext(const Box &layoutBox, const ElementBox &root) noexcept
  {
    krys_debug_assert(root.EstablishesFormattingContext());
    return ContainingBlockChain(layoutBox, root);
  }
}
