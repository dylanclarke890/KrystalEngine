#pragma once

#include "Krystal.HTML/DOMString.hpp"
#include "Krystal.HTML/Node/Enums/InsertAdjacentWhere.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  class Attr;
  class CustomElementRegistry;
  class Document;
  class Element;
  class Node;

  using CustomElementRegistryOrDefault = Maybe<RawPtr<CustomElementRegistry>>;
  constexpr inline auto DefaultCustomElementRegistry = Null;

  class ElementAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#insert-adjacent
    KRYS_NODISCARD static ExceptionOr<RawPtr<Node>>
      InsertAdjacent(Element &element, InsertAdjacentWhere where, Node &node) noexcept;

    static Ref<Element>
      CreateElement(Document &document, DOMStringAtom localName, DOMStringAtom namespaceURI,
                    DOMStringAtom prefix = DOMStringAtom::Null(), DOMStringAtom is = DOMStringAtom::Null(),
                    bool synchronousCustomElements = false,
                    CustomElementRegistryOrDefault registry = DefaultCustomElementRegistry) noexcept;

    /// @see https://dom.spec.whatwg.org/#handle-attribute-changes
    static void HandleAttributeChanges(Attr &attribute, Element &element, DOMStringView oldValue,
                                       DOMStringView newValue) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change
    static void ChangeAttribute(Attr &attribute, DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-append
    static void AppendAttribute(Attr &attribute, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove
    static void RemoveAttribute(Attr &attribute) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-replace
    static void ReplaceAttribute(Attr &oldAttribute, Attr &newAttribute) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-by-name
    KRYS_NODISCARD static RawPtr<Attr> GetAttributeByName(DOMStringAtom qualifiedName,
                                                          const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-by-namespace
    KRYS_NODISCARD static RawPtr<Attr> GetAttributeByNamespace(DOMStringAtom namespaceURI,
                                                               DOMStringAtom localName,
                                                               const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-value
    KRYS_NODISCARD static DOMString
      GetAttributeValue(const Element &element, DOMStringAtom localName,
                        DOMStringAtom namespaceURI = DOMStringAtom::Null()) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-set
    static ExceptionOr<RefPtr<Attr>> SetAttribute(Attr &attr, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-set-value
    static void SetAttributeValue(Element &element, DOMStringAtom localName, DOMString &&value,
                                  DOMStringAtom prefix = DOMStringAtom::Null(),
                                  DOMStringAtom namespaceURI = DOMStringAtom::Null()) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-name
    static RefPtr<Attr> RemoveAttributeByName(DOMStringAtom qualifiedName, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-namespace
    static RefPtr<Attr> RemoveAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                   Element &element) noexcept;
  };
}