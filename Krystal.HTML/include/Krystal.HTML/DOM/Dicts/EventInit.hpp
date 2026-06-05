#pragma once

namespace Krys::HTML
{
  /// @see https://dom.spec.whatwg.org/#dictdef-eventinit
  struct EventInit
  {
    bool Bubbles {false};
    bool Cancelable {false};
    bool Composed {false};
  };
}