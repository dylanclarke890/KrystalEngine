#include "Krystal.Booey/Page/Screen.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo
{
  namespace
  {
    /// @brief The part of the screen windows can use: the whole screen unless the host says otherwise.
    KRYS_NODISCARD FloatRect ScreenAvailableRect(const LocalFrame &frame) noexcept
    {
      auto &environment = frame.GetMediaEnvironment();
      if (!environment.ScreenAvailableRect.IsEmpty())
      {
        return environment.ScreenAvailableRect;
      }
      return FloatRect(FloatPoint(), environment.ScreenSize);
    }
  }

  Screen::Screen(LocalDOMWindow &window) noexcept : _window(CreateWeakPtr(&window))
  {
  }

  LocalFrame *Screen::Frame() const noexcept
  {
    auto *window = _window.get();
    return window ? window->Frame() : nullptr;
  }

  int32 Screen::AvailWidth() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(ScreenAvailableRect(*frame).Width()) : 0;
  }

  int32 Screen::AvailHeight() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(ScreenAvailableRect(*frame).Height()) : 0;
  }

  int32 Screen::AvailLeft() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(ScreenAvailableRect(*frame).X()) : 0;
  }

  int32 Screen::AvailTop() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(ScreenAvailableRect(*frame).Y()) : 0;
  }

  int32 Screen::Width() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(frame->GetMediaEnvironment().ScreenSize.Width()) : 0;
  }

  int32 Screen::Height() const noexcept
  {
    auto *frame = Frame();
    return frame ? static_cast<int32>(frame->GetMediaEnvironment().ScreenSize.Height()) : 0;
  }

  uint32 Screen::ColorDepth() const noexcept
  {
    auto *frame = Frame();
    if (!frame)
    {
      return 24;
    }
    return static_cast<uint32>(frame->GetMediaEnvironment().ScreenDepthPerComponent) * 3;
  }
}
