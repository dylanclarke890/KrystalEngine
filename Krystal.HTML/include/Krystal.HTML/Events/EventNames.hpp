#pragma once

#include "Krystal.HTML/DOMString.hpp"

namespace Krys::HTML::EventNames
{
  inline static DOMStringAtom Click = u8"click";
  inline static DOMStringAtom TouchStart = u8"touchstart";
  inline static DOMStringAtom TouchMove = u8"touchmove";
  inline static DOMStringAtom Wheel = u8"wheel";
  inline static DOMStringAtom MouseWheel = u8"mousewheel";
}