#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::EventNames
{
  constexpr utf8_stringview ReadyStateChange = u8"readystatechange";
  constexpr utf8_stringview Abort = u8"abort";
  constexpr utf8_stringview Click = u8"click";
  constexpr utf8_stringview TouchStart = u8"touchstart";
  constexpr utf8_stringview TouchMove = u8"touchmove";
  constexpr utf8_stringview Wheel = u8"wheel";
  constexpr utf8_stringview MouseWheel = u8"mousewheel";
  constexpr utf8_stringview BeforeUnload = u8"beforeunload";
  constexpr utf8_stringview AnimationEnd = u8"animationend";
  constexpr utf8_stringview AnimationIteration = u8"animationiteration";
  constexpr utf8_stringview AnimationStart = u8"animationstart";
  constexpr utf8_stringview TransitionEnd = u8"transitionend";

  namespace Legacy
  {
    constexpr utf8_stringview AnimationEnd = u8"webkitAnimationEnd";
    constexpr utf8_stringview AnimationIteration = u8"webkitAnimationIteration";
    constexpr utf8_stringview AnimationStart = u8"webkitAnimationStart";
    constexpr utf8_stringview TransitionEnd = u8"webkitTransitionEnd";
  }
}