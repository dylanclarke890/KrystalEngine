/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
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

#include "Krystal.Booey/DOM/Types/Uint8ClampedArray.hpp"
#include "Krystal.Booey/HTML/ImageData/ImageDataPixelFormat.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  /// @brief An `ImageData`'s pixels, four channels each, row after row.
  /// FIXME: A `Float16Array` for `rgba-float16`; the pixels are 8 bits a channel whichever format is asked
  /// for, and the format is only remembered.
  class ImageDataArray
  {
  private:
    Ref<dom::Uint8ClampedArray> _data;
    ImageDataPixelFormat _pixelFormat {ImageDataPixelFormat::RgbaUnorm8};

  public:
    explicit ImageDataArray(Ref<dom::Uint8ClampedArray> &&data,
                            ImageDataPixelFormat pixelFormat = ImageDataPixelFormat::RgbaUnorm8) noexcept
        : _data(krys::move(data)), _pixelFormat(pixelFormat)
    {
    }
    ImageDataArray(ImageDataArray &&original, Maybe<ImageDataPixelFormat> overridingPixelFormat) noexcept
        : _data(krys::move(original._data)),
          _pixelFormat(overridingPixelFormat.value_or(original._pixelFormat))
    {
    }

    /// @brief @p length channels of @p pixelFormat, a copy of @p optionalBytes or zero when it's empty.
    KRYS_NODISCARD static Maybe<ImageDataArray> TryCreate(size_t length, ImageDataPixelFormat pixelFormat,
                                                          Span<const uint8> optionalBytes = {}) noexcept;

    KRYS_NODISCARD ImageDataPixelFormat PixelFormat() const noexcept
    {
      return _pixelFormat;
    }
    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _data->Length();
    }
    KRYS_NODISCARD size_t ByteLength() const noexcept
    {
      return _data->ByteLength();
    }
    /// @brief Whether the pixels were given away; never, as there are no scripts to transfer them.
    KRYS_NODISCARD bool IsDetached() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD Ref<dom::Uint8ClampedArray> AsUint8ClampedArray() const noexcept
    {
      return _data;
    }
  };
}
