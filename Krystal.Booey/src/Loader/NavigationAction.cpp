#include "Krystal.Booey/Loader/NavigationAction.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"

namespace krys::boo
{
  NavigationAction::UIEventWithKeyStateData::UIEventWithKeyStateData(
    const dom::UIEventWithKeyState &uiEvent) noexcept
      : IsTrusted(uiEvent.IsTrusted()), ShiftKey(uiEvent.ShiftKey()), CtrlKey(uiEvent.CtrlKey()),
        AltKey(uiEvent.AltKey()), MetaKey(uiEvent.MetaKey())
  {
  }

  NavigationAction::MouseEventData::MouseEventData(const dom::MouseEvent &mouseEvent) noexcept
      : UIEventWithKeyStateData(mouseEvent), AbsoluteLocation(mouseEvent.AbsoluteLocation()),
        Button(mouseEvent.ButtonType()), ButtonDown(mouseEvent.ButtonDown())
  {
  }

  namespace
  {
    KRYS_NODISCARD Maybe<NavigationAction::UIEventWithKeyStateData>
      KeyStateDataForFirstEventWithKeyState(dom::Event *event) noexcept
    {
      if (auto *uiEvent = dom::FindEventWithKeyState(event))
      {
        return NavigationAction::UIEventWithKeyStateData(*uiEvent);
      }
      return std::nullopt;
    }

    KRYS_NODISCARD Maybe<NavigationAction::MouseEventData>
      MouseEventDataForFirstMouseEvent(dom::Event *event) noexcept
    {
      for (auto *current = event; current; current = current->UnderlyingEvent())
      {
        if (auto *mouseEvent = DynamicDowncast<dom::MouseEvent>(*current))
        {
          return NavigationAction::MouseEventData(*mouseEvent);
        }
      }
      return std::nullopt;
    }

    KRYS_NODISCARD NavigationType GetNavigationType(FrameLoadType frameLoadType, bool isFormSubmission,
                                                    bool haveEvent) noexcept
    {
      if (isFormSubmission)
      {
        return NavigationType::FormSubmitted;
      }
      if (haveEvent)
      {
        return NavigationType::LinkClicked;
      }
      if (IsReload(frameLoadType))
      {
        return NavigationType::Reload;
      }
      if (IsBackForwardLoadType(frameLoadType))
      {
        return NavigationType::BackForward;
      }
      return NavigationType::Other;
    }
  }

  NavigationAction::NavigationAction() noexcept = default;

  NavigationAction::NavigationAction(const ResourceRequest &originalRequest, FrameLoadType frameLoadType,
                                     bool isFormSubmission, dom::Event *event,
                                     dom::DOMStringAtom downloadAttribute,
                                     dom::Element *sourceElement) noexcept
      : _originalRequest(originalRequest), _keyStateEventData(KeyStateDataForFirstEventWithKeyState(event)),
        _mouseEventData(MouseEventDataForFirstMouseEvent(event)),
        _type(GetNavigationType(frameLoadType, isFormSubmission, event != nullptr))
  {
    SetDownloadAttribute(downloadAttribute);
    SetSourceElement(sourceElement);
  }

  NavigationAction::NavigationAction(FrameLoadRequest &request, NavigationType type,
                                     dom::Event *event) noexcept
      : FrameLoadRequestBase(request), _originalRequest(request.GetResourceRequest()),
        _keyStateEventData(KeyStateDataForFirstEventWithKeyState(event)),
        _mouseEventData(MouseEventDataForFirstMouseEvent(event)), _type(type)
  {
  }

  NavigationAction::~NavigationAction() noexcept = default;
  NavigationAction::NavigationAction(const NavigationAction &) noexcept = default;
  NavigationAction &NavigationAction::operator=(const NavigationAction &) noexcept = default;
  NavigationAction::NavigationAction(NavigationAction &&) noexcept = default;
  NavigationAction &NavigationAction::operator=(NavigationAction &&) noexcept = default;
}
