#pragma once

namespace Krys
{
  /// @brief A tag type which can hold arbitrarily many types. Useful as an anchor for overload resolution on
  /// specific entities, extension point anchors, and more. Not related to the tag_invoke infrastructure.
  template <typename...>
  class Tag
  {
  };
}