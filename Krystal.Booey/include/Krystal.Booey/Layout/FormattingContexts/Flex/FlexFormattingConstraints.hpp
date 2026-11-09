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
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief The flex container's constraints along its main and cross axes.
  class ConstraintsForFlexContent
  {
  public:
    struct AxisGeometry
    {
      Maybe<LayoutUnit> MinimumSize;
      Maybe<LayoutUnit> MaximumSize;
      Maybe<LayoutUnit> AvailableSize;
      LayoutUnit StartPosition;
    };

  private:
    AxisGeometry _mainAxisGeometry;
    AxisGeometry _crossAxisGeometry;
    bool _isSizedUnderMinMax {false};

  public:
    ConstraintsForFlexContent(const AxisGeometry &mainAxis, const AxisGeometry &crossAxis,
                              bool isSizedUnderMinMax) noexcept
        : _mainAxisGeometry(mainAxis), _crossAxisGeometry(crossAxis), _isSizedUnderMinMax(isSizedUnderMinMax)
    {
    }

    KRYS_NODISCARD const AxisGeometry &MainAxis() const noexcept
    {
      return _mainAxisGeometry;
    }
    KRYS_NODISCARD const AxisGeometry &CrossAxis() const noexcept
    {
      return _crossAxisGeometry;
    }
    KRYS_NODISCARD bool IsSizedUnderMinMax() const noexcept
    {
      return _isSizedUnderMinMax;
    }
  };
}
