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

#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutUnits.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  /// @brief A box being placed among floats: a float itself (intersecting with its margin box) or a
  /// formatting context root next to them (intersecting with its border box).
  class FloatAvoider
  {
  private:
    // These coordinate values are relative to the formatting root's border box.
    LayoutPoint _absoluteTopLeft;
    // Note that float avoider should work with no height value.
    LayoutUnit _borderBoxWidth;
    BoxGeometry::Edges _margin;
    BoxGeometry::HorizontalEdges _containingBlockAbsoluteContentBox;
    bool _isFloatingPositioned {true};
    bool _isStartAligned {true};

  public:
    FloatAvoider(LayoutPoint absoluteTopLeft, LayoutUnit borderBoxWidth, const BoxGeometry::Edges &margin,
                 BoxGeometry::HorizontalEdges containingBlockAbsoluteContentBox, bool isFloatingPositioned,
                 bool isStartAligned) noexcept;
    virtual ~FloatAvoider() noexcept = default;

    void SetInlineStart(LayoutUnit inlineStart) noexcept;
    void SetBlockStart(LayoutUnit blockStart) noexcept;
    void ResetInlineStart() noexcept
    {
      _absoluteTopLeft.SetX(InitialInlineStart());
    }

    KRYS_NODISCARD bool OverflowsContainingBlock() const noexcept;

    KRYS_NODISCARD LayoutUnit BlockStart() const noexcept
    {
      auto blockStart = _absoluteTopLeft.Y();
      if (IsFloatingBox())
      {
        blockStart -= MarginBefore();
      }
      return blockStart;
    }

    KRYS_NODISCARD LayoutUnit InlineStart() const noexcept
    {
      auto inlineStart = _absoluteTopLeft.X();
      if (IsFloatingBox())
      {
        inlineStart -= MarginStart();
      }
      return inlineStart;
    }

    KRYS_NODISCARD LayoutUnit InlineEnd() const noexcept
    {
      auto inlineEnd = InlineStart() + BorderBoxWidth();
      if (IsFloatingBox())
      {
        inlineEnd += MarginEnd();
      }
      return inlineEnd;
    }

    KRYS_NODISCARD bool IsStartAligned() const noexcept
    {
      return _isStartAligned;
    }

  private:
    KRYS_NODISCARD LayoutUnit BorderBoxWidth() const noexcept
    {
      return _borderBoxWidth;
    }
    KRYS_NODISCARD LayoutUnit InitialInlineStart() const noexcept;

    KRYS_NODISCARD LayoutUnit MarginBefore() const noexcept
    {
      return _margin.Vertical.Before;
    }
    KRYS_NODISCARD LayoutUnit MarginAfter() const noexcept
    {
      return _margin.Vertical.After;
    }
    KRYS_NODISCARD LayoutUnit MarginStart() const noexcept
    {
      return _margin.Horizontal.Start;
    }
    KRYS_NODISCARD LayoutUnit MarginEnd() const noexcept
    {
      return _margin.Horizontal.End;
    }
    KRYS_NODISCARD LayoutUnit MarginBoxWidth() const noexcept
    {
      return MarginStart() + BorderBoxWidth() + MarginEnd();
    }

    KRYS_NODISCARD bool IsFloatingBox() const noexcept
    {
      return _isFloatingPositioned;
    }
  };
}
