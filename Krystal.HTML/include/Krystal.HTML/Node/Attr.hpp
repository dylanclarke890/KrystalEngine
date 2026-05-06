#pragma once

#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/QualifiedName.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;
  class ElementAttributeAlgorithms;

  class Attr : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Attr);

    friend class ElementAttributeAlgorithms;

  private:
    QualifiedName _name;
    WeakPtr<Element> _ownerElement;
    DOMString _value;

  public:
    Attr(Document &document, const QualifiedName &qualifiedName, DOMString &&value) noexcept;

#pragma region Attr

    KRYS_NODISCARD DOMStringAtom NamespaceURI() const noexcept
    {
      return _name.NamespaceURI;
    }

    KRYS_NODISCARD DOMStringAtom Prefix() const noexcept
    {
      return _name.Prefix;
    }

    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _name.LocalName;
    }

    KRYS_NODISCARD DOMString Name() const noexcept
    {
      return _name.Name();
    }

    KRYS_NODISCARD const DOMString &Value() const noexcept
    {
      return _value;
    }

    void Value(DOMString &&value) noexcept
    {
      SetExistingAttributeValue(*this, Krys::Move(value));
    }

    KRYS_NODISCARD RawPtr<Element> OwnerElement() const noexcept
    {
      return _ownerElement.get();
    }

    KRYS_NODISCARD bool Specified() const noexcept
    {
      return true;
    }

#pragma endregion

#pragma region Node

    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return Name();
    }

    KRYS_NODISCARD Maybe<DOMString> NodeValue() const noexcept final
    {
      return Value();
    }

    ExceptionOr<void> NodeValue(DOMString &&value) noexcept final
    {
      SetExistingAttributeValue(*this, Krys::Move(value));
      return {};
    }

    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final
    {
      return Value();
    }

    ExceptionOr<void> TextContent(DOMString &&value) noexcept final
    {
      SetExistingAttributeValue(*this, Krys::Move(value));
      return {};
    }

#pragma endregion

  private:
    /// @see https://dom.spec.whatwg.org/#set-an-existing-attribute-value
    static void SetExistingAttributeValue(Attr &attribute, DOMString &&value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Attr)
  static bool IsType(const Krys::HTML::Node &node)
  {
    return node.IsAttributeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();