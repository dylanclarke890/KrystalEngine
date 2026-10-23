#include "Krystal.Booey/HTML/Shadow/SliderThumbElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/HTML/StepRange.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderSlider.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"

namespace krys::boo::html
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
        krys_debug_assert(dom::TreeQueries::ShadowHost(*GetElement()));
        auto &input = Downcast<HTMLInputElement>(*dom::TreeQueries::ShadowHost(*GetElement()));
        bool isVertical = HasVerticalAppearance(input);

        // FIXME: The track height for a range with a data list's ticks (hasDataList), with datalists.

        if (isVertical && GetWritingMode().IsHorizontal())
        {
          logicalHeight = LayoutUnit(RenderSlider::DefaultTrackLength);
        }
        return RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
      }

    private:
      void Layout() noexcept override
      {
        krys_debug_assert(dom::TreeQueries::ShadowHost(*GetElement()));
        auto &input = Downcast<HTMLInputElement>(*dom::TreeQueries::ShadowHost(*GetElement()));
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
          mutableStyle.SetDirection(TextDirection::LTR);
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

  SliderThumbElement::SliderThumbElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SliderThumbElement> SliderThumbElement::Create(dom::Document &document) noexcept
  {
    auto element = CreateRef<SliderThumbElement>(document);
    element->SetUserAgentPart(css::UserAgentParts::WebkitSliderThumb());
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

    auto offset =
      inputRenderer->AbsoluteToLocal(FloatPoint(absolutePoint), MapCoordinatesMode::UseTransforms);
    auto trackBoundingBox =
      trackRenderer
        ->LocalToContainerQuad(FloatQuad(FloatRect({}, FloatSize(trackRenderer->Size()))), inputRenderer)
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

    // FIXME: Snapping to the nearest tick mark (sliderTickSnappingThreshold, findClosestTickMarkValue), with
    // datalists.

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

  void SliderThumbElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
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
    if (eventType == EventNames::MouseDown && isLeftButton)
    {
      StartDragging();
      return;
    }
    if (eventType == EventNames::MouseUp && isLeftButton)
    {
      input->DispatchFormControlChangeEvent();
      StopDragging();
      return;
    }
    if (eventType == EventNames::MouseMove)
    {
      if (_inDragMode)
      {
        auto location = mouseEvent->AbsoluteLocation();
        SetPositionFromPoint(
          LayoutPoint(FloatPoint(static_cast<float>(location.X()), static_cast<float>(location.Y()))));
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
    return DynamicDowncast<HTMLInputElement>(dom::TreeQueries::ShadowHost(*this));
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

  SliderContainerElement::SliderContainerElement(dom::Document &document) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks)
  {
  }

  Ref<SliderContainerElement> SliderContainerElement::Create(dom::Document &document) noexcept
  {
    auto element = CreateRef<SliderContainerElement>(document);
    element->SetUserAgentPart(css::UserAgentParts::WebkitSliderContainer());
    return element;
  }

  RenderPtr<RenderElement> SliderContainerElement::CreateElementRenderer(RenderStyle &&style) noexcept
  {
    return CreateRenderer<RenderSliderContainer>(*this, krys::move(style));
  }

#pragma endregion
}
