#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class Element;
  class Text;
}

namespace krys::boo::dom
{
  using ElementOrText = Variant<Ref<Element>, Ref<Text>>;
}
