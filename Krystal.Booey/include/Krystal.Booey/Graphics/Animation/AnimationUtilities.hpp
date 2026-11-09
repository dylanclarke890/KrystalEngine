/*
 * Copyright (C) 2011-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include <cmath>

namespace krys::boo
{
  /// @brief The progress and composite operations used to blend two values.
  struct BlendingContext
  {
    double Progress {0};
    bool IsDiscrete {false};
    boo::CompositeOperation CompositeOperation {boo::CompositeOperation::Replace};
    boo::IterationCompositeOperation IterationCompositeOperation {boo::IterationCompositeOperation::Replace};
    double CurrentIteration {0};
    Color FromCurrentColor {};
    Color ToCurrentColor {};

    BlendingContext(double progress = 0, bool isDiscrete = false,
                    boo::CompositeOperation compositeOperation = boo::CompositeOperation::Replace,
                    boo::IterationCompositeOperation iterationCompositeOperation =
                      boo::IterationCompositeOperation::Replace,
                    double currentIteration = 0, Color fromCurrentColor = {},
                    Color toCurrentColor = {}) noexcept
        : Progress(progress), IsDiscrete(isDiscrete), CompositeOperation(compositeOperation),
          IterationCompositeOperation(iterationCompositeOperation), CurrentIteration(currentIteration),
          FromCurrentColor(krys::move(fromCurrentColor)), ToCurrentColor(krys::move(toCurrentColor))
    {
    }

    KRYS_NODISCARD bool IsReplace() const noexcept
    {
      return CompositeOperation == boo::CompositeOperation::Replace
             && IterationCompositeOperation == boo::IterationCompositeOperation::Replace;
    }

    void NormalizeProgress() noexcept
    {
      // https://drafts.csswg.org/web-animations-1/#discrete
      // The property's values cannot be meaningfully combined, thus it is not additive and
      // interpolation swaps from Va to Vb at 50% (p=0.5).
      if (IsDiscrete)
      {
        Progress = Progress < 0.5 ? 0 : 1;
        CompositeOperation = boo::CompositeOperation::Replace;
      }
    }
  };

  KRYS_NODISCARD inline int32 Blend(int32 from, int32 to, const BlendingContext &context) noexcept
  {
    if (context.IterationCompositeOperation == IterationCompositeOperation::Accumulate
        && context.CurrentIteration)
    {
      auto iterationIncrement = static_cast<int32>(context.CurrentIteration * static_cast<double>(to));
      from += iterationIncrement;
      to += iterationIncrement;
    }

    if (context.CompositeOperation == CompositeOperation::Replace)
    {
      return static_cast<int32>(
        RoundTowardsPositiveInfinity(from + (static_cast<double>(to) - from) * context.Progress));
    }

    return static_cast<int32>(
      RoundTowardsPositiveInfinity(static_cast<double>(from) + static_cast<double>(from)
                                   + static_cast<double>(to - from) * context.Progress));
  }

  KRYS_NODISCARD inline uint32 Blend(uint32 from, uint32 to, const BlendingContext &context) noexcept
  {
    if (context.IterationCompositeOperation == IterationCompositeOperation::Accumulate
        && context.CurrentIteration)
    {
      auto iterationIncrement = static_cast<uint32>(context.CurrentIteration * static_cast<double>(to));
      from += iterationIncrement;
      to += iterationIncrement;
    }

    if (context.CompositeOperation == CompositeOperation::Replace)
    {
      return static_cast<uint32>(std::lround(from + (static_cast<double>(to) - from) * context.Progress));
    }

    return static_cast<uint32>(
      std::lround(from + from + (static_cast<double>(to) - from) * context.Progress));
  }

  KRYS_NODISCARD inline double Blend(double from, double to, const BlendingContext &context) noexcept
  {
    if (context.IterationCompositeOperation == IterationCompositeOperation::Accumulate
        && context.CurrentIteration)
    {
      auto iterationIncrement = context.CurrentIteration * to;
      from += iterationIncrement;
      to += iterationIncrement;
    }

    if (context.CompositeOperation == CompositeOperation::Replace)
    {
      return from + (to - from) * context.Progress;
    }

    return from + from + (to - from) * context.Progress;
  }

  KRYS_NODISCARD inline float Blend(float from, float to, const BlendingContext &context) noexcept
  {
    if (context.IterationCompositeOperation == IterationCompositeOperation::Accumulate
        && context.CurrentIteration)
    {
      auto iterationIncrement = static_cast<float>(context.CurrentIteration * to);
      from += iterationIncrement;
      to += iterationIncrement;
    }

    if (context.CompositeOperation == CompositeOperation::Replace)
    {
      return static_cast<float>(from + (to - from) * context.Progress);
    }

    return static_cast<float>(from + from + (to - from) * context.Progress);
  }

  KRYS_NODISCARD inline LayoutUnit Blend(LayoutUnit from, LayoutUnit to,
                                         const BlendingContext &context) noexcept
  {
    return LayoutUnit(Blend(from.ToFloat(), to.ToFloat(), context));
  }

  KRYS_NODISCARD inline IntPoint Blend(const IntPoint &from, const IntPoint &to,
                                       const BlendingContext &context) noexcept
  {
    return IntPoint(Blend(from.X(), to.X(), context), Blend(from.Y(), to.Y(), context));
  }

  KRYS_NODISCARD inline FloatPoint Blend(const FloatPoint &from, const FloatPoint &to,
                                         const BlendingContext &context) noexcept
  {
    return FloatPoint(Blend(from.X(), to.X(), context), Blend(from.Y(), to.Y(), context));
  }

  KRYS_NODISCARD inline LayoutPoint Blend(const LayoutPoint &from, const LayoutPoint &to,
                                          const BlendingContext &context) noexcept
  {
    return LayoutPoint(Blend(from.X(), to.X(), context), Blend(from.Y(), to.Y(), context));
  }
}
