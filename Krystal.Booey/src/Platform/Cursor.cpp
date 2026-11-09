/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Platform/Cursor.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntRect.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD IntPoint DetermineHotSpot(const Image &image, const IntPoint &specifiedHotSpot,
                                             float scale) noexcept
    {
      if (image.IsNull())
      {
        return {};
      }

      // Hot spot must be inside cursor rectangle.
      auto size = image.Size();
      IntRect imageRect(IntPoint(), IntSize(static_cast<int32>(size.Width() / scale),
                                            static_cast<int32>(size.Height() / scale)));
      if (imageRect.Contains(specifiedHotSpot))
      {
        return specifiedHotSpot;
      }

      // If hot spot is not specified externally, it can be extracted from some image formats (e.g. .cur).
      if (auto intrinsicHotSpot = image.HotSpot())
      {
        if (imageRect.Contains(*intrinsicHotSpot))
        {
          return *intrinsicHotSpot;
        }
      }

      return {};
    }
  }

  Cursor::Cursor(Image &image, const IntPoint &hotSpot, float scale) noexcept
      : _type(Type::Custom), _image(ShareRefPtr(&image)), _hotSpot(DetermineHotSpot(image, hotSpot, scale)),
        _imageScaleFactor(scale)
  {
  }
}
