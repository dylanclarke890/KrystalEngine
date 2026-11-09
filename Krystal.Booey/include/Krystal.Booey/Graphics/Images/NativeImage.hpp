/*
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2007-2008 Torch Mobile, Inc.
 * Copyright (C) 2012 Company 100 Inc.
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
#include "Krystal.Booey/Graphics/Images/PlatformImage.hpp"
#include "Krystal.Booey/Graphics/RenderingResourceIdentifier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Color;

  /// @brief A decoded image frame the graphics context draws: the platform's image, identified for the
  /// renderer that caches it.
  // FIXME: Colour spaces other than sRGB, HDR headroom and rendering resource observers, once there are
  // any.
  class NativeImage : public RefCounted<NativeImage>, public CanMakeWeakPtr<NativeImage>
  {
    KRYS_NON_COPY_MOVABLE(NativeImage);

  protected:
    mutable PlatformImagePtr _platformImage;
    RenderingResourceIdentifier _renderingResourceIdentifier {RenderingResourceIdentifier::Generate()};

  public:
    KRYS_NODISCARD static RefPtr<NativeImage> Create(PlatformImagePtr &&platformImage) noexcept;
    /// @brief A native image that's drawn once or a few times.
    KRYS_NODISCARD static RefPtr<NativeImage> CreateTransient(PlatformImagePtr &&platformImage) noexcept;

    virtual ~NativeImage() noexcept;

    KRYS_NODISCARD virtual const PlatformImagePtr &GetPlatformImage() const noexcept;
    KRYS_NODISCARD virtual IntSize Size() const noexcept;
    KRYS_NODISCARD virtual bool HasAlpha() const noexcept;
    /// @brief The colour of a 1x1 image, which can be drawn as a fill.
    KRYS_NODISCARD Maybe<Color> SinglePixelSolidColor() const noexcept;

    void ReplacePlatformImage(PlatformImagePtr &&platformImage) noexcept;

    KRYS_NODISCARD RenderingResourceIdentifier GetRenderingResourceIdentifier() const noexcept
    {
      return _renderingResourceIdentifier;
    }

  protected:
    explicit NativeImage(PlatformImagePtr &&platformImage) noexcept;
  };
}
