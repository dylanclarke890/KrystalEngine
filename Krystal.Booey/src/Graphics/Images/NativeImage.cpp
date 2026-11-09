/*
 * Copyright (C) 2020-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Colour/ColourUtilities.hpp"

namespace krys::boo
{
  RefPtr<NativeImage> NativeImage::Create(PlatformImagePtr &&platformImage) noexcept
  {
    if (!platformImage)
    {
      return nullptr;
    }

    return AdoptRef(*new NativeImage(krys::move(platformImage)));
  }

  RefPtr<NativeImage> NativeImage::CreateTransient(PlatformImagePtr &&image) noexcept
  {
    return Create(krys::move(image));
  }

  NativeImage::NativeImage(PlatformImagePtr &&platformImage) noexcept
      : _platformImage(krys::move(platformImage))
  {
  }

  NativeImage::~NativeImage() noexcept = default;

  const PlatformImagePtr &NativeImage::GetPlatformImage() const noexcept
  {
    return _platformImage;
  }

  IntSize NativeImage::Size() const noexcept
  {
    return _platformImage ? _platformImage->Size() : IntSize();
  }

  bool NativeImage::HasAlpha() const noexcept
  {
    // Decoded images keep an alpha channel.
    return true;
  }

  Maybe<Color> NativeImage::SinglePixelSolidColor() const noexcept
  {
    if (Size() != IntSize(1, 1) || _platformImage->External())
    {
      return null;
    }

    auto pixel = AsSRGBA(PackedColour::ARGB {_platformImage->Pixels()[0]});
    if (_platformImage->PremultipliedAlpha())
    {
      pixel = Unpremultiplied(pixel);
    }
    return Color(pixel);
  }

  void NativeImage::ReplacePlatformImage(PlatformImagePtr &&platformImage) noexcept
  {
    krys_debug_assert(platformImage);
    _platformImage = krys::move(platformImage);
  }
}
