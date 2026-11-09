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

#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoderFrame.hpp"

namespace krys::boo
{
  ScalableImageDecoderFrame::ScalableImageDecoderFrame() noexcept = default;

  ScalableImageDecoderFrame::~ScalableImageDecoderFrame() noexcept = default;

  ScalableImageDecoderFrame &
    ScalableImageDecoderFrame::operator=(const ScalableImageDecoderFrame &other) noexcept
  {
    if (this == &other)
    {
      return *this;
    }

    _decodingStatus = other._decodingStatus;
    if (other.BackingStore())
    {
      (void)Initialize(*other.BackingStore());
    }
    else
    {
      _backingStore = nullptr;
    }
    _disposalMethod = other._disposalMethod;
    _orientation = other._orientation;
    _duration = other._duration;
    _hasAlpha = other._hasAlpha;
    return *this;
  }

  void ScalableImageDecoderFrame::SetDecodingStatus(DecodingStatus decodingStatus) noexcept
  {
    krys_debug_assert(decodingStatus != DecodingStatus::Decoding);
    _decodingStatus = decodingStatus;
  }

  DecodingStatus ScalableImageDecoderFrame::GetDecodingStatus() const noexcept
  {
    krys_debug_assert(_decodingStatus != DecodingStatus::Decoding);
    return _decodingStatus;
  }

  void ScalableImageDecoderFrame::Clear() noexcept
  {
    *this = ScalableImageDecoderFrame();
  }

  bool ScalableImageDecoderFrame::Initialize(const ImageBackingStore &backingStore) noexcept
  {
    if (&backingStore == BackingStore())
    {
      return true;
    }

    _backingStore = ImageBackingStore::Create(backingStore);
    return _backingStore != nullptr;
  }

  bool ScalableImageDecoderFrame::Initialize(const IntSize &size, bool premultiplyAlpha) noexcept
  {
    if (size.IsEmpty())
    {
      return false;
    }

    _backingStore = ImageBackingStore::Create(size, premultiplyAlpha);
    return _backingStore != nullptr;
  }

  IntSize ScalableImageDecoderFrame::Size() const noexcept
  {
    if (HasBackingStore())
    {
      return BackingStore()->Size();
    }

    return {};
  }
}
