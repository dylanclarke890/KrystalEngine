/*
 * Copyright (C) 2024-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/ImageSource.hpp"

namespace krys::boo
{
  /// @brief The source of an image made from a native image: one frame, already decoded.
  class NativeImageSource : public ImageSource
  {
  private:
    ImageFrame _frame;

  public:
    KRYS_NODISCARD static Ref<NativeImageSource> Create(Ref<NativeImage> &&) noexcept;

  private:
    explicit NativeImageSource(Ref<NativeImage> &&) noexcept;

    KRYS_NODISCARD IntSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept final
    {
      return _frame.Size();
    }
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept final
    {
      return _frame.GetNativeImage()->SinglePixelSolidColor();
    }

    const ImageFrame &PrimaryImageFrame(const Maybe<SubsamplingLevel> & = null) noexcept final
    {
      return _frame;
    }
    KRYS_NODISCARD RefPtr<NativeImage> PrimaryNativeImage() noexcept final
    {
      return _frame.GetNativeImage();
    }

    void Dump(TextStream &) const noexcept final;
  };
}
