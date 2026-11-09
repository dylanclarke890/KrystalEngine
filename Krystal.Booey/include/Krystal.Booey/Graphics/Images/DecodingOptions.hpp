/*
 * Copyright (C) 2017-2023 Apple Inc. All rights reserved.
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
