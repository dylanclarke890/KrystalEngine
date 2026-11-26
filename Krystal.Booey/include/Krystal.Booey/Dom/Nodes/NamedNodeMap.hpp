#pragma once

#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Booey/Dom/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/CheckedPtr.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>

namespace krys::boo
{
  class Attr;
  class Element;
}

namespace krys::boo::dom
{
  /// @brief A live view of an element's attribute list.
  /// @see https://dom.spec.whatwg.org/#interface-namednodemap
  /// @see https://dom.spec.whatwg.org/#concept-namednodemap-attribute
  class NamedNodeMap
  {
    friend class boo::Element;

  private:
    /// @see https://dom.spec.whatwg.org/#concept-namednodemap-element
    CheckedRef<Element> _associatedElement;

    explicit NamedNodeMap(Element &associatedElement) noexcept;

  public:
    ~NamedNodeMap() noexcept;

#pragma region NamedNodeMap - https://dom.spec.whatwg.org/#namednodemap

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-length
    KRYS_NODISCARD size_t Length() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-item
    KRYS_NODISCARD RefPtr<Attr> Item(size_t index) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-item
    KRYS_NODISCARD RefPtr<const Attr> Item(size_t index) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem
    KRYS_NODISCARD RefPtr<Attr> GetNamedItem(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem
    KRYS_NODISCARD RefPtr<const Attr> GetNamedItem(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditemns
    KRYS_NODISCARD RefPtr<Attr> GetNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditemns
    KRYS_NODISCARD RefPtr<const Attr> GetNamedItemNS(DOMStringAtom attrNamespace,
                                                     DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-setnameditem
    ExceptionOr<RefPtr<Attr>> SetNamedItem(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-setnameditemns
    ExceptionOr<RefPtr<Attr>> SetNamedItemNS(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-removenameditem
    ExceptionOr<Ref<Attr>> RemoveNamedItem(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-removenameditemns
    ExceptionOr<Ref<Attr>> RemoveNamedItemNS(DOMStringAtom attrNamespace, DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-item
    KRYS_NODISCARD RefPtr<Attr> operator[](size_t index) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-item
    KRYS_NODISCARD RefPtr<const Attr> operator[](size_t index) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem
    KRYS_NODISCARD RefPtr<Attr> operator[](DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-getnameditem
    KRYS_NODISCARD RefPtr<const Attr> operator[](DOMStringAtom qualifiedName) const noexcept;

    /// @brief The attributes' qualified names, without duplicates, leaving out those with uppercase ASCII
    /// letters for an HTML element in an HTML document.
    /// @see https://dom.spec.whatwg.org/#dom-namednodemap-item
    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-names
    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept;

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-indices
    KRYS_NODISCARD bool IsSupportedPropertyIndex(size_t index) const noexcept
    {
      return index < Length();
    }

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-names
    KRYS_NODISCARD bool IsSupportedPropertyName(DOMStringView name) const noexcept
    {
      auto supportedNames = SupportedPropertyNames();
      return std::find(supportedNames.begin(), supportedNames.end(), name) != supportedNames.end();
    }

#pragma endregion
  };
}
