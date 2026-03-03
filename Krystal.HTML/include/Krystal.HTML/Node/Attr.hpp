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
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Attr);

  private:
    QualifiedName _name;
    CheckedPtr<Element> _ownerElement;
    DOMString _value;

  public:
#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    KRYS_NODISCARD DOMString NodeValue() const noexcept final
    {
      return Value();
    }
    ExceptionOr<void> SetNodeValue(DOMString &&value) noexcept final
    {
      return SetExistingAttributeValue(*this, Krys::Move(value));
    }
    KRYS_NODISCARD DOMString TextContent() const noexcept final
    {
      return Value();
    }
    ExceptionOr<void> SetTextContent(DOMString &&value) noexcept final
    {
      return SetExistingAttributeValue(*this, Krys::Move(value));
    }

#pragma endregion

    KRYS_NODISCARD RawPtr<Element> OwnerElement() const noexcept
    {
      return _ownerElement.get();
    }

    KRYS_NODISCARD DOMString Value() const noexcept;

    static ExceptionOr<void> SetExistingAttributeValue(Attr &attribute, DOMString &&value) noexcept
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Attr)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsAttributeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()