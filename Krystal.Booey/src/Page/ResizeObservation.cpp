#include "Krystal.Booey/Page/ResizeObservation.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"

namespace krys::boo
{
  Ref<ResizeObservation> ResizeObservation::Create(dom::Element &target,
                                                   ResizeObserverBoxOptions observedBox) noexcept
  {
    return AdoptRef(*new ResizeObservation(target, observedBox));
  }

  ResizeObservation::ResizeObservation(dom::Element &target, ResizeObserverBoxOptions observedBox) noexcept
      : _target(CreateWeakPtr(&target)),
        _lastObservationSizes {LayoutSize(-1, -1), LayoutSize(-1, -1), LayoutSize(-1, -1)},
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
    _lastObservationSizes = {LayoutSize(-1, -1), LayoutSize(-1, -1), LayoutSize(-1, -1)};
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
        return BoxSizes {size, size, size};
      }
    }

    if (auto *box = _target->GetRenderBox())
    {
      if (box->IsSkippedContent())
      {
        return null;
      }
      return BoxSizes {
        AdjustLayoutSizeForAbsoluteZoom(box->ContentBoxSize(), box->Style()),
        AdjustLayoutSizeForAbsoluteZoom(box->ContentBoxLogicalSize(), box->Style()),
        AdjustLayoutSizeForAbsoluteZoom(box->BorderBoxLogicalSize(), box->Style()),
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

  FloatSize ResizeObservation::SnappedContentBoxSize() const noexcept
  {
    return FloatSize(_lastObservationSizes.ContentBoxLogicalSize); // FIXME: Need to pixel snap.
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
