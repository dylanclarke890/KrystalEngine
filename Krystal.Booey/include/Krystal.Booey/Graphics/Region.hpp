#pragma once

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <utility>

namespace krys::boo
{
  /// @brief A set of integer pixels, stored as horizontal bands (spans) of x-intervals (segments).
  class Region
  {
  public:
    struct Span
    {
      int Y {0};
      uint64 SegmentIndex {0};

      KRYS_NODISCARD friend constexpr bool operator==(const Span &, const Span &) noexcept = default;
    };

    /// @brief The banded representation of a non-rectangular region.
    class Shape
    {
      friend class Region;

      struct UnionOperation;
      struct IntersectOperation;
      struct SubtractOperation;

    private:
      SmallList<int, 32> _segments;
      SmallList<Span, 16> _spans;

    public:
      Shape() noexcept = default;
      Shape(const IntRect &rect) noexcept;

      KRYS_NODISCARD IntRect Bounds() const noexcept;

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return _spans.empty();
      }

      KRYS_NODISCARD bool IsRect() const noexcept
      {
        return _spans.size() <= 2 && _segments.size() <= 2;
      }

      KRYS_NODISCARD uint32 GridSize() const noexcept
      {
        return static_cast<uint32>(_spans.size() * _segments.size());
      }

      KRYS_NODISCARD krys::Span<const Span> Spans() const noexcept
      {
        return {_spans.data(), _spans.size()};
      }

      KRYS_NODISCARD krys::Span<const int> Segments(krys::Span<const Span> spans) const noexcept;

      KRYS_NODISCARD static Shape UnionShapes(const Shape &shape1, const Shape &shape2) noexcept;
      KRYS_NODISCARD static Shape IntersectShapes(const Shape &shape1, const Shape &shape2) noexcept;
      KRYS_NODISCARD static Shape SubtractShapes(const Shape &shape1, const Shape &shape2) noexcept;

      void Translate(const IntSize &offset) noexcept;

      struct CompareContainsOperation;
      struct CompareIntersectsOperation;

      template <typename TCompareOperation>
      KRYS_NODISCARD static bool CompareShapes(const Shape &shape1, const Shape &shape2) noexcept;

      KRYS_NODISCARD static bool IsValidShape(krys::Span<const int> segments,
                                              krys::Span<const Span> spans) noexcept;

      KRYS_NODISCARD static Shape CreateForTesting(SmallList<int, 32> &&segments,
                                                   SmallList<Span, 16> &&spans) noexcept
      {
        return Shape {krys::move(segments), krys::move(spans)};
      }

      KRYS_NODISCARD std::pair<SmallList<int, 32>, SmallList<Span, 16>> DataForTesting() const noexcept
      {
        return {_segments, _spans};
      }

      KRYS_NODISCARD friend bool operator==(const Shape &, const Shape &) noexcept = default;

    private:
      Shape(SmallList<int, 32> &&segments, SmallList<Span, 16> &&spans) noexcept;

      template <typename TOperation>
      KRYS_NODISCARD static Shape ShapeOperation(const Shape &shape1, const Shape &shape2) noexcept;

      void AppendSpan(int y) noexcept;
      void AppendSpan(int y, krys::Span<const int> segments) noexcept;
      void AppendSpans(const Shape &shape, krys::Span<const Span> spans) noexcept;
      KRYS_NODISCARD bool CanCoalesce(krys::Span<const int> segments) const noexcept;
    };

  private:
    IntRect _bounds;
    UniquePtr<Shape> _shape;

  public:
    Region() noexcept;
    Region(const IntRect &rect) noexcept;
    Region(const Region &other) noexcept;
    Region(Region &&other) noexcept;
    ~Region() noexcept;

    Region &operator=(const Region &other) noexcept;
    Region &operator=(Region &&other) noexcept;

    KRYS_NODISCARD IntRect Bounds() const noexcept
    {
      return _bounds;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _bounds.IsEmpty();
    }

    KRYS_NODISCARD bool IsRect() const noexcept
    {
      return !_shape;
    }

    KRYS_NODISCARD SmallList<IntRect, 1> Rects() const noexcept;

    void Unite(const Region &region) noexcept;
    void Intersect(const Region &region) noexcept;
    void Subtract(const Region &region) noexcept;

    void Translate(const IntSize &offset) noexcept;

    // Returns true if the query region is a subset of this region.
    KRYS_NODISCARD bool Contains(const Region &region) const noexcept;
    KRYS_NODISCARD bool Contains(const IntPoint &point) const noexcept;

    // Returns true if the query region intersects any part of this region.
    KRYS_NODISCARD bool Intersects(const Region &region) const noexcept;

    KRYS_NODISCARD uint64 TotalArea() const noexcept;

    KRYS_NODISCARD uint32 GridSize() const noexcept
    {
      return _shape ? _shape->GridSize() : 0;
    }

    KRYS_NODISCARD static Region CreateForTesting(Shape &&shape) noexcept
    {
      return Region {krys::move(shape)};
    }

    KRYS_NODISCARD Shape DataForTesting() const noexcept
    {
      return Data();
    }

    KRYS_NODISCARD friend bool operator==(const Region &a, const Region &b) noexcept
    {
      if (a._bounds != b._bounds)
      {
        return false;
      }
      if (!a._shape || !b._shape)
      {
        return a._shape == b._shape;
      }
      return *a._shape == *b._shape;
    }

  private:
    explicit Region(Shape &&shape) noexcept
    {
      SetShape(krys::move(shape));
    }

    KRYS_NODISCARD Shape Data() const noexcept
    {
      if (_shape)
      {
        return *_shape;
      }
      return _bounds;
    }

    KRYS_NODISCARD UniquePtr<Shape> CopyShape() const noexcept
    {
      return _shape ? CreateUnique<Shape>(*_shape) : nullptr;
    }

    void SetShape(Shape &&shape) noexcept;
  };

  KRYS_NODISCARD inline Region Intersect(const Region &a, const Region &b) noexcept
  {
    Region result(a);
    result.Intersect(b);
    return result;
  }

  KRYS_NODISCARD inline Region Subtract(const Region &a, const Region &b) noexcept
  {
    Region result(a);
    result.Subtract(b);
    return result;
  }

  KRYS_NODISCARD inline Region Translate(const Region &region, const IntSize &offset) noexcept
  {
    Region result(region);
    result.Translate(offset);
    return result;
  }
}
