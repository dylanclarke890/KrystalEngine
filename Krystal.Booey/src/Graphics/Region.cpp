#include "Krystal.Booey/Graphics/Region.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>
#include <compare>

namespace krys::boo
{
  namespace
  {
    template <typename T>
    constexpr void Skip(krys::Span<T> &span, size_t count) noexcept
    {
      span = span.subspan(count);
    }

    KRYS_NODISCARD krys::Span<const int> SegmentsForSpanSegmentIndices(krys::Span<const int> segments,
                                                                       size_t start, size_t end) noexcept
    {
      if (segments.size() <= end)
      {
        return {};
      }
      return segments.subspan(start, end - start);
    }
  }

  Region::Region() noexcept = default;

  Region::Region(const IntRect &rect) noexcept : _bounds(rect)
  {
  }

  Region::Region(const Region &other) noexcept : _bounds(other._bounds), _shape(other.CopyShape())
  {
  }

  Region::Region(Region &&other) noexcept
      : _bounds(krys::move(other._bounds)), _shape(krys::move(other._shape))
  {
  }

  Region::~Region() noexcept = default;

  Region &Region::operator=(const Region &other) noexcept
  {
    _bounds = other._bounds;
    _shape = other.CopyShape();
    return *this;
  }

  Region &Region::operator=(Region &&other) noexcept
  {
    _bounds = krys::move(other._bounds);
    _shape = krys::move(other._shape);
    return *this;
  }

  SmallList<IntRect, 1> Region::Rects() const noexcept
  {
    SmallList<IntRect, 1> rects;

    if (!_shape)
    {
      if (!_bounds.IsEmpty())
      {
        rects.push_back(_bounds);
      }
      return rects;
    }

    for (auto spans = _shape->Spans(); spans.size() > 1; Skip(spans, 1))
    {
      int y = spans[0].Y;
      int height = spans[1].Y - y; // Ok since isValidShape ensures increasing Span::y.

      for (auto segments = _shape->Segments(spans); segments.size() > 1; Skip(segments, 2))
      {
        int x = segments[0];
        int width = segments[1] - x; // Ok since isValidShape ensures increasing segments.

        rects.push_back(IntRect(x, y, width, height));
      }
    }

    return rects;
  }

  bool Region::Contains(const Region &region) const noexcept
  {
    if (!_bounds.Contains(region._bounds))
    {
      return false;
    }

    if (!_shape)
    {
      return true;
    }

    return Shape::CompareShapes<Shape::CompareContainsOperation>(
      *_shape, region._shape ? *region._shape : Shape(region._bounds));
  }

  bool Region::Contains(const IntPoint &point) const noexcept
  {
    if (!_bounds.Contains(point))
    {
      return false;
    }

    if (!_shape)
    {
      return true;
    }

    for (auto spans = _shape->Spans(); spans.size() > 1; Skip(spans, 1))
    {
      int y = spans[0].Y;
      int maxY = spans[1].Y;

      if (y > point.Y())
      {
        break;
      }
      if (maxY <= point.Y())
      {
        continue;
      }

      for (auto segments = _shape->Segments(spans); segments.size() > 1; Skip(segments, 2))
      {
        int x = segments[0];
        int maxX = segments[1];

        if (x > point.X())
        {
          break;
        }
        if (maxX > point.X())
        {
          return true;
        }
      }
    }

    return false;
  }

  bool Region::Intersects(const Region &region) const noexcept
  {
    if (!_bounds.Intersects(region._bounds))
    {
      return false;
    }

    if (!_shape && !region._shape)
    {
      return true;
    }

    return Shape::CompareShapes<Shape::CompareIntersectsOperation>(
      _shape ? *_shape : _bounds, region._shape ? *region._shape : region._bounds);
  }

  uint64 Region::TotalArea() const noexcept
  {
    uint64 totalArea = 0;

    for (auto &rect : Rects())
    {
      totalArea += (rect.Width() * rect.Height());
    }

    return totalArea;
  }

