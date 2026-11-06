#include "Krystal.Booey/Platform/Scrolling/ScrollAnimator.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollableArea.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    // As long as Chrome's: the square root of the distance, in frames at 60 per second, up to a second and a
    // half.
    constexpr double FramesPerSecond = 60;
    constexpr double MaxAnimationDurationInSeconds = 1.5;

    KRYS_NODISCARD float ComponentForAxis(const FloatPoint &point, ScrollEventAxis axis) noexcept
    {
      return axis == ScrollEventAxis::Horizontal ? point.X() : point.Y();
    }

    KRYS_NODISCARD double Interpolate(double progress, float from, float to) noexcept
    {
      return from + progress * (to - from);
    }
  }

  ScrollAnimator::ScrollAnimator(ScrollableArea &scrollableArea) noexcept : _scrollableArea(scrollableArea)
  {
  }

  bool ScrollAnimator::ScrollToOffsetWithAnimation(const FloatPoint &destinationOffset,
                                                   ScrollType scrollType) noexcept
  {
    FloatPoint minimum(_scrollableArea.MinimumScrollOffset());
    FloatPoint maximum(_scrollableArea.MaximumScrollOffset());
    auto destination = destinationOffset.ConstrainedBetween(minimum, maximum);

    // A running animation is retargeted, already moving, so it only eases out. A new one has the curve that
    // fits Chrome's to within 2 pixels over 1000.
    if (_isAnimating)
    {
      _timingFunction = RefPtr<TimingFunction>(
        CubicBezierTimingFunction::Create(CubicBezierTimingFunction::TimingFunctionPreset::EaseOut));
    }
    else
    {
      _currentOffset = FloatPoint(_scrollableArea.GetScrollOffset());
      _timingFunction = RefPtr<TimingFunction>(CubicBezierTimingFunction::Create(0.4, 0, 0, 1));
    }

    _startOffset = _currentOffset;
    _destinationOffset = destination;
    double distance =
      std::hypot(_destinationOffset.X() - _startOffset.X(), _destinationOffset.Y() - _startOffset.Y());
    if (!distance)
    {
      _isAnimating = false;
      return false;
    }

    _durationInSeconds = std::min(std::sqrt(distance) / FramesPerSecond, MaxAnimationDurationInSeconds);
    _startTime = null;
    _scrollType = scrollType;
    _isAnimating = true;
    return true;
  }

  bool ScrollAnimator::ServiceScrollAnimation(MonotonicTime::duration now) noexcept
  {
    if (!_isAnimating)
    {
      return false;
    }
    if (!_startTime)
    {
      _startTime = now;
      return true;
    }

    // What it can reach may have changed since it started.
    _destinationOffset = _destinationOffset.ConstrainedBetween(
      FloatPoint(_scrollableArea.MinimumScrollOffset()), FloatPoint(_scrollableArea.MaximumScrollOffset()));

    double elapsed = std::chrono::duration<double>(now - *_startTime).count();
    bool finished = elapsed >= _durationInSeconds;
    double progress =
      finished ? 1 : _timingFunction->TransformProgress(elapsed / _durationInSeconds, _durationInSeconds);
    _currentOffset =
      FloatPoint(static_cast<float>(Interpolate(progress, _startOffset.X(), _destinationOffset.X())),
                 static_cast<float>(Interpolate(progress, _startOffset.Y(), _destinationOffset.Y())));

    auto oldScrollType = _scrollableArea.CurrentScrollType();
    _scrollableArea.SetCurrentScrollType(_scrollType);
    _scrollableArea.NotifyScrollPositionChanged(
      _scrollableArea.ScrollPositionFromOffset(RoundedIntPoint(_currentOffset)));
    _scrollableArea.SetCurrentScrollType(oldScrollType);

    if (finished)
    {
      _isAnimating = false;
      _scrollableArea.DidStopAnimatedScroll();
      return false;
    }
    return true;
  }

  void ScrollAnimator::ClearSnapOffsetsInfo() noexcept
  {
    _snapOffsetsInfo = null;
    _horizontalSnapTargets.clear();
    _verticalSnapTargets.clear();
  }

  FloatPoint
    ScrollAnimator::ScrollOffsetAdjustedForSnapping(const FloatPoint &offset,
                                                    ScrollSnapPointSelectionMethod method) const noexcept
  {
    if (!_snapOffsetsInfo || method == ScrollSnapPointSelectionMethod::None)
    {
      return offset;
    }

    auto forAxis = [&](ScrollEventAxis axis)
    {
      if (method != ScrollSnapPointSelectionMethod::Directional)
      {
        return AdjustedScrollDestination(axis, offset, 0);
      }
      float current = ComponentForAxis(FloatPoint(_scrollableArea.GetScrollOffset()), axis);
      return AdjustedScrollDestination(axis, offset, ComponentForAxis(offset, axis) - current, current);
    };
    return FloatPoint(forAxis(ScrollEventAxis::Horizontal), forAxis(ScrollEventAxis::Vertical));
  }

  float ScrollAnimator::AdjustedScrollDestination(ScrollEventAxis axis, const FloatPoint &destination,
                                                  float velocity, Maybe<float> originalOffset) const noexcept
  {
    if (!_snapOffsetsInfo)
    {
      return ComponentForAxis(destination, axis);
    }

    Maybe<LayoutUnit> original;
    if (originalOffset)
    {
      original = LayoutUnit(*originalOffset);
    }
    auto [target, index] = _snapOffsetsInfo->ClosestSnapOffset(
      axis, LayoutSize(_scrollableArea.VisibleSize()),
      LayoutPoint(LayoutUnit(destination.X()), LayoutUnit(destination.Y())), velocity, original);
    float minimum = ComponentForAxis(FloatPoint(_scrollableArea.MinimumScrollOffset()), axis);
    float maximum = ComponentForAxis(FloatPoint(_scrollableArea.MaximumScrollOffset()), axis);
    return std::clamp(static_cast<float>(target), minimum, std::max(minimum, maximum));
  }

  void ScrollAnimator::UpdateSnapTargets() noexcept
  {
    if (!_snapOffsetsInfo)
    {
      return;
    }

    auto offset = _scrollableArea.GetScrollOffset();
    auto targetsForAxis = [&](ScrollEventAxis axis, List<WeakPtr<dom::Element>> &targets)
    {
      targets.clear();
      const auto &offsets = _snapOffsetsInfo->OffsetsForAxis(axis);
      if (offsets.empty())
      {
        return;
      }
      auto [target, index] = _snapOffsetsInfo->ClosestSnapOffset(
        axis, LayoutSize(_scrollableArea.VisibleSize()), LayoutPoint(offset), 0);
      if (index)
      {
        for (auto areaIndex : offsets[*index].SnapAreaIndices)
        {
          targets.push_back(_snapOffsetsInfo->SnapAreaTargets[areaIndex]);
        }
      }
    };
    targetsForAxis(ScrollEventAxis::Horizontal, _horizontalSnapTargets);
    targetsForAxis(ScrollEventAxis::Vertical, _verticalSnapTargets);
  }

  void ScrollAnimator::ResnapAfterLayout() noexcept
  {
    if (!_snapOffsetsInfo || _isAnimating || _scrollableArea.IsTouchScrollInProgress())
    {
      return;
    }

    auto current = _scrollableArea.GetScrollOffset();
    auto correctedForAxis = [&](ScrollEventAxis axis, const List<WeakPtr<dom::Element>> &targets,
                                const Scrollbar *scrollbar) -> int32
    {
      int32 value = axis == ScrollEventAxis::Horizontal ? current.X() : current.Y();
      const auto &offsets = _snapOffsetsInfo->OffsetsForAxis(axis);
      if (offsets.empty() || (scrollbar && scrollbar->PressedPart() != NoPart))
      {
        return value;
      }

      // The first offset of an element it was snapped to, one that's focused or targeted if there's one of
      // those.
      const SnapOffset *found = nullptr;
      auto consider = [&](const dom::Element *element)
      {
        for (const auto &offset : offsets)
        {
          for (auto areaIndex : offset.SnapAreaIndices)
          {
            if (_snapOffsetsInfo->SnapAreaTargets[areaIndex].get() == element)
            {
              found = &offset;
              return;
            }
          }
        }
      };
      for (const auto &target : targets)
      {
        auto *element = target.get();
        if (!element)
        {
          continue;
        }
        auto &document = element->NodeDocument();
        if (document.FocusedElement() == element || document.CSSTarget() == element)
        {
          consider(element);
          if (found)
          {
            break;
          }
        }
      }
      for (const auto &target : targets)
      {
        if (found)
        {
          break;
        }
        if (auto *element = target.get())
        {
          consider(element);
        }
      }

      if (found)
      {
        return static_cast<int32>(found->Offset);
      }
      return static_cast<int32>(std::lround(AdjustedScrollDestination(axis, FloatPoint(current), 0)));
    };

    ScrollOffset corrected(
      correctedForAxis(ScrollEventAxis::Horizontal, _horizontalSnapTargets,
                       _scrollableArea.HorizontalScrollbar()),
      correctedForAxis(ScrollEventAxis::Vertical, _verticalSnapTargets, _scrollableArea.VerticalScrollbar()));
    if (corrected != current)
    {
      auto oldScrollType = _scrollableArea.CurrentScrollType();
      _scrollableArea.SetCurrentScrollType(ScrollType::Programmatic);
      _scrollableArea.ScrollToOffsetWithoutAnimation(corrected);
      _scrollableArea.SetCurrentScrollType(oldScrollType);
    }
    UpdateSnapTargets();
  }
}
