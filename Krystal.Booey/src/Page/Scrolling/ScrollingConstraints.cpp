#include "Krystal.Booey/Page/Scrolling/ScrollingConstraints.hpp"
#include <algorithm>

namespace krys::boo
{
  FloatPoint ViewportConstraints::ViewportRelativeLayerPosition(const FloatRect &viewportRect) const noexcept
  {
    FloatSize offset;

    if (HasAnchorEdge(AnchorEdge::Left) || !HasAnchorEdge(AnchorEdge::Right))
    {
      offset.SetWidth(viewportRect.X() - _viewportRectAtLastLayout.X());
    }
    else
    {
      offset.SetWidth(viewportRect.MaxX() - _viewportRectAtLastLayout.MaxX());
    }

    if (HasAnchorEdge(AnchorEdge::Top) || !HasAnchorEdge(AnchorEdge::Bottom))
    {
      offset.SetHeight(viewportRect.Y() - _viewportRectAtLastLayout.Y());
    }
    else
    {
      offset.SetHeight(viewportRect.MaxY() - _viewportRectAtLastLayout.MaxY());
    }

    return _layerPositionAtLastLayout + offset;
  }

  FloatSize
    StickyPositionViewportConstraints::ComputeStickyOffset(const FloatRect &constrainingRect) const noexcept
  {
    FloatRect boxRect = _stickyBoxRect;

    if (HasAnchorEdge(AnchorEdge::Right))
    {
      float rightLimit = constrainingRect.MaxX() - _rightOffset;
      float rightDelta = std::min<float>(0, rightLimit - _stickyBoxRect.MaxX());
      float availableSpace = std::min<float>(0, _containingBlockRect.X() - _stickyBoxRect.X());
      if (rightDelta < availableSpace)
      {
        rightDelta = availableSpace;
      }

      boxRect.Move(rightDelta, 0);
    }

    if (HasAnchorEdge(AnchorEdge::Left))
    {
      float leftLimit = constrainingRect.X() + _leftOffset;
      float leftDelta = std::max<float>(0, leftLimit - _stickyBoxRect.X());
      float availableSpace = std::max<float>(0, _containingBlockRect.MaxX() - _stickyBoxRect.MaxX());
      if (leftDelta > availableSpace)
      {
        leftDelta = availableSpace;
      }

      boxRect.Move(leftDelta, 0);
    }

    if (HasAnchorEdge(AnchorEdge::Bottom))
    {
      float bottomLimit = constrainingRect.MaxY() - _bottomOffset;
      float bottomDelta = std::min<float>(0, bottomLimit - _stickyBoxRect.MaxY());
      float availableSpace = std::min<float>(0, _containingBlockRect.Y() - _stickyBoxRect.Y());
      if (bottomDelta < availableSpace)
      {
        bottomDelta = availableSpace;
      }

      boxRect.Move(0, bottomDelta);
    }

    if (HasAnchorEdge(AnchorEdge::Top))
    {
      float topLimit = constrainingRect.Y() + _topOffset;
      float topDelta = std::max<float>(0, topLimit - _stickyBoxRect.Y());
      float availableSpace = std::max<float>(0, _containingBlockRect.MaxY() - _stickyBoxRect.MaxY());
      if (topDelta > availableSpace)
      {
        topDelta = availableSpace;
      }

      boxRect.Move(0, topDelta);
    }

    return boxRect.Location() - _stickyBoxRect.Location();
  }

  FloatPoint StickyPositionViewportConstraints::AnchorLayerPositionForConstrainingRect(
    const FloatRect &constrainingRect) const noexcept
  {
    FloatSize offset = ComputeStickyOffset(constrainingRect);
    return AnchorLayerPositionAtLastLayout() + offset - _stickyOffsetAtLastLayout;
  }

  FloatPoint StickyPositionViewportConstraints::AnchorLayerPositionAtLastLayout() const noexcept
  {
    return _layerPositionAtLastLayout + _anchorLayerOffsetAtLastLayout;
  }

  FloatRect StickyPositionViewportConstraints::ComputeStickyExtent() const noexcept
  {
    float minShiftX = 0;
    float maxShiftX = 0;
    float minShiftY = 0;
    float maxShiftY = 0;

    if (HasAnchorEdge(AnchorEdge::Right))
    {
      minShiftX = std::min<float>(0, _containingBlockRect.X() - _stickyBoxRect.X());
    }

    if (HasAnchorEdge(AnchorEdge::Left))
    {
      maxShiftX = std::max<float>(0, _containingBlockRect.MaxX() - _stickyBoxRect.MaxX());
    }

    if (HasAnchorEdge(AnchorEdge::Bottom))
    {
      minShiftY = std::min<float>(0, _containingBlockRect.Y() - _stickyBoxRect.Y());
    }

    if (HasAnchorEdge(AnchorEdge::Top))
    {
      maxShiftY = std::max<float>(0, _containingBlockRect.MaxY() - _stickyBoxRect.MaxY());
    }

    float minX = _stickyBoxRect.X() + minShiftX;
    float minY = _stickyBoxRect.Y() + minShiftY;
    float maxX = _stickyBoxRect.MaxX() + maxShiftX;
    float maxY = _stickyBoxRect.MaxY() + maxShiftY;

    return FloatRect(minX, minY, maxX - minX, maxY - minY);
  }
}
