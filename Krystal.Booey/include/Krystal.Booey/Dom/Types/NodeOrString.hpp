#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  class Node;
}

namespace krys::boo::dom
{
  using NodeOrString = Variant<Ref<Node>, DOMString>;
}
