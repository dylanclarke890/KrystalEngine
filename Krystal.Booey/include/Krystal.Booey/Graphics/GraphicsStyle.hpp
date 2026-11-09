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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  // Legacy shadow blur radius is used for canvas.
  // It has different treatment of radii > 8px.
  enum class ShadowRadiusMode : bool
  {
    Default,
    Legacy
  };

  /// @brief A shadow cast by what's drawn: offset, blurred and coloured.
  struct GraphicsDropShadow
  {
    FloatSize Offset;
    float Radius;
    boo::Color Color;
    ShadowRadiusMode RadiusMode {ShadowRadiusMode::Default};
    float Opacity {1};

    KRYS_NODISCARD bool IsVisible() const noexcept
    {
      return Color.IsVisible();
    }
    KRYS_NODISCARD bool IsBlurred() const noexcept
    {
      return IsVisible() && Radius;
    }
    KRYS_NODISCARD bool HasOutsets() const noexcept
    {
      return IsBlurred() || (IsVisible() && !Offset.IsZero());
    }

    KRYS_NODISCARD friend bool operator==(const GraphicsDropShadow &a, const GraphicsDropShadow &b) noexcept
    {
      return a.Offset == b.Offset && a.Radius == b.Radius && a.Color == b.Color;
    }
  };

  /// @brief A Gaussian blur applied to what's drawn.
  struct GraphicsGaussianBlur
  {
    FloatSize Radius;

    KRYS_NODISCARD friend bool operator==(const GraphicsGaussianBlur &,
                                          const GraphicsGaussianBlur &) = default;
  };

  /// @brief A colour matrix applied to what's drawn.
  struct GraphicsColorMatrix
  {
    Array<float, 20> Values; // Really a ColourMatrix<5, 4>.

    KRYS_NODISCARD friend bool operator==(const GraphicsColorMatrix &, const GraphicsColorMatrix &) = default;
  };

  using GraphicsStyle = Variant<GraphicsDropShadow, GraphicsGaussianBlur, GraphicsColorMatrix>;

  TextStream &operator<<(TextStream &ts, const GraphicsDropShadow &dropShadow) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsGaussianBlur &gaussianBlur) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsColorMatrix &colorMatrix) noexcept;
  TextStream &operator<<(TextStream &ts, const GraphicsStyle &style) noexcept;
}
