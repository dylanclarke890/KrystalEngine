#pragma once

#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief The edges of the viewport (or scroller) a fixed or sticky box is placed against.
  enum class AnchorEdge : uint8
  {
    Left,
    Right,
    Top,
    Bottom
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
      StickyPositionConstraint
    };

  protected:
    FloatSize _alignmentOffset;
    EnumSet<AnchorEdge> _anchorEdges;
    FloatRect _viewportRectAtLastLayout;
    FloatPoint _layerPositionAtLastLayout;

  public:
    virtual ~ViewportConstraints() noexcept = default;

    KRYS_NODISCARD virtual ConstraintType GetConstraintType() const noexcept = 0;

    KRYS_NODISCARD EnumSet<AnchorEdge> AnchorEdges() const noexcept
    {
      return _anchorEdges;
    }
    KRYS_NODISCARD bool HasAnchorEdge(AnchorEdge edge) const noexcept
    {
      return _anchorEdges.Contains(edge);
    }
    void AddAnchorEdge(AnchorEdge edge) noexcept
    {
      _anchorEdges.Add(edge);
    }
    void SetAnchorEdges(EnumSet<AnchorEdge> edges) noexcept
    {
      _anchorEdges = edges;
    }

    KRYS_NODISCARD FloatSize AlignmentOffset() const noexcept
    {
      return _alignmentOffset;
    }
    void SetAlignmentOffset(FloatSize offset) noexcept
    {
      _alignmentOffset = offset;
    }

    /// @brief Where the layer goes in @p viewportRect: moved with the edges it's anchored to.
    KRYS_NODISCARD FloatPoint ViewportRelativeLayerPosition(const FloatRect &viewportRect) const noexcept;

    KRYS_NODISCARD const FloatRect &ViewportRectAtLastLayout() const noexcept
    {
      return _viewportRectAtLastLayout;
    }
    void SetViewportRectAtLastLayout(const FloatRect &rect) noexcept
    {
      _viewportRectAtLastLayout = rect;
    }

    KRYS_NODISCARD const FloatPoint &LayerPositionAtLastLayout() const noexcept
    {
      return _layerPositionAtLastLayout;
    }
    void SetLayerPositionAtLastLayout(FloatPoint position) noexcept
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
    FloatRect _constrainingRectAtLastLayout;
    FloatRect _containingBlockRect;
    FloatRect _stickyBoxRect;
    FloatSize _stickyOffsetAtLastLayout;
    FloatSize _anchorLayerOffsetAtLastLayout;

  public:
    StickyPositionViewportConstraints() noexcept = default;

    KRYS_NODISCARD ConstraintType GetConstraintType() const noexcept override
    {
      return ConstraintType::StickyPositionConstraint;
    }

    /// @brief How far the sticky box moves from where it's laid out to stay in @p constrainingRect, as far as
    /// its containing block lets it.
    KRYS_NODISCARD FloatSize ComputeStickyOffset(const FloatRect &constrainingRect) const noexcept;

    KRYS_NODISCARD const FloatSize &AnchorLayerOffsetAtLastLayout() const noexcept
    {
      return _anchorLayerOffsetAtLastLayout;
    }
    void SetAnchorLayerOffsetAtLastLayout(FloatSize offset) noexcept
    {
      _anchorLayerOffsetAtLastLayout = offset;
    }

    KRYS_NODISCARD FloatSize StickyOffsetAtLastLayout() const noexcept
    {
      return _stickyOffsetAtLastLayout;
    }
    void SetStickyOffsetAtLastLayout(FloatSize offset) noexcept
    {
      _stickyOffsetAtLastLayout = offset;
    }

    KRYS_NODISCARD FloatPoint
      AnchorLayerPositionForConstrainingRect(const FloatRect &constrainingRect) const noexcept;
    KRYS_NODISCARD FloatPoint AnchorLayerPositionAtLastLayout() const noexcept;

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
    KRYS_NODISCARD FloatRect ConstrainingRectAtLastLayout() const noexcept
    {
      return _constrainingRectAtLastLayout;
    }
    void SetConstrainingRectAtLastLayout(const FloatRect &rect) noexcept
    {
      _constrainingRectAtLastLayout = rect;
    }

    /// @brief In the scrolling ancestor's coordinate space.
    KRYS_NODISCARD FloatRect ContainingBlockRect() const noexcept
    {
      return _containingBlockRect;
    }
    void SetContainingBlockRect(const FloatRect &rect) noexcept
    {
      _containingBlockRect = rect;
    }

    /// @brief In the scrolling ancestor's coordinate space.
    KRYS_NODISCARD FloatRect StickyBoxRect() const noexcept
    {
      return _stickyBoxRect;
    }
    void SetStickyBoxRect(const FloatRect &rect) noexcept
    {
      _stickyBoxRect = rect;
    }

    /// @brief The smallest rectangle, in the scrolling ancestor's coordinate space, that encloses the sticky
    /// box at every permissible position during its sticky travel.
    KRYS_NODISCARD FloatRect ComputeStickyExtent() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::FixedPositionViewportConstraints)
  KRYS_NODISCARD static bool IsType(const krys::boo::ViewportConstraints &constraints) noexcept
  {
    return constraints.GetConstraintType()
           == krys::boo::ViewportConstraints::ConstraintType::FixedPositionConstraint;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::StickyPositionViewportConstraints)
  KRYS_NODISCARD static bool IsType(const krys::boo::ViewportConstraints &constraints) noexcept
  {
    return constraints.GetConstraintType()
           == krys::boo::ViewportConstraints::ConstraintType::StickyPositionConstraint;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
