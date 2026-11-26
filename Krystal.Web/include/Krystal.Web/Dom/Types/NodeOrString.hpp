#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"

namespace krys::web
{
  class Node;
}

namespace krys::web::dom
{
  using NodeOrString = Variant<Ref<Node>, DOMString>;
}
