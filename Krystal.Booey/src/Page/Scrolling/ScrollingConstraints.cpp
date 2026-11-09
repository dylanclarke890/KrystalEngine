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
