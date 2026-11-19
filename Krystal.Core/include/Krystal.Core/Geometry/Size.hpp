/*
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
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
/*
 * Copyright (C) 2003-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2014 Google Inc. All rights reserved.
 * Copyright (C) 2005 Nokia. All rights reserved.
 *               2008 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/TypeTraits.hpp"
#include <algorithm>
#include <cmath>

namespace krys
{
  class TextStream;

  namespace detail
  {
    /// @brief Whether converting a `TFrom` to a `TTo` keeps every value: an integer to a floating point type,
    /// or a floating point type to one at least as wide. Geometry types convert implicitly only then.
    template <typename TFrom, typename TTo>
    constexpr bool IsWideningGeometryConversion =
      (Integral<TFrom> && FloatingPoint<TTo>)
      || (FloatingPoint<TFrom> && FloatingPoint<TTo> && sizeof(TTo) >= sizeof(TFrom));

    /// @brief @p value as a `TTo`: clamped to the integer range when narrowing a floating point value to an
    /// integer.
    template <Number TTo, Number TFrom>
    KRYS_NODISCARD constexpr TTo ConvertGeometryValue(TFrom value) noexcept
    {
      if constexpr (Integral<TTo> && FloatingPoint<TFrom>)
      {
        return static_cast<TTo>(ClampToInteger(value));
      }
      else
      {
        return static_cast<TTo>(value);
      }
    }

    /// @brief The type geometry types scale by: `double` for `double` coordinates, `float` otherwise.
    template <Number T>
    using GeometryScale = conditional_t<SameType<T, double>, double, float>;
  }

  /// @brief A width and height.
  template <Number T>
  class Size
  {
  private:
    using ScaleType = detail::GeometryScale<T>;

    T _width {0};
    T _height {0};

  public:
    constexpr Size() noexcept = default;

    constexpr Size(T width, T height) noexcept : _width(width), _height(height)
    {
    }

    /// @brief Converts from another coordinate type: implicitly when no value is lost, explicitly (clamping
    /// to the integer range) otherwise.
    template <Number U>
    requires(!SameType<T, U>)
    constexpr explicit(!detail::IsWideningGeometryConversion<U, T>) Size(const Size<U> &size) noexcept
        : _width(detail::ConvertGeometryValue<T>(size.Width())),
          _height(detail::ConvertGeometryValue<T>(size.Height()))
    {
    }

    KRYS_NODISCARD constexpr static Size NarrowPrecision(double width, double height) noexcept
    requires SameType<T, float>
    {
      return Size(static_cast<float>(width), static_cast<float>(height));
    }

    KRYS_NODISCARD constexpr static Size NanSize() noexcept
    requires FloatingPoint<T>
    {
      return {Limits<T>::quiet_NaN(), Limits<T>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr T Width() const noexcept
    {
      return _width;
    }

    KRYS_NODISCARD constexpr T Height() const noexcept
    {
      return _height;
    }

    KRYS_NODISCARD constexpr T MinDimension() const noexcept
    {
      return std::min(_width, _height);
    }

    KRYS_NODISCARD constexpr T MaxDimension() const noexcept
    {
      return std::max(_width, _height);
    }

    constexpr void SetWidth(T width) noexcept
    {
      _width = width;
    }

    constexpr void SetHeight(T height) noexcept
    {
      _height = height;
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _width <= 0 || _height <= 0;
    }

    /// @brief Whether both sides are zero: exactly for integers, within epsilon for floating point.
    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      if constexpr (FloatingPoint<T>)
      {
        return (_width < 0 ? -_width : _width) < Limits<T>::epsilon()
               && (_height < 0 ? -_height : _height) < Limits<T>::epsilon();
      }
      else
      {
        return !_width && !_height;
      }
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    requires FloatingPoint<T>
    {
      // NaN is the only value that isn't equal to itself.
      return _width != _width;
    }

    KRYS_NODISCARD constexpr bool IsExpressibleAsSizeI() const noexcept
    requires SameType<T, float>
    {
      return IsWithinIntRange(_width) && IsWithinIntRange(_height);
    }

    KRYS_NODISCARD constexpr ScaleType AspectRatio() const noexcept
    {
      return static_cast<ScaleType>(_width) / static_cast<ScaleType>(_height);
    }

    KRYS_NODISCARD constexpr double AspectRatioDouble() const noexcept
    {
      return static_cast<double>(_width) / static_cast<double>(_height);
    }

    constexpr void Expand(T width, T height) noexcept
    {
      _width += width;
      _height += height;
    }

    constexpr void Contract(T width, T height) noexcept
    {
      _width -= width;
      _height -= height;
    }

    /// @brief Scales each side; integer sides are truncated.
    constexpr void Scale(ScaleType widthScale, ScaleType heightScale) noexcept
    {
      *this = Scaled(widthScale, heightScale);
    }

    constexpr void Scale(ScaleType scale) noexcept
    {
      Scale(scale, scale);
    }

    /// @brief This scaled on each side; integer sides are truncated.
    KRYS_NODISCARD constexpr Size Scaled(ScaleType widthScale, ScaleType heightScale) const noexcept
    {
      if constexpr (Integral<T>)
      {
        return {static_cast<T>(static_cast<ScaleType>(_width) * widthScale),
                static_cast<T>(static_cast<ScaleType>(_height) * heightScale)};
      }
      else
      {
        return {_width * widthScale, _height * heightScale};
      }
    }

    KRYS_NODISCARD constexpr Size Scaled(ScaleType scale) const noexcept
    {
      return Scaled(scale, scale);
    }

    KRYS_NODISCARD constexpr Size ScaledBy(ScaleType scaleX, ScaleType scaleY) const noexcept
    {
      return Scaled(scaleX, scaleY);
    }

    KRYS_NODISCARD constexpr Size ScaledBy(ScaleType scale) const noexcept
    {
      return Scaled(scale, scale);
    }

    KRYS_NODISCARD constexpr Size ExpandedTo(const Size &other) const noexcept
    {
      return {_width > other._width ? _width : other._width,
              _height > other._height ? _height : other._height};
    }

    KRYS_NODISCARD constexpr Size ShrunkTo(const Size &other) const noexcept
    {
      return {_width < other._width ? _width : other._width,
              _height < other._height ? _height : other._height};
    }

    constexpr void ClampNegativeToZero() noexcept
    {
      *this = ExpandedTo(Size());
    }

    constexpr void ClampToMinimumSize(const Size &minimumSize) noexcept
    {
      _width = std::max(_width, minimumSize._width);
      _height = std::max(_height, minimumSize._height);
    }

    constexpr void ClampToMaximumSize(const Size &maximumSize) noexcept
    {
      _width = std::min(_width, maximumSize._width);
      _height = std::min(_height, maximumSize._height);
    }

    KRYS_NODISCARD constexpr Size ConstrainedBetween(const Size &min, const Size &max) const noexcept
    {
      return {std::max(min._width, std::min(max._width, _width)),
              std::max(min._height, std::min(max._height, _height))};
    }

    KRYS_NODISCARD constexpr T Area() const noexcept
    requires FloatingPoint<T>
    {
      return _width * _height;
    }

    /// @brief The area, from the sides' magnitudes, without overflowing.
    KRYS_NODISCARD constexpr uint64 UnclampedArea() const noexcept
    requires Integral<T>
    {
      return static_cast<uint64>(_width < 0 ? -static_cast<int64>(_width) : _width)
             * static_cast<uint64>(_height < 0 ? -static_cast<int64>(_height) : _height);
    }

    KRYS_NODISCARD T DiagonalLength() const noexcept
    requires FloatingPoint<T>
    {
      return std::hypot(_width, _height);
    }

    KRYS_NODISCARD constexpr T DiagonalLengthSquared() const noexcept
    {
      return _width * _width + _height * _height;
    }

    KRYS_NODISCARD constexpr Size TransposedSize() const noexcept
    {
      return {_height, _width};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const Size &, const Size &) noexcept = default;

    friend constexpr Size &operator+=(Size &a, const Size &b) noexcept
    {
      a.Expand(b._width, b._height);
      return a;
    }

    friend constexpr Size &operator-=(Size &a, const Size &b) noexcept
    {
      a.Contract(b._width, b._height);
      return a;
    }

    KRYS_NODISCARD friend constexpr Size operator+(const Size &a, const Size &b) noexcept
    {
      return {a._width + b._width, a._height + b._height};
    }

    KRYS_NODISCARD friend constexpr Size operator-(const Size &a, const Size &b) noexcept
    {
      return {a._width - b._width, a._height - b._height};
    }

    KRYS_NODISCARD friend constexpr Size operator-(const Size &size) noexcept
    {
      return {-size._width, -size._height};
    }

    // Moving a point by a size is defined by `Point`; declared here too so that a point converts to match a
    // size's coordinate type.
    friend constexpr Point<T> &operator+=(Point<T> &a, const Size &b) noexcept;
    friend constexpr Point<T> &operator-=(Point<T> &a, const Size &b) noexcept;
    friend constexpr Point<T> operator+(const Point<T> &a, const Size &b) noexcept;
    friend constexpr Point<T> operator-(const Point<T> &a, const Size &b) noexcept;
  };

#pragma region Floating point sizes

  KRYS_NODISCARD constexpr SizeF operator*(const SizeF &a, float b) noexcept
  {
    return {a.Width() * b, a.Height() * b};
  }

  KRYS_NODISCARD constexpr SizeF operator*(float a, const SizeF &b) noexcept
  {
    return {a * b.Width(), a * b.Height()};
  }

  KRYS_NODISCARD constexpr SizeF operator*(const SizeF &a, const SizeF &b) noexcept
  {
    return {a.Width() * b.Width(), a.Height() * b.Height()};
  }

  KRYS_NODISCARD constexpr SizeF operator/(const SizeF &a, const SizeF &b) noexcept
  {
    return {a.Width() / b.Width(), a.Height() / b.Height()};
  }

  KRYS_NODISCARD constexpr SizeF operator/(const SizeF &a, float b) noexcept
  {
    return {a.Width() / b, a.Height() / b};
  }

  KRYS_NODISCARD constexpr SizeF operator/(float a, const SizeF &b) noexcept
  {
    return {a / b.Width(), a / b.Height()};
  }

  /// @brief The determinant of the matrix whose columns are @p a and @p b (the z of their cross product).
  KRYS_NODISCARD constexpr float Determinant(const SizeF &a, const SizeF &b) noexcept
  {
    return a.Width() * b.Height() - a.Height() * b.Width();
  }

  KRYS_NODISCARD constexpr bool AreEssentiallyEqual(const SizeF &a, const SizeF &b) noexcept
  {
    return AreEssentiallyEqual(a.Width(), b.Width()) && AreEssentiallyEqual(a.Height(), b.Height());
  }

  KRYS_NODISCARD inline SizeI RoundedSizeI(const SizeF &size) noexcept
  {
    return {ClampToInteger(std::round(size.Width())), ClampToInteger(std::round(size.Height()))};
  }

  KRYS_NODISCARD inline SizeI FlooredSizeI(const SizeF &size) noexcept
  {
    return {ClampToInteger(std::floor(size.Width())), ClampToInteger(std::floor(size.Height()))};
  }

  KRYS_NODISCARD inline SizeI ExpandedSizeI(const SizeF &size) noexcept
  {
    return {ClampToInteger(std::ceil(size.Width())), ClampToInteger(std::ceil(size.Height()))};
  }

#pragma endregion

  TextStream &operator<<(TextStream &ts, const SizeI &value) noexcept;

  TextStream &operator<<(TextStream &ts, const SizeF &value) noexcept;
}
