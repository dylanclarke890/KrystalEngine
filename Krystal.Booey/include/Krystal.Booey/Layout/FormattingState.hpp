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

#include "Krystal.Booey/Layout/FormattingContexts/FormattingConstraints.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  class Box;

  /// @brief The state a formatting context keeps across layouts: the intrinsic widths of its boxes and of its
  /// content.
  class FormattingState
  {
    KRYS_NON_COPY_MOVABLE(FormattingState);

  protected:
    enum class Type : uint8
    {
      Block,
      Table,
      Flex
    };

  private:
    const CheckedRef<LayoutState> _layoutState;
    HashMap<const Box *, IntrinsicWidthConstraints> _intrinsicWidthConstraintsForBoxes;
    Maybe<IntrinsicWidthConstraints> _intrinsicWidthConstraints;
    Type _type;

  public:
    void SetIntrinsicWidthConstraintsForBox(const Box &layoutBox,
                                            IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept;
    KRYS_NODISCARD Maybe<IntrinsicWidthConstraints>
      IntrinsicWidthConstraintsForBox(const Box &layoutBox) const noexcept;
    void ClearIntrinsicWidthConstraints(const Box &layoutBox) noexcept;

    void SetIntrinsicWidthConstraints(IntrinsicWidthConstraints intrinsicWidthConstraints) noexcept
    {
      _intrinsicWidthConstraints = intrinsicWidthConstraints;
    }
    KRYS_NODISCARD Maybe<layout::IntrinsicWidthConstraints> GetIntrinsicWidthConstraints() const noexcept
    {
      return _intrinsicWidthConstraints;
    }

    KRYS_NODISCARD bool IsBlockFormattingState() const noexcept
    {
      return _type == Type::Block;
    }
    KRYS_NODISCARD bool IsTableFormattingState() const noexcept
    {
      return _type == Type::Table;
    }
    KRYS_NODISCARD bool IsFlexFormattingState() const noexcept
    {
      return _type == Type::Flex;
    }

    KRYS_NODISCARD LayoutState &GetLayoutState() const noexcept
    {
      return *_layoutState;
    }

    // FIXME: We need to find a way to limit access to mutable geometry.
    BoxGeometry &GetBoxGeometry(const Box &layoutBox) noexcept;

  protected:
    FormattingState(Type type, LayoutState &layoutState) noexcept;
    ~FormattingState() noexcept;
  };
}
