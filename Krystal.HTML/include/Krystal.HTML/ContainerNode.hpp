#pragma once

#include "Krystal.HTML/Node.hpp"
#include "Krystal.Lib/Pointers/Ref.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class ContainerNode : public Node
  {
  private:
    List<Ref<Node>> _childNodes;
  };
}