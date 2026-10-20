#pragma once

#include "Krystal.Booey/Graphics/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  enum class DecodingMode : uint8
  {
    Auto,
    Synchronous,
    Asynchronous
  };

  /// @brief How an image frame is to be decoded: when, and at what size.
  // FIXME: Decoding to HDR, once there's an HDR colour space.
  class DecodingOptions
  {
  private:
    DecodingMode _decodingMode;
    Maybe<IntSize> _sizeForDrawing;

  public:
    constexpr DecodingOptions(DecodingMode decodingMode = DecodingMode::Synchronous,
                              const Maybe<IntSize> &sizeForDrawing = null) noexcept
        : _decodingMode(decodingMode), _sizeForDrawing(sizeForDrawing)
    {
    }

    KRYS_NODISCARD friend constexpr bool operator==(const DecodingOptions &,
                                                    const DecodingOptions &) noexcept = default;

    KRYS_NODISCARD constexpr DecodingMode GetDecodingMode() const noexcept
    {
      return _decodingMode;
    }
    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _decodingMode == DecodingMode::Auto;
    }
    KRYS_NODISCARD constexpr bool IsSynchronous() const noexcept
    {
      return _decodingMode == DecodingMode::Synchronous;
    }
    KRYS_NODISCARD constexpr bool IsAsynchronous() const noexcept
    {
      return _decodingMode == DecodingMode::Asynchronous;
    }

    KRYS_NODISCARD constexpr Maybe<IntSize> SizeForDrawing() const noexcept
    {
      return _sizeForDrawing;
    }
    KRYS_NODISCARD constexpr bool HasFullSize() const noexcept
    {
      return !_sizeForDrawing;
    }
    KRYS_NODISCARD constexpr bool HasSizeForDrawing() const noexcept
    {
      return _sizeForDrawing.has_value();
    }

    KRYS_NODISCARD constexpr bool IsCompatibleWith(const DecodingOptions &other) const noexcept
    {
      if (IsAuto() || other.IsAuto())
      {
        return false;
      }

      if (HasFullSize())
      {
        return true;
      }

      if (other.HasFullSize())
      {
        return false;
      }

      return SizeForDrawing()->MaxDimension() >= other.SizeForDrawing()->MaxDimension();
    }
  };
}
