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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::layout
{
  struct HorizontalConstraints
  {
    KRYS_NODISCARD LayoutUnit LogicalRight() const noexcept
    {
      return LogicalLeft + LogicalWidth;
    }

    LayoutUnit LogicalLeft;
    LayoutUnit LogicalWidth;
  };

  struct VerticalConstraints
  {
    LayoutUnit LogicalTop;
    LayoutUnit LogicalHeight;
  };

  /// @brief The space in-flow content is laid out in: the containing block's content box's inline extent and
  /// block start.
  class ConstraintsForInFlowContent
  {
  public:
    enum BaseTypeFlag : uint8
    {
      GenericContent,
      InlineContent,
      TableContent,
      FlexContent
    };

  private:
    uint8 _baseTypeFlags : 4; // EnumSet<BaseTypeFlag>
    HorizontalConstraints _horizontal;
    LayoutUnit _logicalTop;

  public:
    ConstraintsForInFlowContent(HorizontalConstraints horizontal, LayoutUnit logicalTop) noexcept
        : ConstraintsForInFlowContent(horizontal, logicalTop, GenericContent)
    {
    }

    KRYS_NODISCARD HorizontalConstraints Horizontal() const noexcept
    {
      return _horizontal;
    }
    KRYS_NODISCARD LayoutUnit LogicalTop() const noexcept
    {
      return _logicalTop;
    }

    KRYS_NODISCARD bool IsConstraintsForInlineContent() const noexcept
    {
      return BaseTypeFlags().Contains(InlineContent);
    }
    KRYS_NODISCARD bool IsConstraintsForTableContent() const noexcept
    {
      return BaseTypeFlags().Contains(TableContent);
    }
    KRYS_NODISCARD bool IsConstraintsForFlexContent() const noexcept
    {
      return BaseTypeFlags().Contains(FlexContent);
    }

  protected:
    ConstraintsForInFlowContent(HorizontalConstraints horizontal, LayoutUnit logicalTop,
                                EnumSet<BaseTypeFlag> baseTypeFlags) noexcept
        : _baseTypeFlags(static_cast<uint8>(baseTypeFlags.ToRaw())), _horizontal(horizontal),
          _logicalTop(logicalTop)
    {
    }

  private:
    KRYS_NODISCARD EnumSet<BaseTypeFlag> BaseTypeFlags() const noexcept
    {
      return EnumSet<BaseTypeFlag>::FromRaw(_baseTypeFlags);
    }
  };

  struct ConstraintsForOutOfFlowContent
  {
    HorizontalConstraints Horizontal;
    VerticalConstraints Vertical;
    // Borders and padding are resolved against the containing block's content box as if the box was an
    // in-flow box.
    LayoutUnit BorderAndPaddingConstraints;
  };

  enum class IntrinsicWidthMode : uint8
  {
    Minimum,
    Maximum
  };

  struct IntrinsicWidthConstraints
  {
    void Expand(LayoutUnit horizontalValue) noexcept
    {
      Minimum += horizontalValue;
      Maximum += horizontalValue;
    }

    IntrinsicWidthConstraints &operator+=(const IntrinsicWidthConstraints &other) noexcept
    {
      Minimum += other.Minimum;
      Maximum += other.Maximum;
      return *this;
    }

    IntrinsicWidthConstraints &operator+=(LayoutUnit value) noexcept
    {
      Expand(value);
      return *this;
    }

    IntrinsicWidthConstraints &operator-=(const IntrinsicWidthConstraints &other) noexcept
    {
      Minimum -= other.Minimum;
      Maximum -= other.Maximum;
      return *this;
    }

    IntrinsicWidthConstraints &operator-=(LayoutUnit value) noexcept
    {
      Expand(-value);
      return *this;
    }

    LayoutUnit Minimum;
    LayoutUnit Maximum;
  };
}
