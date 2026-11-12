/*
 * Copyright (C) 2004, 2005, 2006 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class Image;
  class IntRect;
  struct Settings;

  /// @brief Interface for notification about changes to an image, including decoding, drawing, and animating.
  class ImageObserver : public RefCounted<ImageObserver>, public CanMakeWeakPtr<ImageObserver>
  {
  public:
    virtual ~ImageObserver() noexcept = default;

    KRYS_NODISCARD virtual url::URL SourceUrl() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string MimeType() const noexcept = 0;
    KRYS_NODISCARD virtual uint32 NumberOfClients() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual int64 ExpectedContentLength() const noexcept = 0;

    virtual void EncodedDataStatusChanged(const Image &, EncodedDataStatus) noexcept
    {
    }
    virtual void DecodedSizeChanged(const Image &, int64 delta) noexcept = 0;

    virtual void DidDraw(const Image &) noexcept = 0;
    KRYS_NODISCARD virtual bool CanDestroyDecodedData(const Image &) const noexcept
    {
      return true;
    }

    virtual void ImageFrameAvailable(const Image &, ImageAnimatingState, const IntRect *changeRect = nullptr,
                                     DecodingStatus = DecodingStatus::Invalid) noexcept = 0;
    virtual void ChangedInRect(const Image &, const IntRect *changeRect = nullptr) noexcept = 0;
    virtual void ImageContentChanged(const Image &) noexcept = 0;
    virtual void ScheduleRenderingUpdate(const Image &) noexcept = 0;

    KRYS_NODISCARD virtual bool AllowsAnimation(const Image &) const noexcept
    {
      return true;
    }

    /// @brief The settings of the document that loaded the image, for images that are documents themselves
    /// (SVG images); null if there's none.
    KRYS_NODISCARD virtual Settings *GetSettings() const noexcept
    {
      return nullptr;
    }

    /// @brief Whether what shows the image is drawn with the dark appearance.
    KRYS_NODISCARD virtual bool UseSystemDarkAppearance() const noexcept
    {
      return false;
    }

  protected:
    ImageObserver() noexcept = default;
  };
}
