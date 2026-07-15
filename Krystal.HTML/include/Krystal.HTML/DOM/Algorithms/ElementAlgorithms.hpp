#pragma once

#include "Krystal.HTML/DOM/DOMTokenList.hpp"
#include "Krystal.HTML/DOM/Enums/InsertAdjacentWhere.hpp"
#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/UniquePtr.hpp"

namespace Krys::HTML
{
  class ElementAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-element-defined
    KRYS_NODISCARD static bool IsDefined(const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-custom
    KRYS_NODISCARD static bool IsCustom(const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#insert-adjacent
    KRYS_NODISCARD static ExceptionOr<RawPtr<Node>>
      InsertAdjacent(Element &element, InsertAdjacentWhere where, Node &node) noexcept;

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

    /// @see https://dom.spec.whatwg.org/#concept-attach-a-shadow-root
    KRYS_NODISCARD static ExceptionOr<void> AttachShadowRoot(Element &element, ShadowRootMode mode,
                                                             Clonable clonable, Serializable serializable,
                                                             DelegatesFocus delegatesFocus,
                                                             SlotAssignmentMode slotAssignment,
                                                             RawPtr<CustomElementRegistry> registry) noexcept;

    /// @brief Helper method for creating a new DOMTokenList bound to an element and attribute, optionally
    /// with a function to filter new values to only supported values.
    KRYS_NODISCARD static UniquePtr<DOMTokenList> CreateDOMTokenList(
      Element &element, DOMStringAtom attributeName,
      IsSupportedTokenFunction &&isSupportedToken = DefaultIsSupportedTokenFunction) noexcept;
  };
}