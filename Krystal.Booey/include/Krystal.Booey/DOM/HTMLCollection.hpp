#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @see https://dom.spec.whatwg.org/#interface-htmlcollection
  class HTMLCollection : public RefCounted<HTMLCollection>, public CanMakeWeakPtr<HTMLCollection>
  {
  public:
    virtual ~HTMLCollection() noexcept = default;

#pragma region HTMLCollection - https://dom.spec.whatwg.org/#interface-htmlcollection

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD virtual RefPtr<Element> Item(size_t index) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD virtual RefPtr<const Element> Item(size_t index) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD virtual RefPtr<Element> operator[](size_t index) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD virtual RefPtr<const Element> operator[](size_t index) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD virtual RefPtr<Element> NamedItem(DOMStringView name) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD virtual RefPtr<const Element> NamedItem(DOMStringView name) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD virtual RefPtr<Element> operator[](DOMStringView name) noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD virtual RefPtr<const Element> operator[](DOMStringView name) const noexcept = 0;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-length
    KRYS_NODISCARD virtual size_t Length() const noexcept = 0;

  protected:
    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-names
    KRYS_NODISCARD virtual List<DOMString> SupportedPropertyNames() const noexcept = 0;

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