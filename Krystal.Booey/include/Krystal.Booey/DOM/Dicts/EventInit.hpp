#pragma once

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-eventinit
  struct EventInit
  {
    bool Bubbles {false};
    bool Cancelable {false};
    bool Composed {false};
  };
}