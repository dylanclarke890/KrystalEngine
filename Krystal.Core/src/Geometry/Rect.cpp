/*
 * Copyright (C) 2003, 2006, 2009 Apple Inc. All rights reserved.
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
/*
 * Copyright (C) 2003, 2006, 2007 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Strings/TextStream.hpp"

namespace krys
{
  RectF EncloseRectToDevicePixels(const RectF &rect, float deviceScaleFactor) noexcept
  {
    PointF location = FloorPointToDevicePixels(rect.MinXMinYCorner(), deviceScaleFactor);
    PointF maxPoint = CeilPointToDevicePixels(rect.MaxXMaxYCorner(), deviceScaleFactor);
    return RectF(location, maxPoint - location);
  }

  RectI EnclosingRectI(const RectF &rect) noexcept
  {
    PointF location = FlooredPointI(rect.MinXMinYCorner());
    PointF maxPoint = CeiledPointI(rect.MaxXMaxYCorner());
    return RectI(PointI(location), SizeI(maxPoint - location));
  }

  RectI EnclosingRectIPreservingEmptyRects(const RectF &rect) noexcept
  {
    // Empty rects with fractional x, y values turn into non-empty rects when converting to enclosing.
    // We want to ensure that empty rects stay empty after the conversion, since some callers
    // prefer this behavior.
    PointF location = FlooredPointI(rect.MinXMinYCorner());
    if (rect.IsEmpty())
    {
      return RectI(PointI(location), {});
    }
    PointF maxPoint = CeiledPointI(rect.MaxXMaxYCorner());
    return RectI(PointI(location), SizeI(maxPoint - location));
  }

  RectI RoundedRectI(const RectF &rect) noexcept
  {
    return RectI(RoundedPointI(rect.Location()), RoundedSizeI(rect.Size()));
  }

  TextStream &operator<<(TextStream &ts, const RectI &value) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::SVGStyleRect))
    {
      return ts << "at (" << value.X() << ',' << value.Y() << ") size " << value.Width() << 'x'
                << value.Height();
    }
    return ts << value.Location() << ' ' << value.Size();
  }

  TextStream &operator<<(TextStream &ts, const RectF &value) noexcept
  {
    if (ts.HasFormattingFlag(TextStream::Formatting::SVGStyleRect))
    {
      // FIXME: callers should use the NumberRespectingIntegers flag.
      return ts << "at (" << TextStream::FormatNumberRespectingIntegers(value.X()) << ','
                << TextStream::FormatNumberRespectingIntegers(value.Y()) << ") size "
                << TextStream::FormatNumberRespectingIntegers(value.Width()) << "x"
                << TextStream::FormatNumberRespectingIntegers(value.Height());
    }
    return ts << value.Location() << ' ' << value.Size();
  }
}
