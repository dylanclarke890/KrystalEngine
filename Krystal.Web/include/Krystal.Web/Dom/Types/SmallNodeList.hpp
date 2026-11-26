#pragma once

#include "Krystal.Core/Containers/SmallList.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::web
{
  class Element;
  class Node;
  class Text;
}

namespace krys::web::dom
{
  /// @brief Covers 99.5%. See webkit.org/b/80706
  constexpr inline size_t OptimalSmallNodeListSize = 11uz;

  /// @note Might need to tweak this
  constexpr inline size_t OptimalSmallElementListSize = 11uz;
  
  /// @note Might need to tweak this
  constexpr inline size_t OptimalSmallTextNodeListSize = 4uz;

  using SmallNodeList = SmallList<Ref<Node>, OptimalSmallNodeListSize>;

  using SmallElementList = SmallList<Ref<Element>, OptimalSmallElementListSize>;
  
  using SmallTextNodeList = SmallList<Ref<Text>, OptimalSmallTextNodeListSize>;
  
  using SmallConstTextNodeList = SmallList<Ref<const Text>, OptimalSmallTextNodeListSize>;
}
