#pragma once
#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Variant.hpp"

namespace Krys::HTML
{
  class Node;
  using NodeOrString = Variant<Ref<Node>, DOMString>;
}