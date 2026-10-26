#pragma once

#include "Krystal.Booey/DOM/DataTransfer.hpp"
#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/clipboard-apis/#dictdef-clipboardeventinit
  struct ClipboardEventInit : EventInit
  {
    RefPtr<DataTransfer> ClipboardData;
  };
}
