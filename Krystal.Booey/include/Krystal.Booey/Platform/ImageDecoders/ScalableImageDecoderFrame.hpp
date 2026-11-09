/*
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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
#include "Krystal.Booey/Graphics/Images/ImageBackingStore.hpp"
#include "Krystal.Booey/Graphics/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  /// @brief A frame a scalable image decoder decodes into: its backing store and its metadata.
  class ScalableImageDecoderFrame
  {
  public:
    enum class DisposalMethod : uint8
    {
      Unspecified,
      DoNotDispose,
      RestoreToBackground,
      RestoreToPrevious
    };

  private:
    DecodingStatus _decodingStatus {DecodingStatus::Invalid};

    UniquePtr<ImageBackingStore> _backingStore;
    DisposalMethod _disposalMethod {DisposalMethod::Unspecified};

    ImageOrientation _orientation {ImageOrientation::Orientation::None};
    seconds _duration {};
    bool _hasAlpha {true};

  public:
    ScalableImageDecoderFrame() noexcept;
    ScalableImageDecoderFrame(const ScalableImageDecoderFrame &other) noexcept
    {
      operator=(other);
    }

    ~ScalableImageDecoderFrame() noexcept;

    ScalableImageDecoderFrame &operator=(const ScalableImageDecoderFrame &other) noexcept;

    void Clear() noexcept;

    bool Initialize(const ImageBackingStore &backingStore) noexcept;
    bool Initialize(const IntSize &size, bool premultiplyAlpha) noexcept;

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

    KRYS_NODISCARD IntSize Size() const noexcept;

    void SetDisposalMethod(DisposalMethod method) noexcept
    {
      _disposalMethod = method;
    }
    KRYS_NODISCARD DisposalMethod GetDisposalMethod() const noexcept
    {
      return _disposalMethod;
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
    KRYS_NODISCARD bool HasMetadata() const noexcept
    {
      return !Size().IsEmpty();
    }

    KRYS_NODISCARD ImageBackingStore *BackingStore() const noexcept
    {
      return _backingStore.get();
    }
    KRYS_NODISCARD bool HasBackingStore() const noexcept
    {
      return BackingStore();
    }
  };
}
