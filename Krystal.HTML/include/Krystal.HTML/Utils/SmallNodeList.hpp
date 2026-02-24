#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class Node;

  /// @brief Covers 99.5%. See webkit.org/b/80706
  constexpr inline size_t OptimalSmallNodeListSize = 11;

  using SmallNodeList = SmallList<Ref<Node>, OptimalSmallNodeListSize>;
}