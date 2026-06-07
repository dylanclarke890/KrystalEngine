#pragma once

#include "Krystal.HTML/Types/DOMString.hpp"

namespace Krys::HTML::EventNames
{
  inline DOMStringAtom ReadyStateChange = u8"readystatechange";
  inline DOMStringAtom Abort = u8"abort";
  inline DOMStringAtom Click = u8"click";
  inline DOMStringAtom TouchStart = u8"touchstart";
  inline DOMStringAtom TouchMove = u8"touchmove";
  inline DOMStringAtom Wheel = u8"wheel";
  inline DOMStringAtom MouseWheel = u8"mousewheel";
  inline DOMStringAtom BeforeUnload = u8"beforeunload";
  inline DOMStringAtom AnimationEnd = u8"animationend";
  inline DOMStringAtom AnimationIteration = u8"animationiteration";
  inline DOMStringAtom AnimationStart = u8"animationstart";
  inline DOMStringAtom TransitionEnd = u8"transitionend";

  namespace Legacy
  {
    inline DOMStringAtom AnimationEnd = u8"webkitAnimationEnd";
    inline DOMStringAtom AnimationIteration = u8"webkitAnimationIteration";
    inline DOMStringAtom AnimationStart = u8"webkitAnimationStart";
    inline DOMStringAtom TransitionEnd = u8"webkitTransitionEnd";
  }
}