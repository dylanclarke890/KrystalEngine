#pragma once

#include "Krystal.HTML/DOM/Types/QualifiedName.hpp"
#include "Krystal.HTML/DOM/Node.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"

namespace Krys::HTML
{
  class Document;
  class Element;

  /// @see https://dom.spec.whatwg.org/#interface-attr
  class Attr : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Attr);

    friend class Document;
    friend class Element;
    friend class ElementAlgorithms;
    friend class NodeAlgorithms;

  private:
    QualifiedName _name;
    WeakPtr<Element> _ownerElement;
    DOMString _value;

  protected:
    Attr(Document &document, const QualifiedName &qualifiedName, DOMString &&value = {}) noexcept;

  public:
#pragma region Attr - https://dom.spec.whatwg.org/#attr

    /// @see https://dom.spec.whatwg.org/#dom-attr-namespaceuri
    KRYS_NODISCARD DOMStringAtom NamespaceURI() const noexcept
    {
      return _name.NamespaceURI();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-prefix
    KRYS_NODISCARD DOMStringAtom Prefix() const noexcept
    {
      return _name.NamespacePrefix();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-localname
    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _name.LocalName();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-name
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      return _name.Name();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-value
    KRYS_NODISCARD const DOMString &Value() const noexcept
    {
      return _value;
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-value
    void Value(DOMString &&value) noexcept
    {
      SetExistingAttributeValue(*this, Krys::Move(value));
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-ownerelement
    KRYS_NODISCARD RawPtr<Element> OwnerElement() const noexcept
    {
      return _ownerElement.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-specified
    KRYS_NODISCARD bool Specified() const noexcept
    {
      return true;
    }

#pragma endregion

#pragma region Node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return Name();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    KRYS_NODISCARD Maybe<DOMString> NodeValue() const noexcept final
    {
      return Value();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-nodevalue
    ExceptionOr<void> NodeValue(DOMString &&value) noexcept final
    {
      SetExistingAttributeValue(*this, Krys::Move(value));
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final
    {
      return Value();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
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