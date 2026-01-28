#pragma once

namespace Krys::HTML
{
  struct EventInit
  {
    bool Bubbles {false};
    bool Cancellable {false};
    bool Composed {false};
  };
}