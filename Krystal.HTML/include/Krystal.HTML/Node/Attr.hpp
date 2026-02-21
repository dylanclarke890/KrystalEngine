#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Element;

  class Attr : public Node
  {
  private:
    QualifiedName _name;
    CheckedPtr<Element> _ownerElement;

  public:
    KRYS_NODISCARD DOMString NodeName() const noexcept override;
    KRYS_NODISCARD RawPtr<Element> OwnerElement() const noexcept
    {
      return _ownerElement.get();
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Attr)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsAttributeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()