#pragma once

#include "Krystal.HTML/ContainerNode.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class DocumentFragment : public ContainerNode
  {
  public:
    KRYS_NODISCARD utf8_string NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::DocumentFragment)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsDocumentFragmentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()