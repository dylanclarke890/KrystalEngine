/*
 * Copyright (C) 2019 Igalia S.L.
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

#include "Krystal.Booey/Page/ResizeObserver/ResizeObservation.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include <cmath>

namespace krys::boo
{
  Ref<ResizeObservation> ResizeObservation::Create(dom::Element &target,
                                                   ResizeObserverBoxOptions observedBox) noexcept
  {
    return AdoptRef(*new ResizeObservation(target, observedBox));
  }

  ResizeObservation::ResizeObservation(dom::Element &target, ResizeObserverBoxOptions observedBox) noexcept
      : _target(CreateWeakPtr(&target)),
        _lastObservationSizes {LayoutSize(-1, -1), LayoutSize(-1, -1), LayoutSize(-1, -1), FloatSize(-1, -1)},
        _observedBox(observedBox)
  {
  }

  ResizeObservation::~ResizeObservation() noexcept = default;

  void ResizeObservation::UpdateObservationSize(const BoxSizes &boxSizes) noexcept
  {
    _lastObservationSizes = boxSizes;
  }

  void ResizeObservation::ResetObservationSize() noexcept
  {
    _lastObservationSizes = {LayoutSize(-1, -1), LayoutSize(-1, -1), LayoutSize(-1, -1), FloatSize(-1, -1)};
  }

  namespace
  {
    /// @brief @p size in whole device pixels.
    KRYS_NODISCARD FloatSize ResizeObserverDevicePixels(FloatSize size, float deviceScaleFactor) noexcept
    {
      return FloatSize(std::round(size.Width() * deviceScaleFactor),
                       std::round(size.Height() * deviceScaleFactor));
    }
  }

  Maybe<ResizeObservation::BoxSizes> ResizeObservation::ComputeObservedSizes() const noexcept
  {
    if (auto *svg = DynamicDowncast<svg::SVGElement>(*_target))
    {
      if (svg->HasAssociatedSVGLayoutBox())
      {
        LayoutSize size;
        if (auto svgRect = svg->GetBoundingBox())
        {
          size.SetWidth(svgRect->Width());
          size.SetHeight(svgRect->Height());
        }
        return BoxSizes {
          size, size, size,
          ResizeObserverDevicePixels(FloatSize(size), svg->NodeDocument().DeviceScaleFactor())};
      }
    }

    if (auto *box = _target->GetRenderBox())
    {
      if (box->IsSkippedContent())
      {
        return null;
      }
      // The device pixel size is the content box snapped to device pixels where it sits on the page, so it's
      // the size the box actually draws at. Device pixels aren't zoomed CSS pixels, so zoom doesn't apply.
      auto deviceScaleFactor = box->GetDocument().DeviceScaleFactor();
      auto contentBox = box->ContentBoxRect();
      auto absoluteOrigin = box->LocalToAbsolute(FloatPoint(contentBox.Location()));
      auto snapped =
        SnapRectToDevicePixels(LayoutRect(LayoutPoint(absoluteOrigin), contentBox.Size()), deviceScaleFactor);
      auto devicePixelSize = ResizeObserverDevicePixels(snapped.Size(), deviceScaleFactor);
      if (!box->Style().GetWritingMode().IsHorizontal())
      {
        devicePixelSize = devicePixelSize.TransposedSize();
      }

      return BoxSizes {
        AdjustLayoutSizeForAbsoluteZoom(box->ContentBoxSize(), box->Style()),
        AdjustLayoutSizeForAbsoluteZoom(box->ContentBoxLogicalSize(), box->Style()),
        AdjustLayoutSizeForAbsoluteZoom(box->BorderBoxLogicalSize(), box->Style()),
        devicePixelSize,
      };
    }
    return BoxSizes {};
  }

  LayoutPoint ResizeObservation::ComputeTargetLocation() const noexcept
  {
    if (!_target)
    {
      return {};
    }

    if (auto *box = _target->GetRenderBox())
    {
      return LayoutPoint(box->PaddingLeft(), box->PaddingTop());
    }
    return {};
  }

  FloatRect ResizeObservation::ComputeContentRect() const noexcept
  {
    return FloatRect(FloatPoint(ComputeTargetLocation()), FloatSize(_lastObservationSizes.ContentBoxSize));
  }

  FloatSize ResizeObservation::BorderBoxSize() const noexcept
  {
    return FloatSize(_lastObservationSizes.BorderBoxLogicalSize);
  }

  FloatSize ResizeObservation::ContentBoxSize() const noexcept
  {
    return FloatSize(_lastObservationSizes.ContentBoxLogicalSize);
  }

  FloatSize ResizeObservation::DevicePixelContentBoxSize() const noexcept
  {
    return _lastObservationSizes.DevicePixelContentBoxLogicalSize;
  }

  Maybe<ResizeObservation::BoxSizes> ResizeObservation::ElementSizeChanged() const noexcept
  {
    auto currentSizes = ComputeObservedSizes();
    if (!currentSizes)
    {
      return null;
    }

    switch (_observedBox)
    {
      case ResizeObserverBoxOptions::BorderBox:
        if (_lastObservationSizes.BorderBoxLogicalSize != currentSizes->BorderBoxLogicalSize)
        {
          return currentSizes;
        }
        break;
      case ResizeObserverBoxOptions::ContentBox:
        if (_lastObservationSizes.ContentBoxLogicalSize != currentSizes->ContentBoxLogicalSize)
        {
          return currentSizes;
        }
        break;
      case ResizeObserverBoxOptions::DevicePixelContentBox:
        if (_lastObservationSizes.DevicePixelContentBoxLogicalSize
            != currentSizes->DevicePixelContentBoxLogicalSize)
        {
          return currentSizes;
        }
        break;
    }
    return null;
  }

  size_t ResizeObservation::TargetElementDepth() const noexcept
  {
    // There's no frame tree, so the depth counts the target's own document only.
    size_t depth = 0;
    for (dom::Element *parent = _target.get(); parent; parent = parent->ParentElementInComposedTree())
    {
      ++depth;
    }
    return depth;
  }
}
