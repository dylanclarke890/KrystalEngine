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

#include "Krystal.Booey/Layout/FormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/FormattingContext.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"

namespace krys::boo::layout
{
  FormattingState::FormattingState(Type type, LayoutState &layoutState) noexcept
      : _layoutState(ShareCheckedRef(layoutState)), _type(type)
  {
  }

  FormattingState::~FormattingState() noexcept = default;

  void FormattingState::SetIntrinsicWidthConstraintsForBox(
    const Box &layoutBox, IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept
  {
    krys_debug_assert(!_intrinsicWidthConstraintsForBoxes.contains(&layoutBox));
    krys_debug_assert(
      &_layoutState->FormattingStateForFormattingContext(FormattingContext::FormattingContextRoot(layoutBox))
      == this);
    _intrinsicWidthConstraintsForBoxes[&layoutBox] = intrinsicWidthConstraints;
  }

  void FormattingState::ClearIntrinsicWidthConstraints(const Box &layoutBox) noexcept
  {
    _intrinsicWidthConstraints = null;
    _intrinsicWidthConstraintsForBoxes.erase(&layoutBox);
  }

  Maybe<IntrinsicWidthConstraints>
    FormattingState::IntrinsicWidthConstraintsForBox(const Box &layoutBox) const noexcept
  {
    krys_debug_assert(
      &_layoutState->FormattingStateForFormattingContext(FormattingContext::FormattingContextRoot(layoutBox))
      == this);
    auto iterator = _intrinsicWidthConstraintsForBoxes.find(&layoutBox);
    if (iterator == _intrinsicWidthConstraintsForBoxes.end())
    {
      return null;
    }
    return iterator->second;
  }

  BoxGeometry &FormattingState::GetBoxGeometry(const Box &layoutBox) noexcept
  {
    // FIXME: Remove this when all FormattingStates transtioned to a cache setup.
    return _layoutState->EnsureGeometryForBox(layoutBox);
  }
}
