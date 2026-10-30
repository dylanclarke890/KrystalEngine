#include "Krystal.Booey/Graphics/ByteArrayPixelBuffer.hpp"

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
