/*
 * Copyright (C) 2006-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Web/Html/Shadow/SliderThumbElement.hpp"
#include "Krystal.Core/Geometry/Quad.hpp"
#include "Krystal.Web/Css/Selectors/UserAgentParts.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Internals/TreeTraversal.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Events/Touch/Touch.hpp"
#include "Krystal.Web/Events/Touch/TouchEvent.hpp"
#include "Krystal.Web/Events/Touch/TouchList.hpp"
#include "Krystal.Web/Html/Elements/HTMLDataListElement.hpp"
#include "Krystal.Web/Html/Elements/HTMLInputElement.hpp"
#include "Krystal.Web/Html/InputTypes/StepRange.hpp"
#include "Krystal.Web/Html/Parser/ParserIdioms.hpp"
#include "Krystal.Web/Input/EventHandler.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Rendering/FormControls/RenderSlider.hpp"
#include "Krystal.Web/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Web/Rendering/RenderTheme.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Web/Style/ResolvedStyle.hpp"

namespace krys::web::html
{
  namespace
  {
    KRYS_NODISCARD Decimal SliderPosition(HTMLInputElement &element) noexcept
    {
      const StepRange stepRange(element.CreateStepRange(AnyStepHandling::Reject));
      const Decimal oldValue = ParseToDecimalForNumberType(element.Value(), stepRange.DefaultValue());
      return stepRange.ProportionFromValue(stepRange.ClampValue(oldValue));
    }

    KRYS_NODISCARD bool HasVerticalAppearance(HTMLInputElement &input) noexcept
    {
      krys_debug_assert(input.Renderer());
      return !input.Renderer()->IsHorizontalWritingMode()
             || input.Renderer()->Style().UsedAppearance() == StyleAppearance::SliderVertical;
    }

    // FIXME: Find a way to cascade appearance and adjust heights, and get rid of this class.
    class RenderSliderContainer final : public RenderFlexibleBox
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSliderContainer);

    public:
      RenderSliderContainer(SliderContainerElement &element, RenderStyle &&style) noexcept
          : RenderFlexibleBox(Type::SliderContainer, element, krys::move(style))
      {
      }

      KRYS_NODISCARD LogicalExtentComputedValues
        ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override
      {
        krys_debug_assert(dom::TreeTraversal::ShadowHost(*GetElement()));
        auto &input = Downcast<HTMLInputElement>(*dom::TreeTraversal::ShadowHost(*GetElement()));
        bool isVertical = HasVerticalAppearance(input);

        // A horizontal slider with ticks is tall enough for them.
        if (auto *slider = DynamicDowncast<RenderSlider>(input.Renderer()); slider && !isVertical)
        {
          if (auto trackHeight = slider->TickedTrackHeight())
          {
            return RenderBox::ComputeLogicalHeight(*trackHeight, logicalTop);
          }
        }

        if (isVertical && GetWritingMode().IsHorizontal())
        {
          logicalHeight = LayoutUnit(RenderSlider::DefaultTrackLength);
        }
        return RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
      }

    private:
      void Layout() noexcept override
      {
        krys_debug_assert(dom::TreeTraversal::ShadowHost(*GetElement()));
        auto &input = Downcast<HTMLInputElement>(*dom::TreeTraversal::ShadowHost(*GetElement()));
        bool isVertical = HasVerticalAppearance(input);

        auto &mutableStyle = MutableStyle();
        mutableStyle.SetFlexDirection(isVertical && GetWritingMode().IsHorizontal() ? FlexDirection::Column
                                                                                    : FlexDirection::Row);
        auto oldTextDirection = GetWritingMode().ComputedTextDirection();
        if (isVertical)
        {
          // FIXME: Work around rounding issues in RTL vertical sliders. We want them to
          // render identically to LTR vertical sliders. We can remove this work around when
          // subpixel rendering is enabled on all ports.
          mutableStyle.SetDirection(text::TextDirection::LTR);
        }

        auto *thumb = input.SliderThumbElement() ? input.SliderThumbElement()->GetRenderBox() : nullptr;
        auto *track = input.SliderTrackElement() ? input.SliderTrackElement()->GetRenderBox() : nullptr;
        // Force a layout to reset the position of the thumb so the code below doesn't move the thumb to the
        // wrong place.
        // FIXME: Make a custom Render class for the track and move the thumb positioning code there.
        if (track)
        {
          track->SetChildNeedsLayout(MarkingBehavior::MarkOnlyThis);
        }

        RenderFlexibleBox::Layout();

        mutableStyle.SetDirection(oldTextDirection);
        // These should always exist, unless someone mutates the shadow DOM.
        if (!thumb || !track)
        {
          return;
        }

        double percentageOffset = SliderPosition(input).ToDouble();
        LayoutUnit availableExtent = isVertical ? track->ContentBoxHeight() : track->ContentBoxWidth();
        availableExtent -= isVertical ? thumb->Height() : thumb->Width();
        LayoutUnit offset {percentageOffset * availableExtent};
        LayoutPoint thumbLocation = thumb->Location();
        if (isVertical)
        {
          // appearance: slider-vertical in horizontal writing mode.
          if (GetWritingMode().IsHorizontal())
          {
            thumbLocation.SetY(thumbLocation.Y() + track->ContentBoxHeight() - thumb->Height() - offset);
          }
          else if (GetWritingMode().IsInlineTopToBottom())
          {
            thumbLocation.SetY(thumbLocation.Y() + offset);
          }
          else
          {
            thumbLocation.SetY(thumbLocation.Y() - offset);
          }
        }
        else if (GetWritingMode().IsInlineLeftToRight())
        {
          thumbLocation.SetX(thumbLocation.X() + offset);
        }
        else
        {
          thumbLocation.SetX(thumbLocation.X() - offset);
        }
        thumb->SetLocation(thumbLocation);
        track->Repaint();
        thumb->Repaint();
      }

      KRYS_NODISCARD bool IsFlexibleBoxImpl() const noexcept override
      {
        return true;
      }
    };
  }

#pragma region SliderThumbElement

  SliderThumbElement::SliderThumbElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SliderThumbElement> SliderThumbElement::Create(Document &document) noexcept
  {
    auto element = CreateRef<SliderThumbElement>(document);
    element->SetUserAgentPart(css::user_agent_parts::WebkitSliderThumb());
    return element;
  }

  void SliderThumbElement::SetPositionFromValue() noexcept
  {
    // Since the code to calculate position is in the RenderSliderThumb layout
    // path, we don't actually update the value here. Instead, we poke at the
    // renderer directly to trigger layout.
    if (auto *renderer = Renderer())
    {
      renderer->SetNeedsLayout();
    }
  }

  bool SliderThumbElement::IsDisabledFormControl() const noexcept
  {
    auto *input = HostInput();
    return !input || input->IsDisabledFormControl();
  }

  bool SliderThumbElement::MatchesReadWritePseudoClass() const noexcept
  {
    auto *input = HostInput();
    return input && input->MatchesReadWritePseudoClass();
  }

  void SliderThumbElement::DragFrom(const LayoutPoint &point) noexcept
  {
    Ref protectedThis = ShareRef(*this);
    SetPositionFromPoint(point);
    StartDragging();
  }

  void SliderThumbElement::SetPositionFromPoint(const LayoutPoint &absolutePoint) noexcept
  {
    RefPtr input = ShareRefPtr(HostInput());
    if (!input)
    {
      return;
    }

    auto *inputRenderer = input->GetRenderBox();
    if (!inputRenderer)
    {
      return;
    }

    auto *thumbRenderer = GetRenderBox();
    if (!thumbRenderer)
    {
      return;
    }

    krys_debug_assert(input->SliderTrackElement());
    auto *trackRenderer = input->SliderTrackElement()->GetRenderBox();
    if (!trackRenderer)
    {
      return;
    }

    // Do all the tracking math relative to the input's renderer's box.

    bool isVertical = HasVerticalAppearance(*input);
    bool isInlineFlipped = thumbRenderer->GetWritingMode().IsInlineFlipped()
                           || (isVertical && thumbRenderer->GetWritingMode().IsHorizontal());

    auto offset = inputRenderer->AbsoluteToLocal(PointF(absolutePoint), MapCoordinatesMode::UseTransforms);
    auto trackBoundingBox =
      trackRenderer->LocalToContainerQuad(QuadF(RectF({}, SizeF(trackRenderer->Size()))), inputRenderer)
        .EnclosingBoundingBox();

    LayoutUnit trackLength;
    LayoutUnit position;
    if (isVertical)
    {
      trackLength = trackRenderer->ContentBoxHeight() - thumbRenderer->Height();
      position = LayoutUnit(offset.Y()) - thumbRenderer->Height() / 2 - trackBoundingBox.Y();
      position -= !isInlineFlipped ? thumbRenderer->MarginTop() : thumbRenderer->MarginBottom();
    }
    else
    {
      trackLength = trackRenderer->ContentBoxWidth() - thumbRenderer->Width();
      position = LayoutUnit(offset.X()) - thumbRenderer->Width() / 2 - trackBoundingBox.X();
      position -= !isInlineFlipped ? thumbRenderer->MarginLeft() : thumbRenderer->MarginRight();
    }

    position = std::max<LayoutUnit>(0_lu, std::min(position, trackLength));
    auto ratio = Decimal::FromDouble(static_cast<double>(position) / trackLength.ToDouble());
    auto fraction = isInlineFlipped ? Decimal(1) - ratio : ratio;
    auto stepRange = input->CreateStepRange(AnyStepHandling::Reject);
    auto value = stepRange.ClampValue(stepRange.ValueFromProportion(fraction));

    // Near a tick, the thumb snaps to it.
    const LayoutUnit snappingThreshold = RenderTheme::Singleton().SliderTickSnappingThreshold();
    if (snappingThreshold > 0)
    {
      if (auto closest = input->FindClosestTickMarkValue(value))
      {
        double closestFraction = stepRange.ProportionFromValue(*closest).ToDouble();
        double closestRatio = isInlineFlipped ? 1.0 - closestFraction : closestFraction;
        LayoutUnit closestPosition(trackLength.ToDouble() * closestRatio);
        if ((closestPosition - position).Abs() <= snappingThreshold)
        {
          value = *closest;
        }
      }
    }

    auto valueString = SerializeForNumberType(value);
    if (valueString == input->Value())
    {
      return;
    }

    // FIXME: This is no longer being set from renderer. Consider updating the method name.
    input->SetValueFromRenderer(valueString);
    if (auto *renderer = Renderer())
    {
      // FIXME: The position of repaint rects is incorrect for non-horizontal
      // block flipped writing modes during layout. Repaint beforehand to
      // avoid this.
      auto writingMode = renderer->GetWritingMode();
      if (writingMode.IsBlockFlipped() && !writingMode.IsHorizontal())
      {
        inputRenderer->Repaint();
      }
      renderer->SetNeedsLayout();
    }
  }

  void SliderThumbElement::StartDragging() noexcept
  {
    if (auto *frame = NodeDocument().Frame())
    {
      frame->GetEventHandler().SetCapturingMouseEventsElement(ShareRefPtr(this));
      _inDragMode = true;
    }
  }

  void SliderThumbElement::WillDetachRenderers() noexcept
  {
    if (_inDragMode)
    {
      if (auto *frame = NodeDocument().Frame())
      {
        frame->GetEventHandler().SetCapturingMouseEventsElement(nullptr);
      }
    }
  }

  void SliderThumbElement::StopDragging() noexcept
  {
    if (!_inDragMode)
    {
      return;
    }

    if (auto *frame = NodeDocument().Frame())
    {
      frame->GetEventHandler().SetCapturingMouseEventsElement(nullptr);
    }
    _inDragMode = false;
    if (auto *renderer = Renderer())
    {
      renderer->SetNeedsLayout();
    }
  }

  void SliderThumbElement::DefaultEventHandler(Event &event) noexcept
  {
    auto *mouseEvent = DynamicDowncast<MouseEvent>(event);
    if (!mouseEvent)
    {
      HTMLDivElement::DefaultEventHandler(event);
      return;
    }

    // FIXME: Should handle this readonly/disabled check in more general way.
    // Missing this kind of check is likely to occur elsewhere if adding it in each shadow element.
    RefPtr input = ShareRefPtr(HostInput());
    if (!input || input->IsDisabledFormControl())
    {
      HTMLDivElement::DefaultEventHandler(event);
      return;
    }

    bool isLeftButton = mouseEvent->ButtonType() == MouseButton::Left;
    auto eventType = mouseEvent->Type();

    // We intentionally do not call event->SetDefaultHandled() here because other handlers want to see these
    // mouse events.
    if (eventType == event_names::MouseDown && isLeftButton)
    {
      StartDragging();
      return;
    }
    if (eventType == event_names::MouseUp && isLeftButton)
    {
      input->DispatchFormControlChangeEvent();
      StopDragging();
      return;
    }
    if (eventType == event_names::MouseMove)
    {
      if (_inDragMode)
      {
        auto location = mouseEvent->AbsoluteLocation();
        SetPositionFromPoint(
          LayoutPoint(PointF(static_cast<float>(location.X()), static_cast<float>(location.Y()))));
      }
      return;
    }

    HTMLDivElement::DefaultEventHandler(*mouseEvent);
  }

  void SliderThumbElement::HostDisabledStateChanged() noexcept
  {
    if (IsDisabledFormControl())
    {
      StopDragging();
    }
  }

  HTMLInputElement *SliderThumbElement::HostInput() const noexcept
  {
    // Only HTMLInputElement creates SliderThumbElement instances as its shadow nodes.
    // So, shadowHost() must be an HTMLInputElement.
    return DynamicDowncast<HTMLInputElement>(dom::TreeTraversal::ShadowHost(*this));
  }

  Maybe<style::UnadjustedStyle>
    SliderThumbElement::ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                                           const RenderStyle *hostStyle) noexcept
  {
    if (!hostStyle)
    {
      return null;
    }

    auto elementStyle = ResolveStyle(resolutionContext);
    switch (hostStyle->UsedAppearance())
    {
      case StyleAppearance::SliderVertical:
        elementStyle.Style->SetUsedAppearance(StyleAppearance::SliderThumbVertical);
        break;
      case StyleAppearance::SliderHorizontal:
        elementStyle.Style->SetUsedAppearance(StyleAppearance::SliderThumbHorizontal);
        break;
      default: break;
    }

    return elementStyle;
  }

#pragma endregion

#pragma region SliderContainerElement

  SliderContainerElement::SliderContainerElement(Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlag::HasCustomStyleResolveCallbacks)
  {
    // The container takes touches, which are only dispatched in documents with listeners for them.
    document.AddListenerTypeIfNeeded(event_names::TouchStart);
  }

  void SliderContainerElement::OnAdopt(Document &oldDocument) noexcept
  {
    HTMLDivElement::OnAdopt(oldDocument);
    NodeDocument().AddListenerTypeIfNeeded(event_names::TouchStart);
  }

  void SliderContainerElement::DefaultEventHandler(Event &event) noexcept
  {
    if (auto *touchEvent = DynamicDowncast<TouchEvent>(event))
    {
      HandleTouchEvent(*touchEvent);
      if (touchEvent->DefaultHandled())
      {
        return;
      }
    }
    HTMLDivElement::DefaultEventHandler(event);
  }

  void SliderContainerElement::HandleTouchEvent(TouchEvent &event) noexcept
  {
    RefPtr input = ShareRefPtr(DynamicDowncast<HTMLInputElement>(dom::TreeTraversal::ShadowHost(*this)));
    if (!input || input->IsDisabledFormControl())
    {
      return;
    }

    auto type = event.Type();
    if (type == event_names::TouchEnd)
    {
      // The value the touch slid to is committed as it lifts.
      if (std::exchange(_touchStarted, false))
      {
        input->DispatchFormControlChangeEvent();
        event.SetDefaultHandled();
      }
      _slidingDirection = SlidingDirection::NoMove;
      return;
    }
    if (type == event_names::TouchCancel)
    {
      _touchStarted = false;
      _slidingDirection = SlidingDirection::NoMove;
      return;
    }

    // The touch already went across the slider, so it's the page's.
    if (!CanSlide())
    {
      return;
    }

    RefPtr thumb = ShareRefPtr(DynamicDowncast<SliderThumbElement>(input->SliderThumbElement()));
    auto *touches = event.TargetTouches();
    if (!thumb || !touches || touches->Length() != 1)
    {
      return;
    }

    auto point = touches->Item(0)->AbsoluteLocation();
    auto layoutPoint = LayoutPoint(PointF(static_cast<float>(point.X()), static_cast<float>(point.Y())));
    if (type == event_names::TouchStart)
    {
      _touchStartPoint = point;
      _slidingDirection = SlidingDirection::NoMove;
      _touchStarted = true;
      thumb->SetPositionFromPoint(layoutPoint);
      return;
    }
    if (type != event_names::TouchMove || !_touchStarted)
    {
      return;
    }

    // The touch's first move decides its direction.
    if (_slidingDirection == SlidingDirection::NoMove && point != _touchStartPoint)
    {
      _slidingDirection =
        std::abs(point.X() - _touchStartPoint.X()) >= std::abs(point.Y() - _touchStartPoint.Y())
          ? SlidingDirection::Horizontal
          : SlidingDirection::Vertical;
    }
    if (CanSlide())
    {
      thumb->SetPositionFromPoint(layoutPoint);
      // Handled, so the host doesn't pan with it.
      event.SetDefaultHandled();
    }
  }

  bool SliderContainerElement::CanSlide() const noexcept
  {
    auto *input = DynamicDowncast<HTMLInputElement>(dom::TreeTraversal::ShadowHost(*this));
    if (!input || !input->Renderer())
    {
      return false;
    }
    if (_slidingDirection == SlidingDirection::NoMove)
    {
      return true;
    }
    // A transformed slider may lie in any direction: it takes moves in all of them.
    if (input->Renderer()->Style().HasTransformRelatedProperty())
    {
      return true;
    }
    bool isVertical = HasVerticalAppearance(*input);
    return (_slidingDirection == SlidingDirection::Vertical) == isVertical;
  }

  Ref<SliderContainerElement> SliderContainerElement::Create(Document &document) noexcept
  {
    auto element = CreateRef<SliderContainerElement>(document);
    element->SetUserAgentPart(css::user_agent_parts::WebkitSliderContainer());
    return element;
  }

  RenderPtr<RenderElement> SliderContainerElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSliderContainer>(*this, krys::move(style));
  }

#pragma endregion
}
