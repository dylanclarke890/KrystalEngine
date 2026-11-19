/*
 * Copyright (C) 2003, 2006, 2009 Apple Inc. All rights reserved.
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
/*
 * Copyright (C) 2003-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2005 Nokia. All rights reserved.
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

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Geometry/GeometryForward.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <algorithm>

namespace krys
{
  class TextStream;

  /// @brief An axis-aligned rectangle: a location (its minimum corner) and a size.
  template <Number T>
  class Rect
  {
  private:
    using PointType = krys::Point<T>;
    using ScaleType = detail::GeometryScale<T>;
    using SizeType = krys::Size<T>;

    PointType _location;
    SizeType _size;

  public:
    enum class ContainsMode : uint8
    {
      InsideOrOnStroke,
      InsideButNotOnStroke,
    };

    constexpr Rect() noexcept = default;

    constexpr Rect(const PointType &location, const SizeType &size) noexcept
        : _location(location), _size(size)
    {
    }

    constexpr Rect(T x, T y, T width, T height) noexcept : _location(x, y), _size(width, height)
    {
    }

    // Floating point only: an integer rect is often made from a location and a braced size, which this would
    // make ambiguous.
    constexpr Rect(const PointType &topLeft, const PointType &bottomRight) noexcept
    requires FloatingPoint<T>
        : _location(topLeft), _size(bottomRight.X() - topLeft.X(), bottomRight.Y() - topLeft.Y())
    {
    }

    /// @brief Converts from another coordinate type: implicitly when no value is lost, explicitly (clamping
    /// to the integer range) otherwise.
    template <Number U>
    requires(!SameType<T, U>)
    constexpr explicit(!detail::IsWideningGeometryConversion<U, T>) Rect(const Rect<U> &rect) noexcept
        : _location(rect.Location()), _size(rect.Size())
    {
    }

    KRYS_NODISCARD constexpr static Rect NarrowPrecision(double x, double y, double width,
                                                         double height) noexcept
    requires SameType<T, float>
    {
      return Rect(NarrowPrecisionToFloat(x), NarrowPrecisionToFloat(y), NarrowPrecisionToFloat(width),
                  NarrowPrecisionToFloat(height));
    }

    KRYS_NODISCARD constexpr static Rect InfiniteRect() noexcept
    requires FloatingPoint<T>
    {
      return {-Limits<T>::max() / 2, -Limits<T>::max() / 2, Limits<T>::max(), Limits<T>::max()};
    }

    KRYS_NODISCARD constexpr static Rect SmallestRect() noexcept
    requires FloatingPoint<T>
    {
      return {Limits<T>::max() / 2, Limits<T>::max() / 2, -Limits<T>::max(), -Limits<T>::max()};
    }

    KRYS_NODISCARD constexpr static Rect NanRect() noexcept
    requires FloatingPoint<T>
    {
      return {Limits<T>::quiet_NaN(), Limits<T>::quiet_NaN(), Limits<T>::quiet_NaN(), Limits<T>::quiet_NaN()};
    }

    KRYS_NODISCARD constexpr PointType Location() const noexcept
    {
      return _location;
    }

    KRYS_NODISCARD constexpr SizeType Size() const noexcept
    {
      return _size;
    }

    constexpr void SetLocation(const PointType &location) noexcept
    {
      _location = location;
    }

    constexpr void SetSize(const SizeType &size) noexcept
    {
      _size = size;
    }

    KRYS_NODISCARD constexpr T X() const noexcept
    {
      return _location.X();
    }

    KRYS_NODISCARD constexpr T Y() const noexcept
    {
      return _location.Y();
    }

    KRYS_NODISCARD constexpr T MaxX() const noexcept
    {
      return X() + Width();
    }

    KRYS_NODISCARD constexpr T MaxY() const noexcept
    {
      return Y() + Height();
    }

    KRYS_NODISCARD constexpr T Width() const noexcept
    {
      return _size.Width();
    }

    KRYS_NODISCARD constexpr T Height() const noexcept
    {
      return _size.Height();
    }

    KRYS_NODISCARD constexpr T Area() const noexcept
    requires FloatingPoint<T>
    {
      return _size.Area();
    }

    constexpr void SetX(T x) noexcept
    {
      _location.SetX(x);
    }

    constexpr void SetY(T y) noexcept
    {
      _location.SetY(y);
    }

    constexpr void SetWidth(T width) noexcept
    {
      _size.SetWidth(width);
    }

    constexpr void SetHeight(T height) noexcept
    {
      _size.SetHeight(height);
    }

    KRYS_NODISCARD constexpr bool IsEmpty() const noexcept
    {
      return _size.IsEmpty();
    }

    KRYS_NODISCARD constexpr bool IsZero() const noexcept
    {
      return _size.IsZero();
    }

    KRYS_NODISCARD constexpr bool IsInfinite() const noexcept
    requires FloatingPoint<T>
    {
      return *this == InfiniteRect();
    }

    KRYS_NODISCARD constexpr bool IsSmallest() const noexcept
    requires FloatingPoint<T>
    {
      return *this == SmallestRect();
    }

    KRYS_NODISCARD constexpr bool IsNaN() const noexcept
    requires FloatingPoint<T>
    {
      // NaN is the only value that isn't equal to itself.
      return X() != X() || Y() != Y();
    }

    KRYS_NODISCARD constexpr bool IsExpressibleAsRectI() const noexcept
    requires SameType<T, float>
    {
      return IsWithinIntRange(X()) && IsWithinIntRange(Y()) && IsWithinIntRange(Width())
             && IsWithinIntRange(Height()) && IsWithinIntRange(MaxX()) && IsWithinIntRange(MaxY());
    }

    /// @brief Whether `X() + Width()` and `Y() + Height()` fit in a `T`.
    KRYS_NODISCARD constexpr bool IsValid() const noexcept
    requires Integral<T>
    {
      return !SumOverflows(X(), Width()) && !SumOverflows(Y(), Height());
    }

    /// @brief This with its width and height reduced so that its maximum edges fit in a `T`.
    KRYS_NODISCARD constexpr Rect ToRectWithExtentsClippedToNumericLimits() const noexcept
    requires Integral<T>
    {
      Rect clippedRect {*this};
      if (SumOverflows(X(), Width()))
      {
        clippedRect.SetWidth(Limits<T>::max() - X());
      }
      if (SumOverflows(Y(), Height()))
      {
        clippedRect.SetHeight(Limits<T>::max() - Y());
      }
      return clippedRect;
    }

    /// @brief The centre; integer coordinates round towards zero, so it may not be exact.
    KRYS_NODISCARD constexpr PointType Center() const noexcept
    {
      return {X() + Width() / 2, Y() + Height() / 2};
    }

    constexpr void Move(const SizeType &delta) noexcept
    {
      _location += delta;
    }

    constexpr void MoveBy(const PointType &delta) noexcept
    {
      _location.Move(delta.X(), delta.Y());
    }

    constexpr void Move(T dx, T dy) noexcept
    {
      _location.Move(dx, dy);
    }

    constexpr void Expand(const SizeType &size) noexcept
    {
      _size += size;
    }

    constexpr void Expand(T dw, T dh) noexcept
    {
      _size.Expand(dw, dh);
    }

    constexpr void Contract(const SizeType &size) noexcept
    {
      _size -= size;
    }

    constexpr void Contract(T dw, T dh) noexcept
    {
      _size.Expand(-dw, -dh);
    }

    constexpr void ShiftEdgesTo(T left, T top, T right, T bottom) noexcept
    {
      _location.Set(left, top);
      _size.SetWidth(right - left);
      _size.SetHeight(bottom - top);
    }

    constexpr void ShiftXEdgeTo(T edge) noexcept
    {
      T delta = edge - X();
      SetX(edge);
      SetWidth(std::max(T {0}, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeTo(T edge) noexcept
    {
      T delta = edge - MaxX();
      SetWidth(std::max(T {0}, Width() + delta));
    }

    constexpr void ShiftYEdgeTo(T edge) noexcept
    {
      T delta = edge - Y();
      SetY(edge);
      SetHeight(std::max(T {0}, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeTo(T edge) noexcept
    {
      T delta = edge - MaxY();
      SetHeight(std::max(T {0}, Height() + delta));
    }

    constexpr void ShiftXEdgeBy(T delta) noexcept
    {
      Move(delta, 0);
      SetWidth(std::max(T {0}, Width() - delta));
    }

    constexpr void ShiftMaxXEdgeBy(T delta) noexcept
    {
      if constexpr (Integral<T>)
      {
        SetWidth(std::max(T {0}, Width() + delta));
      }
      else
      {
        ShiftMaxXEdgeTo(MaxX() + delta);
      }
    }

    constexpr void ShiftYEdgeBy(T delta) noexcept
    {
      Move(0, delta);
      SetHeight(std::max(T {0}, Height() - delta));
    }

    constexpr void ShiftMaxYEdgeBy(T delta) noexcept
    {
      if constexpr (Integral<T>)
      {
        SetHeight(std::max(T {0}, Height() + delta));
      }
      else
      {
        ShiftMaxYEdgeTo(MaxY() + delta);
      }
    }

    // Typically top left.
    KRYS_NODISCARD constexpr PointType MinXMinYCorner() const noexcept
    {
      return _location;
    }

    // Typically top right.
    KRYS_NODISCARD constexpr PointType MaxXMinYCorner() const noexcept
    {
      return {MaxX(), Y()};
    }

    // Typically bottom left.
    KRYS_NODISCARD constexpr PointType MinXMaxYCorner() const noexcept
    {
      return {X(), MaxY()};
    }

    // Typically bottom right.
    KRYS_NODISCARD constexpr PointType MaxXMaxYCorner() const noexcept
    {
      return {MaxX(), MaxY()};
    }

    KRYS_NODISCARD constexpr bool Intersects(const Rect &other) const noexcept
    {
      // Checking emptiness handles negative widths and heights as well as zero.
      return !IsEmpty() && !other.IsEmpty() && X() < other.MaxX() && other.X() < MaxX() && Y() < other.MaxY()
             && other.Y() < MaxY();
    }

    KRYS_NODISCARD constexpr bool InclusivelyIntersects(const Rect &other) const noexcept
    {
      return Width() >= 0 && Height() >= 0 && other.Width() >= 0 && other.Height() >= 0 && X() <= other.MaxX()
             && other.X() <= MaxX() && Y() <= other.MaxY() && other.Y() <= MaxY();
    }

    KRYS_NODISCARD constexpr bool Contains(const Rect &other) const noexcept
    {
      return X() <= other.X() && MaxX() >= other.MaxX() && Y() <= other.Y() && MaxY() >= other.MaxY();
    }

    /// @brief Whether (@p px, @p py) is in this: for integers, whether the 1x1 rect below and to the right of
    /// it is (so the maximum edges are excluded); for floating point, including the edges.
    KRYS_NODISCARD constexpr bool Contains(T px, T py) const noexcept
    {
      if constexpr (Integral<T>)
      {
        return px >= X() && px < MaxX() && py >= Y() && py < MaxY();
      }
      else
      {
        return px >= X() && px <= MaxX() && py >= Y() && py <= MaxY();
      }
    }

    KRYS_NODISCARD constexpr bool Contains(const PointType &point) const noexcept
    requires Integral<T>
    {
      return Contains(point.X(), point.Y());
    }

    KRYS_NODISCARD constexpr bool
      Contains(const PointType &point,
               ContainsMode containsMode = ContainsMode::InsideOrOnStroke) const noexcept
    requires FloatingPoint<T>
    {
      if (containsMode == ContainsMode::InsideOrOnStroke)
      {
        return Contains(point.X(), point.Y());
      }
      return X() < point.X() && MaxX() > point.X() && Y() < point.Y() && MaxY() > point.Y();
    }

    KRYS_NODISCARD constexpr bool OverlapsYRange(T y1, T y2) const noexcept
    {
      return !IsEmpty() && y2 >= y1 && y2 >= Y() && y1 <= MaxY();
    }

    KRYS_NODISCARD constexpr bool OverlapsXRange(T x1, T x2) const noexcept
    {
      return !IsEmpty() && x2 >= x1 && x2 >= X() && x1 <= MaxX();
    }

    /// @brief Sets this to its intersection with @p other, or to an empty rect at the origin if they don't
    /// intersect.
    constexpr void Intersect(const Rect &other) noexcept
    {
      T left = std::max(X(), other.X());
      T top = std::max(Y(), other.Y());
      T right = std::min(MaxX(), other.MaxX());
      T bottom = std::min(MaxY(), other.MaxY());

      // Return a clean empty rectangle for non-intersecting cases.
      if (left >= right || top >= bottom)
      {
        left = 0;
        top = 0;
        right = 0;
        bottom = 0;
      }

      ShiftEdgesTo(left, top, right, bottom);
    }

    /// @brief Sets this to its intersection with @p other, where touching edges intersect, or to an empty
    /// rect at the origin if they don't.
    /// @return Whether they intersect.
    constexpr bool EdgeInclusiveIntersect(const Rect &other) noexcept
    {
      PointType newLocation(std::max(X(), other.X()), std::max(Y(), other.Y()));
      PointType newMaxPoint(std::min(MaxX(), other.MaxX()), std::min(MaxY(), other.MaxY()));

      bool intersects = true;

      // Return a clean empty rectangle for non-intersecting cases.
      if (newLocation.X() > newMaxPoint.X() || newLocation.Y() > newMaxPoint.Y())
      {
        newLocation = {};
        newMaxPoint = {};
        intersects = false;
      }

      _location = newLocation;
      _size = newMaxPoint - newLocation;
      return intersects;
    }

    /// @brief Sets this to the union of it and @p other, ignoring either if it's empty.
    constexpr void Unite(const Rect &other) noexcept
    {
      // Handle empty special cases first.
      if (other.IsEmpty())
      {
        return;
      }
      if (IsEmpty())
      {
        *this = other;
        return;
      }

      UniteEvenIfEmpty(other);
    }

    constexpr void UniteEvenIfEmpty(const Rect &other) noexcept
    {
      ShiftEdgesTo(std::min(X(), other.X()), std::min(Y(), other.Y()), std::max(MaxX(), other.MaxX()),
                   std::max(MaxY(), other.MaxY()));
    }

    /// @brief Sets this to the union of it and @p other, ignoring either if its size is zero.
    constexpr void UniteIfNonZero(const Rect &other) noexcept
    {
      // Handle empty special cases first.
      if (other.IsZero())
      {
        return;
      }
      if (IsZero())
      {
        *this = other;
        return;
      }

      UniteEvenIfEmpty(other);
    }

    constexpr void Extend(const PointType &point) noexcept
    {
      ShiftEdgesTo(std::min(X(), point.X()), std::min(Y(), point.Y()), std::max(MaxX(), point.X()),
                   std::max(MaxY(), point.Y()));
    }

    constexpr void Extend(const PointType &minPoint, const PointType &maxPoint) noexcept
    {
      krys_debug_assert(minPoint.X() <= maxPoint.X() && minPoint.Y() <= maxPoint.Y());
      ShiftEdgesTo(std::min(X(), minPoint.X()), std::min(Y(), minPoint.Y()), std::max(MaxX(), maxPoint.X()),
                   std::max(MaxY(), maxPoint.Y()));
    }

    constexpr void InflateX(T dx) noexcept
    {
      _location.SetX(_location.X() - dx);
      _size.SetWidth(_size.Width() + dx + dx);
    }

    constexpr void InflateY(T dy) noexcept
    {
      _location.SetY(_location.Y() - dy);
      _size.SetHeight(_size.Height() + dy + dy);
    }

    constexpr void Inflate(T d) noexcept
    {
      InflateX(d);
      InflateY(d);
    }

    constexpr void Inflate(const SizeType &size) noexcept
    {
      InflateX(size.Width());
      InflateY(size.Height());
    }

    constexpr void Inflate(T deltaX, T deltaY, T deltaMaxX, T deltaMaxY) noexcept
    {
      SetX(X() - deltaX);
      SetY(Y() - deltaY);
      SetWidth(Width() + deltaX + deltaMaxX);
      SetHeight(Height() + deltaY + deltaMaxY);
    }

    /// @brief Scales the location and size; integer values are truncated.
    constexpr void Scale(ScaleType scaleX, ScaleType scaleY) noexcept
    {
      if constexpr (Integral<T>)
      {
        _location.SetX(static_cast<T>(static_cast<ScaleType>(X()) * scaleX));
        _location.SetY(static_cast<T>(static_cast<ScaleType>(Y()) * scaleY));
      }
      else
      {
        _location.SetX(X() * scaleX);
        _location.SetY(Y() * scaleY);
      }
      _size.Scale(scaleX, scaleY);
    }

    constexpr void Scale(ScaleType scale) noexcept
    {
      Scale(scale, scale);
    }

    constexpr void Scale(const SizeType &size) noexcept
    requires FloatingPoint<T>
    {
      Scale(size.Width(), size.Height());
    }

    /// @brief How far @p point is outside this on each axis (zero on an axis it's within), towards it.
    KRYS_NODISCARD constexpr SizeType DifferenceToPoint(const PointType &point) const noexcept
    {
      return {DistanceToInterval(point.X(), X(), MaxX()), DistanceToInterval(point.Y(), Y(), MaxY())};
    }

    KRYS_NODISCARD constexpr T DistanceSquaredToPoint(const PointType &point) const noexcept
    {
      return DifferenceToPoint(point).DiagonalLengthSquared();
    }

    KRYS_NODISCARD constexpr Rect TransposedRect() const noexcept
    {
      return {_location.TransposedPoint(), _size.TransposedSize()};
    }

    KRYS_NODISCARD friend constexpr bool operator==(const Rect &, const Rect &) noexcept = default;

    KRYS_NODISCARD friend constexpr Rect Intersection(const Rect &a, const Rect &b) noexcept
    {
      Rect c = a;
      c.Intersect(b);
      return c;
    }

    KRYS_NODISCARD friend constexpr Rect UnionRect(const Rect &a, const Rect &b) noexcept
    {
      Rect c = a;
      c.Unite(b);
      return c;
    }

  private:
    KRYS_NODISCARD constexpr static bool SumOverflows(T a, T b) noexcept
    requires Integral<T>
    {
      int64 sum = static_cast<int64>(a) + b;
      return sum > Limits<T>::max() || sum < Limits<T>::min();
    }

    KRYS_NODISCARD constexpr static T DistanceToInterval(T pos, T start, T end) noexcept
    {
      if (pos < start)
      {
        return start - pos;
      }
      if (pos > end)
      {
        return end - pos;
      }
      return 0;
    }
  };

  template <Number T>
  constexpr Point<T> Point<T>::ConstrainedWithin(const Rect<T> &rect) const noexcept
  {
    return ConstrainedBetween(rect.MinXMinYCorner(), rect.MaxXMaxYCorner());
  }

#pragma region Integer rects

  constexpr RectI &operator-=(RectI &rect, const PointI &offset) noexcept
  {
    rect.Move(-offset.X(), -offset.Y());
    return rect;
  }

  KRYS_NODISCARD constexpr RectI operator-(const RectI &rect, const PointI &offset) noexcept
  {
    RectI result = rect;
    return result -= offset;
  }

#pragma endregion

#pragma region Floating point rects

  constexpr RectF &operator+=(RectF &a, const RectF &b) noexcept
  {
    a.Move(b.X(), b.Y());
    a.SetWidth(a.Width() + b.Width());
    a.SetHeight(a.Height() + b.Height());
    return a;
  }

  KRYS_NODISCARD constexpr RectF operator+(const RectF &a, const RectF &b) noexcept
  {
    return {a.X() + b.X(), a.Y() + b.Y(), a.Width() + b.Width(), a.Height() + b.Height()};
  }

  KRYS_NODISCARD constexpr bool AreEssentiallyEqual(const RectF &a, const RectF &b) noexcept
  {
    return AreEssentiallyEqual(a.Location(), b.Location()) && AreEssentiallyEqual(a.Size(), b.Size());
  }

  /// @brief @p rect with a non-negative width and height, covering the same area.
  KRYS_NODISCARD constexpr RectF NormaliseRect(const RectF &rect) noexcept
  {
    return {std::min(rect.X(), rect.MaxX()), std::min(rect.Y(), rect.MaxY()),
            std::max(rect.Width(), -rect.Width()), std::max(rect.Height(), -rect.Height())};
  }

  KRYS_NODISCARD RectF EncloseRectToDevicePixels(const RectF &rect, float deviceScaleFactor) noexcept;

  KRYS_NODISCARD RectI EnclosingRectI(const RectF &rect) noexcept;

  /// @brief The smallest integer rect enclosing @p rect, except that an empty @p rect stays empty.
  KRYS_NODISCARD RectI EnclosingRectIPreservingEmptyRects(const RectF &rect) noexcept;

  KRYS_NODISCARD RectI RoundedRectI(const RectF &rect) noexcept;

#pragma endregion

  TextStream &operator<<(TextStream &ts, const RectI &value) noexcept;

  TextStream &operator<<(TextStream &ts, const RectF &value) noexcept;

  template <>
  struct MarkableTraits<RectF>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(const RectF &rect) noexcept
    {
      return rect.IsNaN();
    }

    KRYS_NODISCARD constexpr static RectF EmptyValue() noexcept
    {
      return RectF::NanRect();
    }
  };
}
