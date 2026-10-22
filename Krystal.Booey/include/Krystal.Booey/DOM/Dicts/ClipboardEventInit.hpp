#pragma once

#include "Krystal.Booey/DOM/Dicts/EventInit.hpp"

namespace krys::boo::dom
{
  /// @see https://w3c.github.io/clipboard-apis/#dictdef-clipboardeventinit
  // FIXME: The clipboard data (clipboardData), with DataTransfer.
  struct ClipboardEventInit : EventInit
  {
  };
}
