#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::html
{
  class CustomElementRegistry;
}

namespace krys::boo
{
  class Attr;
  class Element;
  class Node;
  class ShadowRoot;
}

namespace krys::boo::dom
{
  enum class Clonable : bool;
  enum class DelegatesFocus : bool;
  enum class InsertAdjacentWhere : uint8;
  enum class Serialisable : bool;
  enum class ShadowRootMode : uint8;
  enum class SlotAssignmentMode : uint8;

  class ElementAlgorithms
  {
  public:
    /// @see https://dom.spec.whatwg.org/#concept-element-defined
    KRYS_NODISCARD static bool IsDefined(const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-custom
    KRYS_NODISCARD static bool IsCustom(const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#insert-adjacent
    KRYS_NODISCARD static ExceptionOr<Node *> InsertAdjacent(Element &element, InsertAdjacentWhere where,
                                                             Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#handle-attribute-changes
    static void HandleAttributeChanges(Attr &attribute, Element &element, Maybe<DOMStringView> oldValue,
                                       Maybe<DOMStringView> newValue) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change
    static void ChangeAttribute(Attr &attribute, DOMString value) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-append
    static void AppendAttribute(Attr &attribute, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove
    static void RemoveAttribute(Attr &attribute) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-replace
    static void ReplaceAttribute(Attr &oldAttribute, Attr &newAttribute) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-by-name
    KRYS_NODISCARD static Attr *GetAttributeByName(DOMStringAtom qualifiedName,
                                                   const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-by-namespace
    KRYS_NODISCARD static Attr *GetAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                        const Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-get-value
    KRYS_NODISCARD static DOMString GetAttributeValue(const Element &element, DOMStringAtom localName,
                                                      DOMStringAtom namespaceURI = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-set
    static ExceptionOr<RefPtr<Attr>> SetAttribute(Attr &attr, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-set-value
    static void SetAttributeValue(Element &element, const DOMStringAtom &localName, DOMString value,
                                  DOMStringAtom prefix = null,
                                  const DOMStringAtom &namespaceURI = null) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-name
    static RefPtr<Attr> RemoveAttributeByName(DOMStringAtom qualifiedName, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-remove-by-namespace
    static RefPtr<Attr> RemoveAttributeByNamespace(DOMStringAtom namespaceURI, DOMStringAtom localName,
                                                   Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-attach-a-shadow-root
    KRYS_NODISCARD static ExceptionOr<void>
      AttachShadowRoot(Element &element, ShadowRootMode mode, Clonable clonable, Serialisable serializable,
                       DelegatesFocus delegatesFocus, SlotAssignmentMode slotAssignment,
                       Maybe<DOMString> referenceTarget, html::CustomElementRegistry *registry) noexcept;

    /// @brief Attaches a user agent shadow root to @p element, which must have no shadow root.
    static ShadowRoot &AttachUserAgentShadowRoot(Element &element) noexcept;

  private:
    static void AddShadowRoot(Element &element, const Ref<ShadowRoot> &shadow) noexcept;
  };
}
