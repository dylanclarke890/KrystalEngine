/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class FloatSize;

  /// @brief A width and height in double precision.
  class DoubleSize
  {
  private:
    double _width {0};
    double _height {0};

  public:
    constexpr DoubleSize() noexcept = default;
    constexpr DoubleSize(double width, double height) noexcept : _width(width), _height(height)
    {
    }

    explicit DoubleSize(const FloatSize &size) noexcept;

    KRYS_NODISCARD constexpr double Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr double Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr DoubleSize ScaledBy(double scaleX, double scaleY) const noexcept
    {
      return DoubleSize(_width * scaleX, _height * scaleY);
    }

    KRYS_NODISCARD constexpr DoubleSize ScaledBy(double scale) const noexcept
    {
      return ScaledBy(scale, scale);
    }

    KRYS_NODISCARD friend constexpr bool operator==(const DoubleSize &,
                                                    const DoubleSize &) noexcept = default;
  };
}
