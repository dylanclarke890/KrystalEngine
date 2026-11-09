/*
 * Copyright (C) 2022-2024 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/ImageBuffers/ByteArrayPixelBuffer.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsByteFormat(PixelFormat format) noexcept
    {
      return format == PixelFormat::RGBA8 || format == PixelFormat::BGRA8;
    }
  }

  Ref<ByteArrayPixelBuffer> ByteArrayPixelBuffer::Create(const PixelBufferFormat &format, const IntSize &size,
                                                         dom::Uint8ClampedArray &data) noexcept
  {
    krys_assert(IsByteFormat(format.Format));
    return AdoptRef(*new ByteArrayPixelBuffer(format, size, ShareRef(data)));
  }

  Maybe<Ref<ByteArrayPixelBuffer>> ByteArrayPixelBuffer::Create(const PixelBufferFormat &format,
                                                                const IntSize &size,
                                                                Span<const uint8> data) noexcept
  {
    if (!IsByteFormat(format.Format))
    {
      krys_debug_assert(false);
      return {};
    }
    auto computedBufferSize = PixelBuffer::ComputeBufferSize(format.Format, size);
    if (!computedBufferSize)
    {
      krys_debug_assert(false);
      return {};
    }
    if (data.size_bytes() != *computedBufferSize)
    {
      krys_debug_assert(false);
      return {};
    }
    auto buffer = dom::Uint8ClampedArray::Create(data);
    return ByteArrayPixelBuffer::Create(format, size, *buffer);
  }

  RefPtr<ByteArrayPixelBuffer> ByteArrayPixelBuffer::TryCreate(const PixelBufferFormat &format,
                                                               const IntSize &size) noexcept
  {
    krys_debug_assert(SupportedPixelFormat(format.Format));
    if (!IsByteFormat(format.Format))
    {
      krys_debug_assert(false);
      return nullptr;
    }
    auto bufferSize = ComputeBufferSize(format.Format, size);
    if (!bufferSize)
    {
      return nullptr;
    }
    return AdoptRef(*new ByteArrayPixelBuffer(format, size, dom::Uint8ClampedArray::Create(*bufferSize)));
  }

  RefPtr<ByteArrayPixelBuffer> ByteArrayPixelBuffer::TryCreate(const PixelBufferFormat &format,
                                                               const IntSize &size,
                                                               Ref<dom::Uint8ClampedArray> &&data) noexcept
  {
    krys_debug_assert(SupportedPixelFormat(format.Format));
    if (!IsByteFormat(format.Format))
    {
      krys_debug_assert(false);
      return nullptr;
    }
    auto bufferSize = ComputeBufferSize(format.Format, size);
    if (!bufferSize || *bufferSize != data->ByteLength())
    {
      return nullptr;
    }
    return AdoptRef(*new ByteArrayPixelBuffer(format, size, krys::move(data)));
  }

  ByteArrayPixelBuffer::ByteArrayPixelBuffer(const PixelBufferFormat &format, const IntSize &size,
                                             Ref<dom::Uint8ClampedArray> &&data) noexcept
      : PixelBuffer(format, size, data->MutableSpan()), _data(krys::move(data))
  {
  }

  RefPtr<PixelBuffer> ByteArrayPixelBuffer::CreateScratchPixelBuffer(const IntSize &size) const noexcept
  {
    return ByteArrayPixelBuffer::TryCreate(_format, size);
  }
}
