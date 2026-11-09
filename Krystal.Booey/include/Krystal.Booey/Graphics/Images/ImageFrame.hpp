/*
 * Copyright (C) 2016-2025 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  /// @brief A frame of a bitmap image: its metadata, and its native image once decoded.
  // FIXME: The HDR source, once there's HDR.
  class ImageFrame
  {
    friend class BitmapImageSource;
    friend class ImageDecoder;

  public:
    enum class Caching : uint8
    {
      Metadata,
      MetadataAndImage
    };

  private:
    struct Source
    {
      RefPtr<NativeImage> Image;
      DecodingOptions Options {DecodingMode::Auto};

      KRYS_NODISCARD bool HasNativeImage() const noexcept
      {
        return static_cast<bool>(Image);
      }

      KRYS_NODISCARD bool HasFullSizeNativeImage() const noexcept
      {
        return HasNativeImage() && Options.HasFullSize();
      }

      KRYS_NODISCARD bool
        HasDecodedNativeImageCompatibleWithOptions(const DecodingOptions &options) const noexcept
      {
        return HasNativeImage() && Options.IsCompatibleWith(options);
      }

      void Clear() noexcept
      {
        if (std::exchange(Image, nullptr))
        {
          Options = DecodingOptions();
        }
      }
    };

    DecodingStatus _decodingStatus {DecodingStatus::Invalid};
    IntSize _size;
    Maybe<IntSize> _densityCorrectedSize;
    SubsamplingLevel _subsamplingLevel {SubsamplingLevel::Default};
    ImageOrientation _orientation {ImageOrientation::Orientation::None};
    seconds _duration {};
    bool _hasAlpha {true};
    Source _source;

  public:
    ImageFrame() noexcept = default;
    explicit ImageFrame(Ref<NativeImage> &&nativeImage) noexcept;
    ImageFrame(const ImageFrame &other) noexcept = default;
    ImageFrame &operator=(const ImageFrame &other) noexcept = default;
    ~ImageFrame() noexcept;

    KRYS_NODISCARD static const ImageFrame &DefaultFrame() noexcept;

    /// @return The bytes freed.
    uint32 ClearImage() noexcept;
    uint32 Clear() noexcept;

    void SetDecodingStatus(DecodingStatus decodingStatus) noexcept;
    KRYS_NODISCARD DecodingStatus GetDecodingStatus() const noexcept;

    KRYS_NODISCARD bool IsInvalid() const noexcept
    {
      return _decodingStatus == DecodingStatus::Invalid;
    }
    KRYS_NODISCARD bool IsPartial() const noexcept
    {
      return _decodingStatus == DecodingStatus::Partial;
    }
    KRYS_NODISCARD bool IsComplete() const noexcept
    {
      return _decodingStatus == DecodingStatus::Complete;
    }

    void SetSize(const IntSize &size) noexcept
    {
      _size = size;
    }
    KRYS_NODISCARD IntSize Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD uint32 SizeInBytes() const noexcept
    {
      return static_cast<uint32>(Size().UnclampedArea()) * sizeof(uint32);
    }

    void SetDensityCorrectedSize(const IntSize &size) noexcept
    {
      _densityCorrectedSize = size;
    }
    KRYS_NODISCARD Maybe<IntSize> DensityCorrectedSize() const noexcept
    {
      return _densityCorrectedSize;
    }
    KRYS_NODISCARD SubsamplingLevel GetSubsamplingLevel() const noexcept
    {
      return _subsamplingLevel;
    }

    KRYS_NODISCARD RefPtr<NativeImage> GetNativeImage() const noexcept
    {
      return _source.Image;
    }
    KRYS_NODISCARD DecodingOptions GetDecodingOptions() const noexcept
    {
      return _source.Options;
    }

    void SetOrientation(ImageOrientation orientation) noexcept
    {
      _orientation = orientation;
    }
    KRYS_NODISCARD ImageOrientation Orientation() const noexcept
    {
      return _orientation;
    }

    void SetDuration(seconds duration) noexcept
    {
      _duration = duration;
    }
    KRYS_NODISCARD seconds Duration() const noexcept
    {
      return _duration;
    }

    void SetHasAlpha(bool hasAlpha) noexcept
    {
      _hasAlpha = hasAlpha;
    }
    KRYS_NODISCARD bool HasAlpha() const noexcept
    {
      return !HasMetadata() || _hasAlpha;
    }

    KRYS_NODISCARD bool HasNativeImage() const noexcept
    {
      return _source.HasNativeImage();
    }
    KRYS_NODISCARD bool HasNativeImage(SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool HasFullSizeNativeImage(SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool
      HasDecodedNativeImageCompatibleWithOptions(const DecodingOptions &options,
                                                 SubsamplingLevel subsamplingLevel) const noexcept;
    KRYS_NODISCARD bool HasMetadata() const noexcept
    {
      return !Size().IsEmpty();
    }
  };
}
