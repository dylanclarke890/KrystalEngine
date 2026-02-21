#pragma once

#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Attr;

  class Element : public ContainerNode
  {
  protected:
    Element(Document &document, NodeFlag nodeFlags = NodeFlag::None) noexcept;

  public:
    KRYS_NODISCARD DOMString NodeName() const noexcept final;
    KRYS_NODISCARD ExceptionOr<void> RemoveAttributeNode(Attr &attribute) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Element)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
