/*
 * Copyright (C) 2006, 2016 Apple Inc. All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 * Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)
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
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Strings/String.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Image/EXIFMetadata.hpp"
#include "Krystal.Image/ImageDecoderFrame.hpp"
#include "Krystal.Image/ImageOrientation.hpp"
#include "Krystal.Image/ImageTypes.hpp"
#include "Krystal.Image/PixelData.hpp"

namespace krys::img
{
  /// @brief Decodes an image's encoded data, as it arrives, into its size, frames and frame metadata: the
  /// base of the format decoders (`PNGImageDecoder` and the others), which keeps their frame cache.
  // FIXME: The lock that guards the frame cache, once decoding moves off the main thread.
  class ImageDecoder : public RefCounted<ImageDecoder>
  {
    KRYS_NON_COPY_MOVABLE(ImageDecoder);

  protected:
    RefPtr<const SharedBuffer> _data;
    List<ImageDecoderFrame> _frameBufferCache;
    bool _premultiplyAlpha;
    bool _ignoreGammaAndColorProfile;
    ImageOrientation _orientation;
    Maybe<SizeI> _densityCorrectedSize;

  private:
    SizeI _size;
    EncodedDataStatus _encodedDataStatus {EncodedDataStatus::TypeAvailable};
    bool _decodingSizeFromSetData {false};

  public:
    /// @brief A decoder for @p data, sniffed from its first bytes, or null if no decoder supports it (or not
    /// enough has arrived to tell).
    KRYS_NODISCARD static RefPtr<ImageDecoder>
      Create(const SharedBuffer &data, AlphaOption alphaOption,
             GammaAndColorProfileOption gammaAndColorProfileOption) noexcept;

    virtual ~ImageDecoder() noexcept = default;

    /// @brief Whether there's a decoder for @p mimeType (PNG, JPEG, GIF, WebP, BMP and ICO), ignoring case.
    // FIXME: AVIF once it's supported.
    KRYS_NODISCARD static bool SupportsMimeType(utf8_stringview mimeType) noexcept;

    KRYS_NODISCARD bool PremultiplyAlpha() const noexcept
    {
      return _premultiplyAlpha;
    }

    KRYS_NODISCARD size_t BytesDecodedToDetermineProperties() const noexcept
    {
      return 0;
    }

    KRYS_NODISCARD bool IsAllDataReceived() const noexcept
    {
      krys_debug_assert(!_decodingSizeFromSetData);
      return _encodedDataStatus == EncodedDataStatus::Complete;
    }

    virtual void SetData(const SharedBuffer &data, bool allDataReceived) noexcept
    {
      if (_encodedDataStatus == EncodedDataStatus::Error)
      {
        return;
      }

      _data = ShareRefPtr(&data);

      if (_encodedDataStatus == EncodedDataStatus::TypeAvailable)
      {
        _decodingSizeFromSetData = true;
        TryDecodeSize(allDataReceived);
        _decodingSizeFromSetData = false;
      }

      if (_encodedDataStatus == EncodedDataStatus::Error)
      {
        return;
      }

      if (allDataReceived)
      {
        krys_debug_assert(_encodedDataStatus == EncodedDataStatus::SizeAvailable);
        _encodedDataStatus = EncodedDataStatus::Complete;
      }
    }

    KRYS_NODISCARD EncodedDataStatus GetEncodedDataStatus() const noexcept
    {
      return _encodedDataStatus;
    }

    KRYS_NODISCARD bool IsSizeAvailable() const noexcept
    {
      return _encodedDataStatus >= EncodedDataStatus::SizeAvailable;
    }

    KRYS_NODISCARD virtual SizeI Size() const noexcept
    {
      return IsSizeAvailable() ? _size : SizeI();
    }

    // This will only differ from size() for ICO (where each frame is a
    // different icon) or other formats where different frames are different
    // sizes. This does NOT differ from size() for GIF, since decoding GIFs
    // composites any smaller frames against previous frames to create full-
    // size frames.
    KRYS_NODISCARD virtual SizeI FrameSizeAtIndex(size_t) const noexcept
    {
      return Size();
    }

    // Returns whether the size is legal (i.e. not going to result in
    // overflow elsewhere). If not, marks decoding as failed.
    virtual bool SetSize(const SizeI &size) noexcept
    {
      if (ImageBackingStore::IsOverSize(size))
      {
        return SetFailed();
      }
      _size = size;
      _encodedDataStatus = EncodedDataStatus::SizeAvailable;
      return true;
    }

    // Lazily-decodes enough of the image to get the frame count (if
    // possible), without decoding the individual frames.
    // FIXME: Right now that has to be done by each subclass; factor the
    // decode call out and use it here.
    KRYS_NODISCARD virtual size_t FrameCount() const noexcept
    {
      return 1;
    }

    KRYS_NODISCARD size_t PrimaryFrameIndex() const noexcept
    {
      return 0;
    }

    KRYS_NODISCARD virtual RepetitionCount GetRepetitionCount() const noexcept
    {
      return RepetitionCountNone;
    }

    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept = 0;

    KRYS_NODISCARD utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }

    // Decodes as much of the requested frame as possible, and returns an
    // ImageDecoder-owned pointer.
    virtual ImageDecoderFrame *FrameBufferAtIndex(size_t index) noexcept = 0;

    KRYS_NODISCARD bool FrameIsCompleteAtIndex(size_t index) const noexcept;

    // Make the best effort guess to check if the requested frame has alpha channel.
    KRYS_NODISCARD bool FrameHasAlphaAtIndex(size_t index) const noexcept;

    KRYS_NODISCARD seconds FrameDurationAtIndex(size_t index) const noexcept;

    /// @brief The frame at @p index, decoded as far as its data allows, sharing its pixels with the decoder;
    /// none if it can't be decoded.
    KRYS_NODISCARD Maybe<DecodedImage> CreateFrameImageAtIndex(size_t index) noexcept;

    void SetIgnoreGammaAndColorProfile(bool flag) noexcept
    {
      _ignoreGammaAndColorProfile = flag;
    }

    KRYS_NODISCARD bool IgnoresGammaAndColorProfile() const noexcept
    {
      return _ignoreGammaAndColorProfile;
    }

    KRYS_NODISCARD ImageOrientation FrameOrientationAtIndex(size_t) const noexcept
    {
      return _orientation;
    }

    KRYS_NODISCARD Maybe<SizeI> FrameDensityCorrectedSizeAtIndex(size_t) const noexcept
    {
      return _densityCorrectedSize;
    }

    /// @brief Takes the image's orientation and density-corrected size from its EXIF metadata.
    void SetEXIFMetadata(const EXIFMetadata &metadata) noexcept
    {
      _orientation = metadata.Orientation;
      _densityCorrectedSize = metadata.DensityCorrectedSize;
    }

    constexpr static size_t ICCColorProfileHeaderLength = 128;

    // Sets the "decode failure" flag. For caller convenience (since so
    // many callers want to return false after calling this), returns false
    // to enable easy tailcalling. Subclasses may override this to also
    // clean up any local data.
    virtual bool SetFailed() noexcept
    {
      _encodedDataStatus = EncodedDataStatus::Error;
      return false;
    }

    KRYS_NODISCARD bool Failed() const noexcept
    {
      return _encodedDataStatus == EncodedDataStatus::Error;
    }

    // Clears decoded pixel data from before the provided frame unless that
    // data may be needed to decode future frames (e.g. due to GIF frame
    // compositing).
    virtual void ClearFrameBufferCache(size_t) noexcept
    {
    }

    // If the image has a cursor hot-spot, stores it in the argument
    // and returns true. Otherwise returns false.
    KRYS_NODISCARD virtual Maybe<PointI> HotSpot() const noexcept
    {
      return null;
    }

  protected:
    ImageDecoder(AlphaOption alphaOption, GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
        : _premultiplyAlpha(alphaOption == AlphaOption::Premultiplied),
          _ignoreGammaAndColorProfile(gammaAndColorProfileOption == GammaAndColorProfileOption::Ignored)
    {
    }

  private:
    virtual void TryDecodeSize(bool allDataReceived) noexcept = 0;
  };
}
