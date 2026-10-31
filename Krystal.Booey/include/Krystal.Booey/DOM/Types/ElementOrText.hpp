#pragma once

#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::dom
{
  class Element;
  class Text;
  using ElementOrText = Variant<Ref<Element>, Ref<Text>>;
}
