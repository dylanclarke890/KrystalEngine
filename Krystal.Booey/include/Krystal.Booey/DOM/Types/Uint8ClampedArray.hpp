#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <cmath>

namespace krys::boo::dom
{
  /// @brief Bytes whose writes are clamped to 0 to 255 and rounded to the nearest, ties to even.
  /// @note What holds an `ImageData`'s pixels, shared with the pixel buffer they were read into.
  /// @see https://tc39.es/ecma262/#sec-touint8clamp
  class Uint8ClampedArray final : public RefCounted<Uint8ClampedArray>
  {
    KRYS_NON_COPY_MOVABLE(Uint8ClampedArray);

  private:
    List<uint8> _bytes;

  public:
    /// @brief @p length bytes, all zero.
    KRYS_NODISCARD static Ref<Uint8ClampedArray> Create(size_t length) noexcept
    {
      return AdoptRef(*new Uint8ClampedArray(List<uint8>(length, uint8 {0})));
    }
    /// @brief A copy of @p bytes.
    KRYS_NODISCARD static Ref<Uint8ClampedArray> Create(Span<const uint8> bytes) noexcept
    {
      return AdoptRef(*new Uint8ClampedArray(List<uint8>(bytes.begin(), bytes.end())));
    }
    KRYS_NODISCARD static Ref<Uint8ClampedArray> Create(List<uint8> &&bytes) noexcept
    {
      return AdoptRef(*new Uint8ClampedArray(krys::move(bytes)));
    }

    KRYS_NODISCARD size_t Length() const noexcept
    {
      return _bytes.size();
    }
    KRYS_NODISCARD size_t ByteLength() const noexcept
    {
      return _bytes.size();
    }

    KRYS_NODISCARD Span<const uint8> GetSpan() const noexcept
    {
      return {_bytes.data(), _bytes.size()};
    }
    KRYS_NODISCARD Span<uint8> MutableSpan() noexcept
    {
      return {_bytes.data(), _bytes.size()};
    }

    KRYS_NODISCARD uint8 Item(size_t index) const noexcept
    {
      krys_assert(index < _bytes.size());
      return _bytes[index];
    }
    /// @brief Sets the byte at @p index to @p value, clamped and rounded.
    void Set(size_t index, double value) noexcept
    {
      krys_assert(index < _bytes.size());
      _bytes[index] = ToNativeFromDouble(value);
    }

    /// @brief @p value clamped to 0 to 255 and rounded to the nearest, ties to even; NaN is 0.
    KRYS_NODISCARD static uint8 ToNativeFromDouble(double value) noexcept
    {
      if (!(value > 0))
      {
        return 0;
      }
      if (value >= 255)
      {
        return 255;
      }
      const auto floor = std::floor(value);
      const auto fraction = value - floor;
      auto rounded = floor;
      if (fraction > 0.5 || (fraction == 0.5 && std::fmod(floor, 2.0) != 0))
      {
        rounded += 1;
      }
      return static_cast<uint8>(rounded);
    }

  private:
    explicit Uint8ClampedArray(List<uint8> &&bytes) noexcept : _bytes(krys::move(bytes))
    {
    }
  };
}
