/*
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Booey/Rendering/FormControls/RenderProgress.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLProgressElement.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  RenderProgress::RenderProgress(html::HTMLElement &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::Progress, element, krys::move(style)),
        _position(html::HTMLProgressElement::InvalidPosition),
        _animationTimer(*this, &RenderProgress::AnimationTimerFired)
  {
    krys_debug_assert(IsRenderProgress());
  }

  RenderProgress::~RenderProgress() noexcept = default;

  void RenderProgress::WillBeDestroyed() noexcept
  {
    _animationTimer.Stop();
    RenderBlockFlow::WillBeDestroyed();
  }

  void RenderProgress::UpdateFromElement() noexcept
  {
    auto *element = ProgressElement();
    if (_position == element->Position())
    {
      return;
    }
    _position = element->Position();

    UpdateAnimationState();
    Repaint();
    RenderBlockFlow::UpdateFromElement();
  }

  RenderBox::LogicalExtentComputedValues
    RenderProgress::ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept
  {
    auto computedValues = RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
    LayoutRect frame = FrameRect();
    if (IsHorizontalWritingMode())
    {
      frame.SetHeight(computedValues.Extent);
    }
    else
    {
      frame.SetWidth(computedValues.Extent);
    }
    IntSize frameSize =
      RenderTheme::Singleton().ProgressBarRectForBounds(*this, SnappedIntRect(frame)).Size();
    computedValues.Extent = LayoutUnit(IsHorizontalWritingMode() ? frameSize.Height() : frameSize.Width());
    return computedValues;
  }

  double RenderProgress::AnimationProgress() const noexcept
  {
    using Seconds = std::chrono::duration<double>;
    auto duration =
      std::chrono::duration_cast<Seconds>(RenderTheme::Singleton().AnimationDurationForProgressBar());
    krys_debug_assert(duration.count() > 0);
    if (!_animating)
    {
      return 0;
    }
    auto elapsed = std::chrono::duration_cast<Seconds>(MonotonicTime::Now() - _animationStartTime);
    return std::fmod(elapsed.count(), duration.count()) / duration.count();
  }

  bool RenderProgress::IsDeterminate() const noexcept
  {
    return (html::HTMLProgressElement::IndeterminatePosition != Position()
            && html::HTMLProgressElement::InvalidPosition != Position());
  }

  void RenderProgress::AnimationTimerFired() noexcept
  {
    // FIXME: Ideally obtaining the repeat interval from Page is not RenderTheme-specific, but it
    // currently is as it also determines whether we animate at all.
    auto repeatInterval = RenderTheme::Singleton().AnimationRepeatIntervalForProgressBar(*this);
    Repaint();
    if (!_animationTimer.IsActive() && _animating)
    {
      _animationTimer.StartOneShot(repeatInterval);
    }
  }

  void RenderProgress::UpdateAnimationState() noexcept
  {
    auto repeatInterval = RenderTheme::Singleton().AnimationRepeatIntervalForProgressBar(*this);

    bool animating =
      Style().HasUsedAppearance() && repeatInterval > MonotonicTime::duration::zero() && !IsDeterminate();
    if (animating == _animating)
    {
      return;
    }

    _animating = animating;
    if (_animating)
    {
      _animationStartTime = MonotonicTime::Now();
      _animationTimer.StartOneShot(repeatInterval);
    }
    else
    {
      _animationTimer.Stop();
    }
  }

  html::HTMLProgressElement *RenderProgress::ProgressElement() const noexcept
  {
    auto *element = GetElement();
    if (!element)
    {
      return nullptr;
    }

    if (auto *progressElement = DynamicDowncast<html::HTMLProgressElement>(*element))
    {
      return progressElement;
    }

    krys_debug_assert(dom::TreeQueries::ShadowHost(*element));
    return Downcast<html::HTMLProgressElement>(dom::TreeQueries::ShadowHost(*element));
  }
}
