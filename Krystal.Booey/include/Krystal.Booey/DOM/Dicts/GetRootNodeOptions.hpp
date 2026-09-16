#pragma once

namespace krys::boo::dom
{
  /// @see https://dom.spec.whatwg.org/#dictdef-getrootnodeoptions
  struct GetRootNodeOptions
  {
    bool Composed = false;
  };
}