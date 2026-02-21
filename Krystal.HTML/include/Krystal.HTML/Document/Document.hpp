#pragma once

#include "Krystal.HTML/Document/TreeScope.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document : public ContainerNode, public TreeScope
  {
  public:
    Document() noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<Node>> AdoptNode(Node &node) noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Document)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
