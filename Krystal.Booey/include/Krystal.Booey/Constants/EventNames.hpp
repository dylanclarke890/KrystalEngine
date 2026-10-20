#pragma once

#include "Krystal.Core/Types/String.hpp"

namespace krys::boo::EventNames
{
  constexpr utf8_stringview ReadyStateChange = u8"readystatechange";
  constexpr utf8_stringview Abort = u8"abort";
  constexpr utf8_stringview Change = u8"change";
  constexpr utf8_stringview Load = u8"load";
  constexpr utf8_stringview Error = u8"error";
  constexpr utf8_stringview Click = u8"click";
  constexpr utf8_stringview AuxClick = u8"auxclick";
  constexpr utf8_stringview DblClick = u8"dblclick";
  constexpr utf8_stringview ContextMenu = u8"contextmenu";
  constexpr utf8_stringview MouseDown = u8"mousedown";
  constexpr utf8_stringview MouseUp = u8"mouseup";
  constexpr utf8_stringview MouseMove = u8"mousemove";
  constexpr utf8_stringview MouseOver = u8"mouseover";
  constexpr utf8_stringview MouseOut = u8"mouseout";
  constexpr utf8_stringview MouseEnter = u8"mouseenter";
  constexpr utf8_stringview MouseLeave = u8"mouseleave";
  constexpr utf8_stringview PointerMove = u8"pointermove";
  constexpr utf8_stringview KeyDown = u8"keydown";
  constexpr utf8_stringview KeyPress = u8"keypress";
  constexpr utf8_stringview KeyUp = u8"keyup";
  constexpr utf8_stringview Scroll = u8"scroll";
  constexpr utf8_stringview ScrollEnd = u8"scrollend";
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