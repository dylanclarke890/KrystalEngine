#pragma once

#include "Krystal.Web/Dom/Collections/HTMLCollection.hpp"
#include "Krystal.Web/Dom/Internals/SmallNodeList.hpp"

namespace krys::web
{
  class Element;

  /// @brief A static element collection: the elements it was made with.
  /// @see https://dom.spec.whatwg.org/#concept-collection-static
  class StaticHTMLCollection : public HTMLCollection
  {
  private:
    dom::SmallElementList _elements;

  public:
    explicit StaticHTMLCollection(dom::SmallElementList &&elements) noexcept;

#pragma region HTMLCollection - https://dom.spec.whatwg.org/#htmlcollection

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD RefPtr<Element> Item(size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD RefPtr<const Element> Item(size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD RefPtr<Element> operator[](size_t index) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-item
    KRYS_NODISCARD RefPtr<const Element> operator[](size_t index) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD RefPtr<Element> NamedItem(DOMStringView name) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD RefPtr<const Element> NamedItem(DOMStringView name) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD RefPtr<Element> operator[](DOMStringView name) noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-nameditem-key
    KRYS_NODISCARD RefPtr<const Element> operator[](DOMStringView name) const noexcept override;

    /// @see https://dom.spec.whatwg.org/#dom-htmlcollection-length
    KRYS_NODISCARD size_t Length() const noexcept override;

#pragma endregion

    /// @see https://webidl.spec.whatwg.org/#dfn-supported-property-names
    KRYS_NODISCARD List<DOMString> SupportedPropertyNames() const noexcept override;
  };
}
