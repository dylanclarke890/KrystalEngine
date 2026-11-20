/*
 * Copyright (C) 2005-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2016-2020 Google Inc. All rights reserved.
 * Copyright (C) 2009 Torch Mobile, Inc.
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

#include "Krystal.Booey/Graphics/Transforms/LayoutTransforms.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutRect.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD float ClampEdgeValue(float f) noexcept
    {
      krys_debug_assert(!std::isnan(f));
      return std::min<float>(std::max<float>(f, -LayoutUnit::Max() / 2), LayoutUnit::Max() / 2);
    }
  }

  LayoutRect ClampedBoundsOfProjectedQuad(const TransformationMatrix &transform, const QuadF &q) noexcept
  {
    RectF mappedQuadBounds = transform.ProjectQuad(q).BoundingBox();

    float left = ClampEdgeValue(std::floor(mappedQuadBounds.X()));
    float top = ClampEdgeValue(std::floor(mappedQuadBounds.Y()));

    float right;
    if (std::isinf(mappedQuadBounds.X()) && std::isinf(mappedQuadBounds.Width()))
    {
      right = LayoutUnit::Max() / 2;
    }
    else
    {
      right = ClampEdgeValue(std::ceil(mappedQuadBounds.MaxX()));
    }

    float bottom;
    if (std::isinf(mappedQuadBounds.Y()) && std::isinf(mappedQuadBounds.Height()))
    {
      bottom = LayoutUnit::Max() / 2;
    }
    else
    {
      bottom = ClampEdgeValue(std::ceil(mappedQuadBounds.MaxY()));
    }

    return LayoutRect(LayoutUnit::Clamp(left), LayoutUnit::Clamp(top), LayoutUnit::Clamp(right - left),
                      LayoutUnit::Clamp(bottom - top));
  }

  LayoutRect MapRect(const TransformationMatrix &transform, const LayoutRect &r) noexcept
  {
    return EnclosingLayoutRect(transform.MapRect(RectF(r)));
  }
}
