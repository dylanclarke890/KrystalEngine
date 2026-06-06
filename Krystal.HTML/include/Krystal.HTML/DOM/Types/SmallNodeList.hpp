#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class Node;
  class Element;
  class Text;

  /// @brief Covers 99.5%. See webkit.org/b/80706
  constexpr inline size_t OptimalSmallNodeListSize = 11;

  using SmallNodeList = SmallList<Ref<Node>, OptimalSmallNodeListSize>;
  using SmallElementList = SmallList<Ref<Element>, OptimalSmallNodeListSize>;

  // TODO(check): OptimalSmallNodeListSize is for the generic node case, not text nodes specifically. Might need to tweak
  // this constant.
  using SmallTextNodeList = SmallList<Ref<Text>, OptimalSmallNodeListSize>;
  using SmallConstTextNodeList = SmallList<Ref<const Text>, OptimalSmallNodeListSize>;
}