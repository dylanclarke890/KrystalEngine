#pragma once

#include "Krystal.HTML/Document/TreeScope.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;

  class DocumentFragment : public ContainerNode, public TreeScope
  {
  public:
    DocumentFragment(Document &document, NodeFlags flags = NodeFlags::None) noexcept;

    KRYS_NODISCARD DOMString NodeName() const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::DocumentFragment)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsDocumentFragmentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()