#pragma once

#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo::dom
{
  class Document;
  class Element;

  /// @brief An element's attribute: a qualified name and a value.
  /// @see https://dom.spec.whatwg.org/#interface-attr
  /// @see https://dom.spec.whatwg.org/#concept-attribute
  class Attr : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Attr);

    friend class Document;
    friend class Element;
    friend class ElementAlgorithms;
    friend class NodeAlgorithms;

  private:
    /// @brief The attribute's namespace, namespace prefix and local name.
    /// @see https://dom.spec.whatwg.org/#concept-attribute-qualified-name
    /// @see https://dom.spec.whatwg.org/#concept-attribute-namespace
    /// @see https://dom.spec.whatwg.org/#concept-attribute-namespace-prefix
    /// @see https://dom.spec.whatwg.org/#concept-attribute-local-name
    QualifiedName _name;
    /// @see https://dom.spec.whatwg.org/#concept-attribute-element
    WeakPtr<Element> _ownerElement;
    /// @see https://dom.spec.whatwg.org/#concept-attribute-value
    DOMString _value;

  protected:
    /// @brief Creates an attribute with the qualified name and value, and no element.
    /// @see https://dom.spec.whatwg.org/#create-an-attribute
    Attr(Document &document, const QualifiedName &qualifiedName, DOMString value = {}) noexcept;

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

    /// @brief The attribute's namespace, namespace prefix and local name.
    KRYS_NODISCARD const QualifiedName &GetQualifiedName() const noexcept
    {
      return _name;
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-localname
    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _name.LocalName();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-name
    /// @see https://dom.spec.whatwg.org/#concept-named-attribute
    KRYS_NODISCARD DOMString Name() const noexcept
    {
      return _name.Name();
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-value
    KRYS_NODISCARD const DOMString &Value() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-attr-value
    void Value(DOMString value) noexcept
    {
      html::CEReactionsScope ceReactions;
      SetExistingAttributeValue(*this, krys::move(value));
    }

    /// @see https://dom.spec.whatwg.org/#dom-attr-ownerelement
    KRYS_NODISCARD Element *OwnerElement() const noexcept
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
    ExceptionOr<void> NodeValue(DOMString value) noexcept final
    {
      html::CEReactionsScope ceReactions;
      SetExistingAttributeValue(*this, krys::move(value));
      return {};
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final
    {
      return Value();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    ExceptionOr<void> TextContent(DOMString value) noexcept final
    {
      html::CEReactionsScope ceReactions;
      SetExistingAttributeValue(*this, krys::move(value));
      return {};
    }

#pragma endregion

  private:
    /// @see https://dom.spec.whatwg.org/#set-an-existing-attribute-value
    static void SetExistingAttributeValue(Attr &attribute, DOMString value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Attr)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsAttributeNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();