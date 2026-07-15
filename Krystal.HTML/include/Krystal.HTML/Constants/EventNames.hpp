#pragma once

#include "Krystal.HTML/DOM/Types/DOMString.hpp"

namespace Krys::HTML::EventNames
{
  constexpr inline DOMStringView ReadyStateChange = u8"readystatechange";
  constexpr inline DOMStringView Abort = u8"abort";
  constexpr inline DOMStringView Click = u8"click";
  constexpr inline DOMStringView TouchStart = u8"touchstart";
  constexpr inline DOMStringView TouchMove = u8"touchmove";
  constexpr inline DOMStringView Wheel = u8"wheel";
  constexpr inline DOMStringView MouseWheel = u8"mousewheel";
  constexpr inline DOMStringView BeforeUnload = u8"beforeunload";
  constexpr inline DOMStringView AnimationEnd = u8"animationend";
  constexpr inline DOMStringView AnimationIteration = u8"animationiteration";
  constexpr inline DOMStringView AnimationStart = u8"animationstart";
  constexpr inline DOMStringView TransitionEnd = u8"transitionend";

  namespace Legacy
  {
    constexpr inline DOMStringView AnimationEnd = u8"webkitAnimationEnd";
    constexpr inline DOMStringView AnimationIteration = u8"webkitAnimationIteration";
    constexpr inline DOMStringView AnimationStart = u8"webkitAnimationStart";
    constexpr inline DOMStringView TransitionEnd = u8"webkitTransitionEnd";
  }
}