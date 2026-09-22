#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Numeric.hpp"
#include <algorithm>

namespace krys::boo::dom
{
  class Attr;
  class Element;

  /// @see https://dom.spec.whatwg.org/#interface-namednodemap
  class NamedNodeMap
  {
    friend class Element;

  private:
    CheckedRef<Element> _associatedElement;

    explicit NamedNodeMap(Element &associatedElement) noexcept
        : _associatedElement(ShareCheckedRef(associatedElement))
    {
    }

  public:
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

  protected:
    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-names
    KRYS_NODISCARD virtual List<DOMString> SupportedPropertyNames() const noexcept;

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