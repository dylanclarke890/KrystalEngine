#pragma once

#include "Krystal.Lib/String/StringAtom.hpp"

namespace Krys::HTML::EventNames
{
  inline static StringAtom Click = u8"click";
  inline static StringAtom TouchStart = u8"touchstart";
  inline static StringAtom TouchMove = u8"touchmove";
  inline static StringAtom Wheel = u8"wheel";
  inline static StringAtom MouseWheel = u8"mousewheel";
}