  template <typename TCompareOperation>
  bool Region::Shape::CompareShapes(const Shape &aShape, const Shape &bShape) noexcept
  {
    bool result = TCompareOperation::DefaultResult;

    auto aSpans = aShape.Spans();
    auto bSpans = bShape.Spans();

    bool aHadSegmentInPreviousSpan = false;
    bool bHadSegmentInPreviousSpan = false;
    while (aSpans.size() > 1 && bSpans.size() > 1)
    {
      int aY = aSpans[0].Y;
      int aMaxY = aSpans[1].Y;
      int bY = bSpans[0].Y;
      int bMaxY = bSpans[1].Y;

      auto aSegments = aShape.Segments(aSpans);
      auto bSegments = bShape.Segments(bSpans);

      // Look for a non-overlapping part of the spans. If B had a segment in its previous span, then we
      // already tested A against B within that span.
      bool aHasSegmentInSpan = !aSegments.empty();
      bool bHasSegmentInSpan = !bSegments.empty();
      if (aY < bY && !bHadSegmentInPreviousSpan && aHasSegmentInSpan && TCompareOperation::AOutsideB(result))
      {
        return result;
      }
      if (bY < aY && !aHadSegmentInPreviousSpan && bHasSegmentInSpan && TCompareOperation::BOutsideA(result))
      {
        return result;
      }

      aHadSegmentInPreviousSpan = aHasSegmentInSpan;
      bHadSegmentInPreviousSpan = bHasSegmentInSpan;

      bool spansOverlap = bMaxY > aY && bY < aMaxY;
      if (spansOverlap)
      {
        while (!aSegments.empty() && !bSegments.empty())
        {
          int aX = aSegments[0];
          int aMaxX = aSegments[1];
          int bX = bSegments[0];
          int bMaxX = bSegments[1];

          bool segmentsOverlap = bMaxX > aX && bX < aMaxX;
          if (segmentsOverlap && TCompareOperation::AOverlapsB(result))
          {
            return result;
          }
          if (aX < bX && TCompareOperation::AOutsideB(result))
          {
            return result;
          }
          if (bX < aX && TCompareOperation::BOutsideA(result))
          {
            return result;
          }

          if (aMaxX < bMaxX)
          {
            Skip(aSegments, 2);
          }
          else if (bMaxX < aMaxX)
          {
            Skip(bSegments, 2);
          }
          else
          {
            Skip(aSegments, 2);
            Skip(bSegments, 2);
          }
        }

        if (!aSegments.empty() && TCompareOperation::AOutsideB(result))
        {
          return result;
        }
        if (!bSegments.empty() && TCompareOperation::BOutsideA(result))
        {
          return result;
        }
      }

      if (aMaxY < bMaxY)
      {
        Skip(aSpans, 1);
      }
      else if (bMaxY < aMaxY)
      {
        Skip(bSpans, 1);
      }
      else
      {
        Skip(aSpans, 1);
        Skip(bSpans, 1);
      }
    }

    if (aSpans.size() > 1 && TCompareOperation::AOutsideB(result))
    {
      return result;
    }
    if (bSpans.size() > 1 && TCompareOperation::BOutsideA(result))
    {
      return result;
    }

    return result;
  }

  struct Region::Shape::CompareContainsOperation
  {
    constexpr static bool DefaultResult = true;

    KRYS_NODISCARD constexpr static bool AOutsideB(bool & /* result */) noexcept
    {
      return false;
    }

    KRYS_NODISCARD constexpr static bool BOutsideA(bool &result) noexcept
    {
      result = false;
      return true;
    }

    KRYS_NODISCARD constexpr static bool AOverlapsB(bool & /* result */) noexcept
    {
      return false;
    }
  };

  struct Region::Shape::CompareIntersectsOperation
  {
    constexpr static bool DefaultResult = false;

    KRYS_NODISCARD constexpr static bool AOutsideB(bool & /* result */) noexcept
    {
      return false;
    }

    KRYS_NODISCARD constexpr static bool BOutsideA(bool & /* result */) noexcept
    {
      return false;
    }

    KRYS_NODISCARD constexpr static bool AOverlapsB(bool &result) noexcept
    {
      result = true;
      return true;
    }
  };

  Region::Shape::Shape(const IntRect &rect) noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    _segments.push_back(rect.X());
    _segments.push_back(rect.MaxX());

