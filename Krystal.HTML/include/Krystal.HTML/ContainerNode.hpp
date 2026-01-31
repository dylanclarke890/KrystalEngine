#pragma once

#include "Krystal.HTML/Node.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ContainerNode)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()