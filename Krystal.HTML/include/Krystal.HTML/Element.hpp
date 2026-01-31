#pragma once

#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Element : public ContainerNode
  {
  public:
    KRYS_NODISCARD utf8_string NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
