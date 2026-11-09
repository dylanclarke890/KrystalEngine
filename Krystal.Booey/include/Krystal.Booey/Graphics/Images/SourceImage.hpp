/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class ImageBuffer;
  class NativeImage;

  /// @brief An image a pattern tiles: a native image or an image buffer.
  class SourceImage
  {
  public:
    using ImageVariant = Variant<Ref<NativeImage>, Ref<ImageBuffer>>;

  private:
    ImageVariant _imageVariant;
    mutable Maybe<ImageVariant> _transformedImageVariant;

  public:
    SourceImage(ImageVariant &&) noexcept;

    SourceImage(const SourceImage &) noexcept;
    SourceImage(SourceImage &&) noexcept;
    SourceImage &operator=(const SourceImage &) noexcept;
    SourceImage &operator=(SourceImage &&) noexcept;
    ~SourceImage() noexcept;

    KRYS_NODISCARD bool operator==(const SourceImage &) const noexcept;

    KRYS_NODISCARD NativeImage *NativeImageIfExists() const noexcept;
    KRYS_NODISCARD NativeImage *GetNativeImage() const noexcept;

    KRYS_NODISCARD ImageBuffer *ImageBufferIfExists() const noexcept;
    KRYS_NODISCARD ImageBuffer *GetImageBuffer() const noexcept;

    KRYS_NODISCARD RenderingResourceIdentifier ImageIdentifier() const noexcept;
    KRYS_NODISCARD IntSize Size() const noexcept;
  };
}
