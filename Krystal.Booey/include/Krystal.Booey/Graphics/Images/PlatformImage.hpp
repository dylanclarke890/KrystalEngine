/*
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/Span.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Image/PixelData.hpp"

namespace krys::boo
{
  /// @brief An image the host holds outside Booey, such as a texture on its GPU: only the host's renderer
  /// can draw it, and Booey can't read its pixels.
  class ExternalImage : public RefCounted<ExternalImage>
  {
    KRYS_NON_COPY_MOVABLE(ExternalImage);

  public:
    virtual ~ExternalImage() noexcept = default;

  protected:
    ExternalImage() noexcept = default;
  };

  /// @brief The platform's image: decoded sRGB pixels in memory, which the host's renderer uploads when it
  /// draws them, or an image the host holds itself.
  /// @note What `NativeImage` wraps, as Skia's `SkImage` or Cairo's surface does.
  class PlatformImage final : public RefCounted<PlatformImage>
  {
    KRYS_NON_COPY_MOVABLE(PlatformImage);

  private:
    SizeI _size;
    RefPtr<img::PixelData> _pixels;
    RefPtr<ExternalImage> _external;
    bool _premultipliedAlpha;

  public:
    PlatformImage(const SizeI &size, Ref<img::PixelData> &&pixels, bool premultipliedAlpha) noexcept
        : _size(size), _pixels(krys::move(pixels)), _premultipliedAlpha(premultipliedAlpha)
    {
    }

    PlatformImage(const SizeI &size, Ref<ExternalImage> &&external, bool premultipliedAlpha) noexcept
        : _size(size), _external(krys::move(external)), _premultipliedAlpha(premultipliedAlpha)
    {
    }

    KRYS_NODISCARD const SizeI &Size() const noexcept
    {
      return _size;
    }

    /// @brief The pixels, `Size().Width()` a row, as ARGB (alpha in the high byte); empty for an external
    /// image.
    KRYS_NODISCARD Span<const uint32> Pixels() const noexcept
    {
      if (!_pixels)
      {
        return {};
      }
      return {_pixels->Pixels.data(), static_cast<size_t>(_size.UnclampedArea())};
    }

    /// @brief The host's image, if this is one.
    KRYS_NODISCARD ExternalImage *External() const noexcept
    {
      return _external.get();
    }

    KRYS_NODISCARD bool PremultipliedAlpha() const noexcept
    {
      return _premultipliedAlpha;
    }
  };

  using PlatformImagePtr = RefPtr<PlatformImage>;

  /// @brief A platform image of a decoder's frame, sharing its pixels; null without one.
  KRYS_NODISCARD inline PlatformImagePtr CreatePlatformImage(Maybe<img::DecodedImage> image) noexcept
  {
    if (!image)
    {
      return nullptr;
    }
    return AdoptRef(*new PlatformImage(image->Size, krys::move(image->Pixels), image->PremultipliedAlpha));
  }
}
