#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Booey/Graphics/PixelBufferFormat.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  /// @brief Type for holding pixel buffers data.
  /// @note For functions that source pixel buffers, see `PixelBufferSourceView`.
  class PixelBuffer : public RefCounted<PixelBuffer>
  {
    KRYS_NON_COPY_MOVABLE(PixelBuffer);

  public:
    enum class Type : uint8
    {
      ByteArray,
      Other
    };

  protected:
    PixelBufferFormat _format;
    IntSize _size;
    Span<uint8> _bytes;

  public:
    virtual ~PixelBuffer() noexcept;

    KRYS_NODISCARD static constexpr uint32 BytesPerPixelComponent(PixelFormat) noexcept
    {
      return 1;
    }
    KRYS_NODISCARD static constexpr uint32 ComponentsPerPixel(PixelFormat) noexcept
    {
      return 4;
    }
    KRYS_NODISCARD static constexpr uint32 BytesPerPixel(PixelFormat pixelFormat) noexcept
    {
      return BytesPerPixelComponent(pixelFormat) * ComponentsPerPixel(pixelFormat);
    }

    /// @brief The pixels in @p size; empty if they don't fit in an `int32`.
    KRYS_NODISCARD static Maybe<uint32> ComputePixelCount(const IntSize &size) noexcept;
    /// @brief The channels in @p size's pixels; empty if they don't fit in an `int32`.
    KRYS_NODISCARD static Maybe<uint32> ComputePixelComponentCount(PixelFormat, const IntSize &size) noexcept;
    /// @brief The bytes in @p size's pixels; empty if they don't fit in an `int32`.
    KRYS_NODISCARD static Maybe<uint32> ComputeBufferSize(PixelFormat, const IntSize &size) noexcept;
    KRYS_NODISCARD static bool SupportedPixelFormat(PixelFormat) noexcept;

    KRYS_NODISCARD const PixelBufferFormat &Format() const noexcept
    {
      return _format;
    }
    KRYS_NODISCARD const IntSize &Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD Span<uint8> Bytes() const noexcept
    {
      return _bytes;
    }

    KRYS_NODISCARD virtual Type GetType() const noexcept
    {
      return Type::Other;
    }
    KRYS_NODISCARD virtual RefPtr<PixelBuffer> CreateScratchPixelBuffer(const IntSize &) const noexcept = 0;

    bool SetRange(Span<const uint8> data, size_t byteOffset) noexcept;
    bool ZeroRange(size_t byteOffset, size_t rangeByteLength) noexcept;
    void ZeroFill() noexcept
    {
      ZeroRange(0, Bytes().size());
    }

    KRYS_NODISCARD uint8 Item(size_t index) const noexcept;
    void Set(size_t index, double value) noexcept;

  protected:
    PixelBuffer(const PixelBufferFormat &, const IntSize &, Span<uint8> bytes) noexcept;
  };

  /// @brief Type to use for functions that use the PixelBuffer data as source during the call, but do not
  /// store a reference to the object or modify the data.
  class PixelBufferSourceView
  {
  private:
    PixelBufferFormat _format;
    IntSize _size;
    Span<const uint8> _bytes;

  public:
    PixelBufferSourceView() = delete;
    PixelBufferSourceView(const PixelBuffer &pixelBuffer) noexcept
        : PixelBufferSourceView(pixelBuffer.Format(), pixelBuffer.Size(), pixelBuffer.Bytes())
    {
    }

    /// @brief A view of @p bytes as @p size pixels of @p format; empty if they're the wrong length or the
    /// format isn't supported.
    KRYS_NODISCARD static Maybe<PixelBufferSourceView>
      Create(const PixelBufferFormat &format, const IntSize &size, Span<const uint8> bytes) noexcept
    {
      if (!PixelBuffer::SupportedPixelFormat(format.Format))
      {
        return {};
      }
      auto bufferSize = PixelBuffer::ComputeBufferSize(format.Format, size);
      if (!bufferSize || bytes.size() != *bufferSize)
      {
        return {};
      }
      return PixelBufferSourceView(format, size, bytes);
    }

    KRYS_NODISCARD const PixelBufferFormat &Format() const noexcept
    {
      return _format;
    }
    KRYS_NODISCARD IntSize Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD Span<const uint8> Bytes() const noexcept
    {
      return _bytes;
    }

  private:
    PixelBufferSourceView(const PixelBufferFormat &format, const IntSize &size,
                          Span<const uint8> bytes) noexcept
        : _format(format), _size(size), _bytes(bytes)
    {
    }
  };
}
