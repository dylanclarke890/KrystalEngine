#pragma once

#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Dicts/MouseEventInit.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @see https://html.spec.whatwg.org/multipage/dnd.html#drageventinit
  struct DragEventInit : MouseEventInit
  {
    RefPtr<dom::DataTransfer> DataTransfer;
  };
}
