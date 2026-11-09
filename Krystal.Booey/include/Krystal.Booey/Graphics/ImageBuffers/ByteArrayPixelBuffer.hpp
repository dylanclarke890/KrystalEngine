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

#include "Krystal.Booey/DOM/Types/Uint8ClampedArray.hpp"
#include "Krystal.Booey/Graphics/ImageBuffers/PixelBuffer.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  /// @brief A pixel buffer of 8 bits a channel, whose bytes are a clamped byte array it can share.
  class ByteArrayPixelBuffer final : public PixelBuffer
  {
  private:
    Ref<dom::Uint8ClampedArray> _data;

  public:
    KRYS_NODISCARD static Ref<ByteArrayPixelBuffer> Create(const PixelBufferFormat &, const IntSize &,
                                                           dom::Uint8ClampedArray &) noexcept;
    /// @brief A buffer holding a copy of @p data; empty if it's not as long as @p size's pixels.
    KRYS_NODISCARD static Maybe<Ref<ByteArrayPixelBuffer>> Create(const PixelBufferFormat &, const IntSize &,
                                                                  Span<const uint8> data) noexcept;
    /// @brief A buffer of @p size, all zero; null if it's too big.
    KRYS_NODISCARD static RefPtr<ByteArrayPixelBuffer> TryCreate(const PixelBufferFormat &,
                                                                 const IntSize &size) noexcept;
    /// @brief A buffer whose bytes are @p data; null if it's not as long as @p size's pixels.
    KRYS_NODISCARD static RefPtr<ByteArrayPixelBuffer>
      TryCreate(const PixelBufferFormat &, const IntSize &size, Ref<dom::Uint8ClampedArray> &&data) noexcept;

    KRYS_NODISCARD dom::Uint8ClampedArray &Data() const noexcept
    {
      return const_cast<dom::Uint8ClampedArray &>(*_data);
    }
    KRYS_NODISCARD Ref<dom::Uint8ClampedArray> ProtectedData() const noexcept
    {
      return _data;
    }
    /// @brief The bytes, shared: what's written to them after shows in the buffer too.
    KRYS_NODISCARD Ref<dom::Uint8ClampedArray> TakeData() noexcept
    {
      return _data;
    }

    KRYS_NODISCARD Type GetType() const noexcept override
    {
      return Type::ByteArray;
    }
    KRYS_NODISCARD RefPtr<PixelBuffer> CreateScratchPixelBuffer(const IntSize &) const noexcept override;

  private:
    ByteArrayPixelBuffer(const PixelBufferFormat &, const IntSize &, Ref<dom::Uint8ClampedArray> &&) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ByteArrayPixelBuffer)
  KRYS_NODISCARD static bool IsType(const krys::boo::PixelBuffer &pixelBuffer) noexcept
  {
    return pixelBuffer.GetType() == krys::boo::PixelBuffer::Type::ByteArray;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
