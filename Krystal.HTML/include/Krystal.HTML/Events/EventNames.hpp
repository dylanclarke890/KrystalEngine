#pragma once

#include "Krystal.HTML/DOMString.hpp"

namespace Krys::HTML::EventNames
{
  inline static DOMStringAtom Click = u8"click";
  inline static DOMStringAtom TouchStart = u8"touchstart";
  inline static DOMStringAtom TouchMove = u8"touchmove";
  inline static DOMStringAtom Wheel = u8"wheel";
  inline static DOMStringAtom MouseWheel = u8"mousewheel";
  inline static DOMStringAtom BeforeUnload = u8"beforeunload";
  inline static DOMStringAtom AnimationEnd = u8"animationend";
  inline static DOMStringAtom AnimationIteration = u8"animationiteration";
  inline static DOMStringAtom AnimationStart = u8"animationstart";
  inline static DOMStringAtom TransitionEnd = u8"transitionend";

  namespace Legacy
  {
    inline static DOMStringAtom AnimationEnd = u8"webkitAnimationEnd";
    inline static DOMStringAtom AnimationIteration = u8"webkitAnimationIteration";
    inline static DOMStringAtom AnimationStart = u8"webkitAnimationStart";
    inline static DOMStringAtom TransitionEnd = u8"webkitTransitionEnd";
  }
}