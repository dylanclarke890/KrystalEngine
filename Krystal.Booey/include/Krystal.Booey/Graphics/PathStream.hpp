#pragma once

#include "Krystal.Booey/Graphics/PathImpl.hpp"
#include "Krystal.Booey/Graphics/PathSegment.hpp"
#include "Krystal.Core/Types/DataRef.hpp"

#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class PathStream final : public PathImpl
  {
  public:
    KRYS_NODISCARD static Ref<PathStream> Create() noexcept;
    KRYS_NODISCARD static Ref<PathStream> Create(PathSegment &&) noexcept;
    KRYS_NODISCARD static Ref<PathStream> Create(const List<FloatPoint> &) noexcept;
    KRYS_NODISCARD static Ref<PathStream> Create(List<PathSegment> &&) noexcept;

    KRYS_NODISCARD bool DefinitelyEqual(const PathImpl &) const noexcept final;
    KRYS_NODISCARD Ref<PathImpl> Copy() const noexcept final;

    void Add(PathMoveTo) noexcept final;
    void Add(PathLineTo) noexcept final;
    void Add(PathQuadCurveTo) noexcept final;
    void Add(PathBezierCurveTo) noexcept final;
    void Add(PathArcTo) noexcept final;
    void Add(PathArc) noexcept final;
    void Add(PathClosedArc) noexcept final;
    void Add(PathEllipse) noexcept final;
    void Add(PathEllipseInRect) noexcept final;
    void Add(PathRect) noexcept final;
    void Add(PathRoundedRect) noexcept final;
    void Add(PathContinuousRoundedRect) noexcept final;
    void Add(PathCloseSubpath) noexcept final;

    KRYS_NODISCARD const List<PathSegment> &Segments() const noexcept
    {
      return _segments;
    }

    void ApplySegments(const PathSegmentApplier &) const noexcept final;
    KRYS_NODISCARD bool ApplyElements(const PathElementApplier &) const noexcept final;

    KRYS_NODISCARD bool Transform(const AffineTransform &) noexcept final;

    KRYS_NODISCARD FloatRect FastBoundingRect() const noexcept final;
    KRYS_NODISCARD FloatRect BoundingRect() const noexcept final;

    KRYS_NODISCARD bool HasSubpaths() const noexcept final;

    KRYS_NODISCARD static FloatRect ComputeFastBoundingRect(std::span<const PathSegment>) noexcept;
    KRYS_NODISCARD static FloatRect ComputeBoundingRect(std::span<const PathSegment>) noexcept;
    KRYS_NODISCARD static bool ComputeHasSubpaths(std::span<const PathSegment>) noexcept;

  private:
    PathStream() noexcept = default;
    PathStream(PathSegment &&) noexcept;
    PathStream(List<PathSegment> &&) noexcept;
    PathStream(const List<PathSegment> &) noexcept;

    KRYS_NODISCARD static Ref<PathStream> Create(const List<PathSegment> &) noexcept;

    KRYS_NODISCARD const PathMoveTo *LastIfMoveTo() const noexcept;

    KRYS_NODISCARD bool IsPathStream() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD Maybe<PathSegment> SingleSegment() const noexcept final;

    KRYS_NODISCARD bool IsClosed() const noexcept final;
    KRYS_NODISCARD FloatPoint CurrentPoint() const noexcept final;

    KRYS_NODISCARD List<PathSegment> &Segments() noexcept
    {
      return _segments;
    }

    List<PathSegment> _segments;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::PathStream)
  KRYS_NODISCARD static bool IsType(const krys::boo::PathImpl &pathImpl) noexcept
  {
    return pathImpl.IsPathStream();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
