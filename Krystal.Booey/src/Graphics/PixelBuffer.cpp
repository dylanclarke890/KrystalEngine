#include "Krystal.Booey/Graphics/PixelBuffer.hpp"
#include "Krystal.Booey/DOM/Types/Uint8ClampedArray.hpp"
#include "Krystal.Core/Assert.hpp"
#include <cstring>
#include <limits>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD Maybe<uint32> MustFitInInt32(uint64 value) noexcept
    {
      if (value > static_cast<uint64>(std::numeric_limits<int32>::max()))
      {
        return {};
      }
      return static_cast<uint32>(value);
    }

    KRYS_NODISCARD uint64 ComputeRawPixelCount(const IntSize &size) noexcept
    {
      if (size.Width() < 0 || size.Height() < 0)
      {
        return std::numeric_limits<uint64>::max();
      }
      return static_cast<uint64>(size.Width()) * static_cast<uint64>(size.Height());
    }

    KRYS_NODISCARD uint64 ComputeRawPixelComponentCount(PixelFormat pixelFormat, const IntSize &size) noexcept
    {
      krys_debug_assert(PixelBuffer::SupportedPixelFormat(pixelFormat));
      const auto pixels = ComputeRawPixelCount(size);
      if (pixels > std::numeric_limits<uint64>::max() / PixelBuffer::ComponentsPerPixel(pixelFormat))
      {
        return std::numeric_limits<uint64>::max();
      }
      return pixels * PixelBuffer::ComponentsPerPixel(pixelFormat);
    }
  }

  bool PixelBuffer::SupportedPixelFormat(PixelFormat pixelFormat) noexcept
  {
    switch (pixelFormat)
    {
      case PixelFormat::RGBA8:
      case PixelFormat::BGRA8: return true;
      case PixelFormat::BGRX8: return false;
    }
    return false;
  }

  Maybe<uint32> PixelBuffer::ComputePixelCount(const IntSize &size) noexcept
  {
    return MustFitInInt32(ComputeRawPixelCount(size));
  }

  Maybe<uint32> PixelBuffer::ComputePixelComponentCount(PixelFormat pixelFormat, const IntSize &size) noexcept
  {
    return MustFitInInt32(ComputeRawPixelComponentCount(pixelFormat, size));
  }

  Maybe<uint32> PixelBuffer::ComputeBufferSize(PixelFormat pixelFormat, const IntSize &size) noexcept
  {
    const auto components = ComputeRawPixelComponentCount(pixelFormat, size);
    if (components > std::numeric_limits<uint64>::max() / BytesPerPixelComponent(pixelFormat))
    {
      return {};
    }
    return MustFitInInt32(components * BytesPerPixelComponent(pixelFormat));
  }

  PixelBuffer::PixelBuffer(const PixelBufferFormat &format, const IntSize &size, Span<uint8> bytes) noexcept
      : _format(format), _size(size), _bytes(bytes)
  {
    krys_assert(ComputeBufferSize(format.Format, size).value_or(0u) <= bytes.size());
  }

  PixelBuffer::~PixelBuffer() noexcept = default;

  bool PixelBuffer::SetRange(Span<const uint8> data, size_t byteOffset) noexcept
  {
    if (byteOffset > _bytes.size() || data.size() > _bytes.size() - byteOffset)
    {
      return false;
    }
    if (!data.empty())
    {
      std::memmove(_bytes.data() + byteOffset, data.data(), data.size());
    }
    return true;
  }

  bool PixelBuffer::ZeroRange(size_t byteOffset, size_t rangeByteLength) noexcept
  {
    if (byteOffset > _bytes.size() || rangeByteLength > _bytes.size() - byteOffset)
    {
      return false;
    }
    if (rangeByteLength)
    {
      std::memset(_bytes.data() + byteOffset, 0, rangeByteLength);
    }
    return true;
  }

  uint8 PixelBuffer::Item(size_t index) const noexcept
  {
    krys_assert(index < _bytes.size());
    return _bytes[index];
  }

  void PixelBuffer::Set(size_t index, double value) noexcept
  {
    krys_assert(index < _bytes.size());
    _bytes[index] = dom::Uint8ClampedArray::ToNativeFromDouble(value);
  }
}
