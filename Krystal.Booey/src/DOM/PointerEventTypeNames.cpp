#include "Krystal.Booey/DOM/PointerEventTypeNames.hpp"

namespace krys::boo::dom
{
  const DOMString &MousePointerEventType() noexcept
  {
    static const DOMString mouseType(u8"mouse");
    return mouseType;
  }

  const DOMString &PenPointerEventType() noexcept
  {
    static const DOMString penType(u8"pen");
    return penType;
  }

  const DOMString &TouchPointerEventType() noexcept
  {
    static const DOMString touchType(u8"touch");
    return touchType;
  }
}
