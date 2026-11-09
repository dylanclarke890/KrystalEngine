/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2012-2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo
{
  /// @brief How an image is rotated and mirrored when drawn, as the EXIF orientation values.
  struct ImageOrientation
  {
    enum class Orientation : uint8
    {
      FromImage = 0, // Orientation from the image should be respected.

      // This range intentionally matches the orientation values from the EXIF spec.
      // See JEITA CP-3451, page 18. http://www.exif.org/Exif2-2.PDF
      OriginTopLeft = 1,     // default
      OriginTopRight = 2,    // mirror along y-axis
      OriginBottomRight = 3, // 180 degree rotation
      OriginBottomLeft = 4,  // mirror along the x-axis
      OriginLeftTop = 5,     // mirror along x-axis + 270 degree CW rotation
      OriginRightTop = 6,    // 90 degree CW rotation
      OriginRightBottom = 7, // mirror along x-axis + 90 degree CW rotation
      OriginLeftBottom = 8,  // 270 degree CW rotation

      None = OriginTopLeft
    };

  private:
    static constexpr auto EXIFFirst = Orientation::OriginTopLeft;
    static constexpr auto EXIFLast = Orientation::OriginLeftBottom;
    static constexpr auto First = Orientation::FromImage;
    static constexpr auto Last = EXIFLast;

    Orientation _orientation {Orientation::None};

  public:
    constexpr ImageOrientation() noexcept = default;

    constexpr ImageOrientation(Orientation orientation) noexcept : _orientation(orientation)
    {
    }

    explicit ImageOrientation(int orientation) noexcept
    {
      krys_assert(IsValidOrientation(orientation));
      _orientation = static_cast<Orientation>(orientation);
    }

    KRYS_NODISCARD static constexpr Orientation FromEXIFValue(int exifValue) noexcept
    {
      return IsValidEXIFOrientation(exifValue) ? static_cast<Orientation>(exifValue) : Orientation::None;
    }

    KRYS_NODISCARD constexpr operator int() const noexcept
    {
      return static_cast<int>(_orientation);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const ImageOrientation &,
                                                    const ImageOrientation &) noexcept = default;

    KRYS_NODISCARD constexpr Orientation GetOrientation() const noexcept
    {
      return _orientation;
    }

    KRYS_NODISCARD constexpr bool UsesWidthAsHeight() const noexcept
    {
      krys_debug_assert(_orientation != Orientation::FromImage);
      // Values 5 through 8 all flip the width/height.
      switch (_orientation)
      {
        case Orientation::OriginLeftTop:
        case Orientation::OriginRightTop:
        case Orientation::OriginRightBottom:
        case Orientation::OriginLeftBottom:  return true;
        default:                             return false;
      }
    }

    KRYS_NODISCARD constexpr AffineTransform TransformFromDefault(const FloatSize &drawnSize) const noexcept
    {
      float w = drawnSize.Width();
      float h = drawnSize.Height();

      switch (_orientation)
      {
        case Orientation::FromImage:         krys_debug_assert_msg(false, "Unreachable"); return AffineTransform();
        case Orientation::OriginTopLeft:     return AffineTransform();
        case Orientation::OriginTopRight:    return AffineTransform(-1, 0, 0, 1, w, 0);
        case Orientation::OriginBottomRight: return AffineTransform(-1, 0, 0, -1, w, h);
        case Orientation::OriginBottomLeft:  return AffineTransform(1, 0, 0, -1, 0, h);
        case Orientation::OriginLeftTop:     return AffineTransform(0, 1, 1, 0, 0, 0);
        case Orientation::OriginRightTop:    return AffineTransform(0, 1, -1, 0, w, 0);
        case Orientation::OriginRightBottom: return AffineTransform(0, -1, -1, 0, w, h);
        case Orientation::OriginLeftBottom:  return AffineTransform(0, -1, 1, 0, 0, h);
      }

      krys_debug_assert_msg(false, "Unreachable");
      return AffineTransform();
    }

    KRYS_NODISCARD constexpr ImageOrientation WithFlippedY() const noexcept
    {
      krys_debug_assert(_orientation != Orientation::FromImage);

      switch (_orientation)
      {
        case Orientation::FromImage:         krys_debug_assert_msg(false, "Unreachable"); return Orientation::None;
        case Orientation::OriginTopLeft:     return Orientation::OriginBottomLeft;
        case Orientation::OriginTopRight:    return Orientation::OriginBottomRight;
        case Orientation::OriginBottomRight: return Orientation::OriginTopRight;
        case Orientation::OriginBottomLeft:  return Orientation::OriginTopLeft;
        case Orientation::OriginLeftTop:     return Orientation::OriginLeftBottom;
        case Orientation::OriginRightTop:    return Orientation::OriginRightBottom;
        case Orientation::OriginRightBottom: return Orientation::OriginRightTop;
        case Orientation::OriginLeftBottom:  return Orientation::OriginLeftTop;
      }

      krys_debug_assert_msg(false, "Unreachable");
      return Orientation::None;
    }

  private:
    KRYS_NODISCARD static constexpr bool IsValidOrientation(int orientation) noexcept
    {
      return orientation >= static_cast<int>(First) && orientation <= static_cast<int>(Last);
    }

    KRYS_NODISCARD static constexpr bool IsValidEXIFOrientation(int orientation) noexcept
    {
      return orientation >= static_cast<int>(EXIFFirst) && orientation <= static_cast<int>(EXIFLast);
    }
  };
}
