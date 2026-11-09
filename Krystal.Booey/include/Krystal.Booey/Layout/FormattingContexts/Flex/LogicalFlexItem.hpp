/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief A flex item with its geometry resolved along the flex container's main and cross axes.
  class LogicalFlexItem
  {
  public:
    struct MainAxisGeometry
    {
      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return MarginStart.value_or(0_lu) + MarginEnd.value_or(0_lu);
      }

      Maybe<LayoutUnit> DefiniteFlexBasis;
      Maybe<LayoutUnit> Size;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MarginStart;
      Maybe<LayoutUnit> MarginEnd;
      LayoutUnit BorderAndPadding;
    };

    struct CrossAxisGeometry
    {
      KRYS_NODISCARD LayoutUnit Margin() const noexcept
      {
        return MarginStart.value_or(0_lu) + MarginEnd.value_or(0_lu);
      }
      KRYS_NODISCARD bool HasNonAutoMargins() const noexcept
      {
        return MarginStart && MarginEnd;
      }

      Maybe<LayoutUnit> DefiniteSize;
      LayoutUnit Ascent;
      LayoutUnit Descent;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MarginStart;
      Maybe<LayoutUnit> MarginEnd;
      LayoutUnit BorderAndPadding;
      bool HasSizeAuto {false};
    };

  private:
    CheckedPtr<const ElementBox> _layoutBox;
    MainAxisGeometry _mainAxisGeometry;
    CrossAxisGeometry _crossAxisGeometry;
    bool _hasAspectRatio {false};
    bool _isOrthogonal {false};

  public:
    LogicalFlexItem(const ElementBox &flexItem, const MainAxisGeometry &mainGeometry,
                    const CrossAxisGeometry &crossGeometry, bool hasAspectRatio, bool isOrthogonal) noexcept
        : _layoutBox(ShareCheckedPtr(&flexItem)), _mainAxisGeometry(mainGeometry),
          _crossAxisGeometry(crossGeometry), _hasAspectRatio(hasAspectRatio), _isOrthogonal(isOrthogonal)
    {
    }
    LogicalFlexItem() noexcept = default;

    KRYS_NODISCARD const MainAxisGeometry &MainAxis() const noexcept
    {
      return _mainAxisGeometry;
    }
    KRYS_NODISCARD const CrossAxisGeometry &CrossAxis() const noexcept
    {
      return _crossAxisGeometry;
    }

    KRYS_NODISCARD float GrowFactor() const noexcept
    {
      return Style().GetFlexGrow().Value;
    }
    KRYS_NODISCARD float ShrinkFactor() const noexcept
    {
      return Style().GetFlexShrink().Value;
    }

    KRYS_NODISCARD bool HasContentFlexBasis() const noexcept
    {
      return Style().GetFlexBasis().IsContent();
    }
    KRYS_NODISCARD bool HasAvailableSpaceDependentFlexBasis() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD bool HasAspectRatio() const noexcept
    {
      return _hasAspectRatio;
    }
    KRYS_NODISCARD bool IsOrthogonal() const noexcept
    {
      return _isOrthogonal;
    }
    KRYS_NODISCARD bool IsContentBoxBased() const noexcept
    {
      return Style().GetBoxSizing() == BoxSizing::ContentBox;
    }

    KRYS_NODISCARD const ElementBox &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return LayoutBox().Style();
    }
    KRYS_NODISCARD boo::WritingMode GetWritingMode() const noexcept
    {
      return Style().GetWritingMode();
    }
  };
}
