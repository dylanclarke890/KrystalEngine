#pragma once

#include "Krystal.Core/Containers/List.hpp"

namespace krys::web::css::mq
{
  struct MediaQuery;

  /// @brief A comma-separated list of media queries, as a `media` attribute or `@media` rule holds.
  using MediaQueryList = List<MediaQuery>;
}
