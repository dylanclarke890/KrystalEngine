/*
 * Copyright (C) 2012 Apple Inc. All rights reserved.
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
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::web
{
  /// @brief The edges of the viewport (or scroller) a fixed or sticky box is placed against.
  enum class AnchorEdge : uint8
  {
    Left,
    Right,
    Top,
    Bottom,
  };

  /// @brief What's needed to place again a box whose layout depends on the viewport rect (fixed and sticky
  /// positions) when the view scrolls or zooms.
  /// FIXME: Absolute position constraints and the text stream dumps, once there's a scrolling tree.
  class ViewportConstraints
  {
  public:
    enum class ConstraintType : uint8
    {
      FixedPositionConstraint,
      StickyPositionConstraint,
    };

  protected:
    SizeF _alignmentOffset;
    EnumSet<AnchorEdge> _anchorEdges;
    RectF _viewportRectAtLastLayout;
    PointF _layerPositionAtLastLayout;

  public:
    virtual ~ViewportConstraints() noexcept = default;

    KRYS_NODISCARD virtual ConstraintType GetConstraintType() const noexcept = 0;

    KRYS_NODISCARD EnumSet<AnchorEdge> AnchorEdges() const noexcept
    {
      return _anchorEdges;
    }

    KRYS_NODISCARD bool HasAnchorEdge(AnchorEdge edge) const noexcept
    {
      return _anchorEdges.contains(edge);
    }

    void AddAnchorEdge(AnchorEdge edge) noexcept
    {
      _anchorEdges.Add(edge);
    }

    void SetAnchorEdges(EnumSet<AnchorEdge> edges) noexcept
    {
      _anchorEdges = edges;
    }

    KRYS_NODISCARD SizeF AlignmentOffset() const noexcept
    {
      return _alignmentOffset;
    }

    void SetAlignmentOffset(SizeF offset) noexcept
    {
      _alignmentOffset = offset;
    }

    /// @brief Where the layer goes in @p viewportRect: moved with the edges it's anchored to.
    KRYS_NODISCARD PointF ViewportRelativeLayerPosition(const RectF &viewportRect) const noexcept;

    KRYS_NODISCARD const RectF &ViewportRectAtLastLayout() const noexcept
    {
      return _viewportRectAtLastLayout;
    }

    void SetViewportRectAtLastLayout(const RectF &rect) noexcept
    {
      _viewportRectAtLastLayout = rect;
    }

    KRYS_NODISCARD const PointF &LayerPositionAtLastLayout() const noexcept
    {
      return _layerPositionAtLastLayout;
    }

    void SetLayerPositionAtLastLayout(PointF position) noexcept
    {
      _layerPositionAtLastLayout = position;
    }

  protected:
    ViewportConstraints() noexcept = default;
  };

  /// @brief The constraints of a fixed box.
  class FixedPositionViewportConstraints final : public ViewportConstraints
  {
  public:
    FixedPositionViewportConstraints() noexcept = default;

    KRYS_NODISCARD ConstraintType GetConstraintType() const noexcept override
    {
      return ConstraintType::FixedPositionConstraint;
    }
  };

  /// @brief The constraints of a sticky box: its insets from the edges it sticks to, the rect that constrains
  /// it, and its containing block's and its own rects, which give the offset that keeps it in view.
  class StickyPositionViewportConstraints final : public ViewportConstraints
  {
  private:
    float _leftOffset {0};
    float _rightOffset {0};
    float _topOffset {0};
    float _bottomOffset {0};
    RectF _constrainingRectAtLastLayout;
    RectF _containingBlockRect;
    RectF _stickyBoxRect;
    SizeF _stickyOffsetAtLastLayout;
    SizeF _anchorLayerOffsetAtLastLayout;

  public:
    StickyPositionViewportConstraints() noexcept = default;

    KRYS_NODISCARD ConstraintType GetConstraintType() const noexcept override
    {
      return ConstraintType::StickyPositionConstraint;
    }

    /// @brief How far the sticky box moves from where it's laid out to stay in @p constrainingRect, as far as
    /// its containing block lets it.
    KRYS_NODISCARD SizeF ComputeStickyOffset(const RectF &constrainingRect) const noexcept;

    KRYS_NODISCARD const SizeF &AnchorLayerOffsetAtLastLayout() const noexcept
    {
      return _anchorLayerOffsetAtLastLayout;
    }

    void SetAnchorLayerOffsetAtLastLayout(SizeF offset) noexcept
    {
      _anchorLayerOffsetAtLastLayout = offset;
    }

    KRYS_NODISCARD SizeF StickyOffsetAtLastLayout() const noexcept
    {
      return _stickyOffsetAtLastLayout;
    }

    void SetStickyOffsetAtLastLayout(SizeF offset) noexcept
    {
      _stickyOffsetAtLastLayout = offset;
    }

    KRYS_NODISCARD PointF
      AnchorLayerPositionForConstrainingRect(const RectF &constrainingRect) const noexcept;

    KRYS_NODISCARD PointF AnchorLayerPositionAtLastLayout() const noexcept;

    KRYS_NODISCARD float LeftOffset() const noexcept
    {
      return _leftOffset;
    }

    KRYS_NODISCARD float RightOffset() const noexcept
    {
      return _rightOffset;
    }

    KRYS_NODISCARD float TopOffset() const noexcept
    {
      return _topOffset;
    }

    KRYS_NODISCARD float BottomOffset() const noexcept
    {
      return _bottomOffset;
    }

    void SetLeftOffset(float offset) noexcept
    {
      _leftOffset = offset;
    }

    void SetRightOffset(float offset) noexcept
    {
      _rightOffset = offset;
    }

    void SetTopOffset(float offset) noexcept
    {
      _topOffset = offset;
    }

    void SetBottomOffset(float offset) noexcept
    {
      _bottomOffset = offset;
    }

    /// @brief The viewport rect if the box sticks to the viewport, and the overflow area's scrolled clip rect
    /// if it sticks inside an overflow area.
    KRYS_NODISCARD RectF ConstrainingRectAtLastLayout() const noexcept
    {
      return _constrainingRectAtLastLayout;
    }

    void SetConstrainingRectAtLastLayout(const RectF &rect) noexcept
    {
      _constrainingRectAtLastLayout = rect;
    }

    /// @brief In the scrolling ancestor's coordinate space.
    KRYS_NODISCARD RectF ContainingBlockRect() const noexcept
    {
      return _containingBlockRect;
    }

    void SetContainingBlockRect(const RectF &rect) noexcept
    {
      _containingBlockRect = rect;
    }

    /// @brief In the scrolling ancestor's coordinate space.
    KRYS_NODISCARD RectF StickyBoxRect() const noexcept
    {
      return _stickyBoxRect;
    }

    void SetStickyBoxRect(const RectF &rect) noexcept
    {
      _stickyBoxRect = rect;
    }

    /// @brief The smallest rectangle, in the scrolling ancestor's coordinate space, that encloses the sticky
    /// box at every permissible position during its sticky travel.
    KRYS_NODISCARD RectF ComputeStickyExtent() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::FixedPositionViewportConstraints)
  KRYS_NODISCARD static bool IsType(const krys::web::ViewportConstraints &constraints) noexcept
  {
    return constraints.GetConstraintType()
           == krys::web::ViewportConstraints::ConstraintType::FixedPositionConstraint;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::StickyPositionViewportConstraints)
  KRYS_NODISCARD static bool IsType(const krys::web::ViewportConstraints &constraints) noexcept
  {
    return constraints.GetConstraintType()
           == krys::web::ViewportConstraints::ConstraintType::StickyPositionConstraint;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
