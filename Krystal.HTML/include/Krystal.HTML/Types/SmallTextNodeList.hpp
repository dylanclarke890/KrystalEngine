#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  class Text;

  // TODO(check): the below is for the generic node case, not text nodes specifically. Might need to tweak
  // this constant.
  /// @brief Covers 99.5%. See webkit.org/b/80706
  constexpr inline size_t OptimalSmallTextNodeListSize = 11uz;

  using SmallTextNodeList = SmallList<Ref<Text>, OptimalSmallTextNodeListSize>;
  using SmallConstTextNodeList = SmallList<Ref<const Text>, OptimalSmallTextNodeListSize>;
}