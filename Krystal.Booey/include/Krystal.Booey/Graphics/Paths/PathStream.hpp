/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Paths/PathImpl.hpp"
#include "Krystal.Booey/Graphics/Paths/PathSegment.hpp"
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
