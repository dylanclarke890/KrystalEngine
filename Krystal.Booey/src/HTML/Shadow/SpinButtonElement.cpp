#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/CSS/Selectors/UserAgentParts.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/Page/EventHandler.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderTheme.hpp"

namespace krys::boo::html
{
  SpinButtonElement::SpinButtonElement(dom::Document &document, SpinButtonOwner &spinButtonOwner) noexcept
      : HTMLDivElement(document, dom::NodeFlags::HasCustomStyleResolveCallbacks),
        _spinButtonOwner(&spinButtonOwner), _repeatingTimer(*this, &SpinButtonElement::RepeatingTimerFired)
  {
  }

  Ref<SpinButtonElement> SpinButtonElement::Create(dom::Document &document,
                                                   SpinButtonOwner &spinButtonOwner) noexcept
  {
    auto element = CreateRef<SpinButtonElement>(document, spinButtonOwner);
    element->SetUserAgentPart(css::UserAgentParts::WebkitInnerSpinButton());
    return element;
  }

  bool SpinButtonElement::IsDisabledFormControl() const noexcept
  {
    auto *host = dom::TreeQueries::ShadowHost(*this);
    return host && host->IsDisabledFormControl();
  }

  void SpinButtonElement::DefaultEventHandler(dom::Event &event) noexcept
  {
    auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(event);
    if (!mouseEvent)
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    auto *box = GetRenderBox();
    if (!box)
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    if (!ShouldRespondToMouseEvents())
    {
      if (!event.DefaultHandled())
      {
        HTMLDivElement::DefaultEventHandler(event);
      }
      return;
    }

    auto absoluteLocation = mouseEvent->AbsoluteLocation();
    IntPoint local = RoundedIntPoint(box->AbsoluteToLocal(
      FloatPoint(static_cast<float>(absoluteLocation.X()), static_cast<float>(absoluteLocation.Y())),
      MapCoordinatesMode::UseTransforms));
    auto type = mouseEvent->Type();
    if (type == EventNames::MouseDown && mouseEvent->ButtonType() == MouseButton::Left)
    {
      if (box->BorderBoxRect().Contains(local))
      {
        Ref protectedThis = ShareRef(*this);
        if (auto *owner = _spinButtonOwner)
        {
          owner->FocusAndSelectSpinButtonOwner();
        }
        if (Renderer())
        {
          if (_upDownState != UpDownState::Indeterminate)
          {
            // An event handler called in DoStepAction() below might change the element state and we might
            // need to cancel the repeating timer by the state change. If we started the timer after
            // DoStepAction(), we would have no chance to cancel the timer.
            StartRepeatingTimer();
            DoStepAction(_upDownState == UpDownState::Up ? 1 : -1);
          }
        }
        mouseEvent->SetDefaultHandled();
      }
    }
    else if (type == EventNames::MouseUp && mouseEvent->ButtonType() == MouseButton::Left)
    {
      StopRepeatingTimer();
    }
    else if (type == EventNames::MouseMove)
    {
      if (box->BorderBoxRect().Contains(local))
      {
        if (!_capturing)
        {
          if (auto *frame = NodeDocument().Frame())
          {
            frame->GetEventHandler().SetCapturingMouseEventsElement(ShareRefPtr(this));
            _capturing = true;
            // FIXME: Registering as a popup opening observer (Chrome::registerPopupOpeningObserver), with
            // popups (G14d3).
          }
        }
        auto oldUpDownState = _upDownState;
        auto *renderer = Renderer();
        switch (RenderTheme::Singleton().GetInnerSpinButtonLayout(*renderer))
        {
          case RenderTheme::InnerSpinButtonLayout::Vertical:
            _upDownState = local.Y() < box->Height() / 2 ? UpDownState::Up : UpDownState::Down;
            break;
          case RenderTheme::InnerSpinButtonLayout::HorizontalUpLeft:
            _upDownState = local.X() < box->Width() / 2 ? UpDownState::Up : UpDownState::Down;
            break;
          case RenderTheme::InnerSpinButtonLayout::HorizontalUpRight:
            _upDownState = local.X() > box->Width() / 2 ? UpDownState::Up : UpDownState::Down;
            break;
        }
        if (_upDownState != oldUpDownState)
        {
          renderer->Repaint();
        }
      }
      else
      {
        ReleaseCapture();
        _upDownState = UpDownState::Indeterminate;
      }
    }

    if (!mouseEvent->DefaultHandled())
    {
      HTMLDivElement::DefaultEventHandler(*mouseEvent);
    }
  }

  void SpinButtonElement::DoStepAction(int32 amount) noexcept
  {
    auto *owner = _spinButtonOwner;
    if (!owner)
    {
      return;
    }

    if (amount > 0)
    {
      owner->SpinButtonStepUp();
    }
    else if (amount < 0)
    {
      owner->SpinButtonStepDown();
    }
  }

  void SpinButtonElement::ReleaseCapture() noexcept
  {
    StopRepeatingTimer();
    if (_capturing)
    {
      if (auto *frame = NodeDocument().Frame())
      {
        frame->GetEventHandler().SetCapturingMouseEventsElement(nullptr);
        _capturing = false;
      }
    }
  }

  bool SpinButtonElement::MatchesReadWritePseudoClass() const noexcept
  {
    return dom::TreeQueries::ShadowHost(*this)->MatchesReadWritePseudoClass();
  }

  void SpinButtonElement::StartRepeatingTimer() noexcept
  {
    _pressStartingState = _upDownState;
    auto &theme = ScrollbarTheme::Theme();
    _repeatingTimer.Start(theme.InitialAutoscrollTimerDelay(), theme.AutoscrollTimerDelay());
  }

  void SpinButtonElement::StopRepeatingTimer() noexcept
  {
    _repeatingTimer.Stop();
  }

  void SpinButtonElement::Step(int32 amount) noexcept
  {
    if (!ShouldRespondToMouseEvents())
    {
      return;
    }

    // On Mac OS, NSStepper updates the value for the button under the mouse
    // cursor regardless of the button pressed at the beginning.
    if (_upDownState != _pressStartingState)
    {
      return;
    }

    DoStepAction(amount);
  }

  void SpinButtonElement::RepeatingTimerFired() noexcept
  {
    if (_upDownState != UpDownState::Indeterminate)
    {
      Step(_upDownState == UpDownState::Up ? 1 : -1);
    }
  }

  void SpinButtonElement::SetHovered(bool flag, style::InvalidationScope invalidationScope,
                                     HitTestRequest request) noexcept
  {
    if (!flag)
    {
      _upDownState = UpDownState::Indeterminate;
    }
    HTMLDivElement::SetHovered(flag, invalidationScope, request);
  }

  bool SpinButtonElement::ShouldRespondToMouseEvents() const noexcept
  {
    auto *owner = _spinButtonOwner;
    return !owner || owner->ShouldSpinButtonRespondToMouseEvents();
  }
}
