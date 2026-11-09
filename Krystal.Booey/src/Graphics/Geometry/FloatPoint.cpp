/*
 * Copyright (C) 2004, 2006, 2007 Apple Inc. All rights reserved.
 * Copyright (C) 2005 Nokia. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Core/Text/TextStream.hpp"

namespace krys::boo
{
  FloatPoint FloatPoint::ConstrainedWithin(const FloatRect &rect) const noexcept
  {
    return ConstrainedBetween(rect.MinXMinYCorner(), rect.MaxXMaxYCorner());
  }

  FloatPoint FloatPoint::MatrixTransform(const AffineTransform &transform) const noexcept
  {
    double newX, newY;
    transform.Map(static_cast<double>(_x), static_cast<double>(_y), newX, newY);
    return NarrowPrecision(newX, newY);
  }

  FloatPoint FloatPoint::MatrixTransform(const TransformationMatrix &transform) const noexcept
  {
    double newX, newY;
    transform.Map(static_cast<double>(_x), static_cast<double>(_y), newX, newY);
    return NarrowPrecision(newX, newY);
  }

  TextStream &operator<<(TextStream &ts, const FloatPoint &p) noexcept
  {
    // FIXME: callers should use the NumberRespectingIntegers flag.
    return ts << '(' << TextStream::FormatNumberRespectingIntegers(p.X()) << ','
              << TextStream::FormatNumberRespectingIntegers(p.Y()) << ')';
  }
}