    _spans.push_back({rect.Y(), 0});
    _spans.push_back({rect.MaxY(), 2});
  }

  Region::Shape::Shape(SmallList<int, 32> &&segments, SmallList<Span, 16> &&spans) noexcept
      : _segments(krys::move(segments)), _spans(krys::move(spans))
  {
    krys_debug_assert(IsValidShape(krys::Span<const int>(_segments.data(), _segments.size()), Spans()));
  }

  void Region::Shape::AppendSpan(int y) noexcept
  {
    _spans.push_back({y, _segments.size()});
  }

  bool Region::Shape::CanCoalesce(krys::Span<const int> segments) const noexcept
  {
    if (_spans.empty())
    {
      return false;
    }

    auto lastSegments =
      krys::Span<const int>(_segments.data(), _segments.size()).subspan(_spans.back().SegmentIndex);
    return std::ranges::equal(segments, lastSegments);
  }

  void Region::Shape::AppendSpan(int y, krys::Span<const int> segments) noexcept
  {
    if (CanCoalesce(segments))
    {
      return;
    }

    AppendSpan(y);
    _segments.append(segments.begin(), segments.end());
  }

  void Region::Shape::AppendSpans(const Shape &shape, krys::Span<const Span> spans) noexcept
  {
    for (; !spans.empty(); Skip(spans, 1))
    {
      AppendSpan(spans[0].Y, shape.Segments(spans));
    }
  }

  krys::Span<const int> Region::Shape::Segments(krys::Span<const Span> span) const noexcept
  {
    krys_debug_assert(span.data() >= _spans.data());
    krys_debug_assert(span.data() < _spans.data() + _spans.size());
    krys_debug_assert(span.data() + span.size() <= _spans.data() + _spans.size());

    // Check if this span has any segments.
    if (span[0].SegmentIndex == _segments.size())
    {
      return {};
    }

    return krys::Span<const int>(_segments.data(), _segments.size())
      .subspan(span[0].SegmentIndex, span[1].SegmentIndex - span[0].SegmentIndex);
  }

  IntRect Region::Shape::Bounds() const noexcept
  {
    if (IsEmpty())
    {
      return IntRect();
    }

    auto spans = Spans();
    int minY = spans.front().Y;
    int maxY = spans.back().Y;

    int minX = Limits<int>::max();
    int maxX = Limits<int>::min();

    while (spans.size() > 1)
    {
      auto segments = Segments(spans);
      if (!segments.empty())
      {
        auto &firstSegment = segments.front();
        auto &lastSegment = segments.back();
        krys_debug_assert(&firstSegment != &lastSegment);

        if (firstSegment < minX)
        {
          minX = firstSegment;
        }

        if (lastSegment > maxX)
        {
          maxX = lastSegment;
        }
      }

      Skip(spans, 1);
    }

    krys_debug_assert(minX <= maxX);
    krys_debug_assert(minY <= maxY);

    int64 width = static_cast<int64>(maxX) - minX;
    int64 height = static_cast<int64>(maxY) - minY;
    return IntRect(minX, minY, static_cast<int>(std::min<int64>(width, Limits<int32>::max())),
                   static_cast<int>(std::min<int64>(height, Limits<int32>::max())));
  }

  void Region::Shape::Translate(const IntSize &offset) noexcept
  {
    for (auto &segment : _segments)
    {
      segment += offset.Width();
    }
    for (auto &span : _spans)
    {
      span.Y += offset.Height();
    }
  }

  template <typename TOperation>
  Region::Shape Region::Shape::ShapeOperation(const Shape &shape1, const Shape &shape2) noexcept
  {
    static_assert(
      !(!TOperation::ShouldAddRemainingSegmentsFromSpan1 && TOperation::ShouldAddRemainingSegmentsFromSpan2),
      "invalid segment combination");
    static_assert(
      !(!TOperation::ShouldAddRemainingSpansFromShape1 && TOperation::ShouldAddRemainingSpansFromShape2),
      "invalid span combination");

    Shape result;
    if (TOperation::TrySimpleOperation(shape1, shape2, result))
    {
      return result;
    }

    auto spans1 = shape1.Spans();
    auto spans2 = shape2.Spans();

    krys::Span<const int> segments1;
    krys::Span<const int> segments2;

    // Iterate over all spans.
    while (!spans1.empty() && !spans2.empty())
    {
      int y = 0;
      auto test = spans1[0].Y <=> spans2[0].Y;

      if (test <= 0)
      {
        y = spans1[0].Y;

        segments1 = shape1.Segments(spans1);
        Skip(spans1, 1);
      }
      if (test >= 0)
      {
        y = spans2[0].Y;

        segments2 = shape2.Segments(spans2);
        Skip(spans2, 1);
      }

      int flag = 0;
      int oldFlag = 0;

      auto s1 = segments1;
      auto s2 = segments2;

      SmallList<int, 32> segments;

      // Now iterate over the segments in each span and construct a new vector of segments.
      while (!s1.empty() && !s2.empty())
      {
        auto segmentTest = s1[0] <=> s2[0];
        int x = 0;

        if (segmentTest <= 0)
        {
          x = s1[0];
          flag = flag ^ 1;
          Skip(s1, 1);
        }
        if (segmentTest >= 0)
        {
          x = s2[0];
          flag = flag ^ 2;
          Skip(s2, 1);
        }

        if (flag == TOperation::OpCode || oldFlag == TOperation::OpCode)
        {
          segments.push_back(x);
        }

        oldFlag = flag;
      }

      // Add any remaining segments.
      if (TOperation::ShouldAddRemainingSegmentsFromSpan1 && !s1.empty())
      {
        segments.append(s1.begin(), s1.end());
      }
      else if (TOperation::ShouldAddRemainingSegmentsFromSpan2 && !s2.empty())
      {
        segments.append(s2.begin(), s2.end());
      }

      // Add the span.
      if (!segments.empty() || !result.IsEmpty())
      {
        result.AppendSpan(y, krys::Span<const int>(segments.data(), segments.size()));
      }
    }

    // Add any remaining spans.
    if (TOperation::ShouldAddRemainingSpansFromShape1 && !spans1.empty())
    {
      result.AppendSpans(shape1, spans1);
    }
    else if (TOperation::ShouldAddRemainingSpansFromShape2 && !spans2.empty())
    {
      result.AppendSpans(shape2, spans2);
    }

    return result;
  }

  struct Region::Shape::UnionOperation
  {
    KRYS_NODISCARD static bool TrySimpleOperation(const Shape &shape1, const Shape &shape2,
                                                  Shape &result) noexcept
    {
      if (shape1.IsEmpty())
      {
        result = shape2;
        return true;
      }

      return false;
    }

    constexpr static int OpCode = 0;

    constexpr static bool ShouldAddRemainingSegmentsFromSpan1 = true;
    constexpr static bool ShouldAddRemainingSegmentsFromSpan2 = true;
    constexpr static bool ShouldAddRemainingSpansFromShape1 = true;
    constexpr static bool ShouldAddRemainingSpansFromShape2 = true;
  };

  Region::Shape Region::Shape::UnionShapes(const Shape &shape1, const Shape &shape2) noexcept
  {
    return ShapeOperation<UnionOperation>(shape1, shape2);
  }

  struct Region::Shape::IntersectOperation
  {
    KRYS_NODISCARD static bool TrySimpleOperation(const Shape &, const Shape &, Shape &) noexcept
    {
      return false;
    }

    constexpr static int OpCode = 3;

    constexpr static bool ShouldAddRemainingSegmentsFromSpan1 = false;
    constexpr static bool ShouldAddRemainingSegmentsFromSpan2 = false;
    constexpr static bool ShouldAddRemainingSpansFromShape1 = false;
    constexpr static bool ShouldAddRemainingSpansFromShape2 = false;
  };

  Region::Shape Region::Shape::IntersectShapes(const Shape &shape1, const Shape &shape2) noexcept
  {
    return ShapeOperation<IntersectOperation>(shape1, shape2);
  }

  struct Region::Shape::SubtractOperation
  {
    KRYS_NODISCARD static bool TrySimpleOperation(const Shape &, const Shape &, Region::Shape &) noexcept
    {
      return false;
    }

    constexpr static int OpCode = 1;

    constexpr static bool ShouldAddRemainingSegmentsFromSpan1 = true;
    constexpr static bool ShouldAddRemainingSegmentsFromSpan2 = false;
    constexpr static bool ShouldAddRemainingSpansFromShape1 = true;
    constexpr static bool ShouldAddRemainingSpansFromShape2 = false;
  };

  Region::Shape Region::Shape::SubtractShapes(const Shape &shape1, const Shape &shape2) noexcept
  {
    return ShapeOperation<SubtractOperation>(shape1, shape2);
  }

  void Region::Intersect(const Region &region) noexcept
  {
    if (_bounds.IsEmpty())
    {
      return;
    }
    if (!_bounds.Intersects(region._bounds))
    {
      _shape = nullptr;
      _bounds = IntRect();
      return;
    }
    if (!_shape && !region._shape)
    {
      _bounds = boo::Intersection(_bounds, region._bounds);
      return;
    }

    SetShape(
      Shape::IntersectShapes(_shape ? *_shape : _bounds, region._shape ? *region._shape : region._bounds));
  }

  void Region::Unite(const Region &region) noexcept
  {
    if (region.IsEmpty())
    {
      return;
    }
    if (IsEmpty())
    {
      _bounds = region._bounds;
      _shape = region.CopyShape();
      return;
    }
    if (region.IsRect() && region._bounds.Contains(_bounds))
    {
      _bounds = region._bounds;
      _shape = nullptr;
      return;
    }
    if (Contains(region))
    {
      return;
    }

    SetShape(Shape::UnionShapes(_shape ? *_shape : _bounds, region._shape ? *region._shape : region._bounds));
  }

  void Region::Subtract(const Region &region) noexcept
  {
    if (IsEmpty())
    {
      return;
    }
    if (region.IsEmpty())
    {
      return;
    }
    if (!_bounds.Intersects(region._bounds))
    {
      return;
    }

    SetShape(
      Shape::SubtractShapes(_shape ? *_shape : _bounds, region._shape ? *region._shape : region._bounds));
  }

  void Region::Translate(const IntSize &offset) noexcept
  {
    _bounds.Move(offset);
    if (_shape)
    {
      _shape->Translate(offset);
    }
  }

  void Region::SetShape(Shape &&shape) noexcept
  {
    _bounds = shape.Bounds();

    if (shape.IsRect())
    {
      _shape = nullptr;
      return;
    }

    if (!_shape)
    {
      _shape = CreateUnique<Shape>(krys::move(shape));
    }
    else
    {
      *_shape = krys::move(shape);
    }
  }

  bool Region::Shape::IsValidShape(krys::Span<const int> segments, krys::Span<const Span> spans) noexcept
  {
    const size_t spansSize = spans.size();
    const size_t segmentsSize = segments.size();
    if (!spansSize)
    {
      return !segmentsSize;
    }
    if (!segmentsSize)
    {
      return !spansSize;
    }
    if (spansSize == 1) KRYS_UNLIKELY
    {
      return false;
    }
    if (segmentsSize % 2) KRYS_UNLIKELY
    {
      return false;
    }
    for (size_t i = 0; i < spansSize; ++i)
    {
      auto &span = spans[i];
      if (span.SegmentIndex > segmentsSize) KRYS_UNLIKELY
      {
        return false;
      }
      if (span.SegmentIndex % 2) KRYS_UNLIKELY
      {
        return false;
      }

      if (i < spansSize - 1)
      {
        auto &nextSpan = spans[i + 1];

        if (span.Y >= nextSpan.Y) KRYS_UNLIKELY
        {
          return false;
        }
        if (span.SegmentIndex > nextSpan.SegmentIndex) KRYS_UNLIKELY
        {
          return false;
        }

        auto spanSegments = SegmentsForSpanSegmentIndices(segments, span.SegmentIndex, nextSpan.SegmentIndex);
        int lastX = Limits<int>::min();
        for (int segment : spanSegments)
        {
          if (lastX > segment) KRYS_UNLIKELY
          {
            return false;
          }
          lastX = segment;
        }
      }
      else if (span.SegmentIndex != segments.size()) KRYS_UNLIKELY
      {
        return false;
      }
    }
    return true;
  }
}
