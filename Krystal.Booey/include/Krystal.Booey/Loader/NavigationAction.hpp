#pragma once

#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Loader/FrameLoadRequest.hpp"
#include "Krystal.Booey/Page/MouseEventTypes.hpp"
#include "Krystal.Booey/Page/NavigationNavigationType.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Event;
  class MouseEvent;
  class UIEventWithKeyState;
}

namespace krys::boo
{
  /// @brief A navigation as the host is asked about it: the URL, what started it (a link, a form) and the
  /// input event that did.
  // FIXME: User gestures, back and forward items, the requester's origin and the Navigation API's type, once
  // there's history and scripting.
  class NavigationAction : public FrameLoadRequestBase
  {
  public:
    /// @brief The modifier keys of the event that started a navigation.
    struct UIEventWithKeyStateData
    {
      bool IsTrusted {false};
      bool ShiftKey {false};
      bool CtrlKey {false};
      bool AltKey {false};
      bool MetaKey {false};

      explicit UIEventWithKeyStateData(const dom::UIEventWithKeyState &uiEvent) noexcept;
    };

    /// @brief The mouse event that started a navigation.
    struct MouseEventData : UIEventWithKeyStateData
    {
      /// @brief Where it happened, in the frame's absolute coordinates.
      DoublePoint AbsoluteLocation;
      MouseButton Button {MouseButton::None};
      bool ButtonDown {false};

      explicit MouseEventData(const dom::MouseEvent &mouseEvent) noexcept;
    };

  private:
    ResourceRequest _originalRequest;
    Maybe<UIEventWithKeyStateData> _keyStateEventData;
    Maybe<MouseEventData> _mouseEventData;
    NavigationType _type {NavigationType::Other};
    Maybe<NavigationNavigationType> _navigationAPIType;

  public:
    NavigationAction() noexcept;

    /// @brief The Navigation API's type of the navigation, if it's one the API reports.
    KRYS_NODISCARD Maybe<NavigationNavigationType> NavigationAPIType() const noexcept
    {
      return _navigationAPIType;
    }
    void SetNavigationAPIType(Maybe<NavigationNavigationType> navigationAPIType) noexcept
    {
      _navigationAPIType = navigationAPIType;
    }
    NavigationAction(const ResourceRequest &originalRequest, FrameLoadType frameLoadType,
                     bool isFormSubmission, dom::Event *event = nullptr,
                     dom::DOMStringAtom downloadAttribute = dom::DOMStringAtom::Null(),
                     dom::Element *sourceElement = nullptr) noexcept;
    NavigationAction(FrameLoadRequest &request, NavigationType type = NavigationType::Other,
                     dom::Event *event = nullptr) noexcept;
    ~NavigationAction() noexcept;
    NavigationAction(const NavigationAction &) noexcept;
    NavigationAction &operator=(const NavigationAction &) noexcept;
    NavigationAction(NavigationAction &&) noexcept;
    NavigationAction &operator=(NavigationAction &&) noexcept;

    /// @brief The modifier keys of the first event with them in the chain of events that started the
    /// navigation (a simulated click's underlying key press), if any.
    KRYS_NODISCARD const Maybe<UIEventWithKeyStateData> &KeyStateEventData() const noexcept
    {
      return _keyStateEventData;
    }

    /// @brief The first mouse event in the chain of events that started the navigation, if any.
    KRYS_NODISCARD const Maybe<MouseEventData> &GetMouseEventData() const noexcept
    {
      return _mouseEventData;
    }

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _originalRequest.URL().IsEmpty();
    }

    KRYS_NODISCARD const url::URL &URL() const noexcept
    {
      return _originalRequest.URL();
    }

    KRYS_NODISCARD const ResourceRequest &OriginalRequest() const noexcept
    {
      return _originalRequest;
    }

    KRYS_NODISCARD NavigationType Type() const noexcept
    {
      return _type;
    }
  };
}
