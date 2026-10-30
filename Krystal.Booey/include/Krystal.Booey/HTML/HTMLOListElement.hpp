#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlolistelement
  class HTMLOListElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOListElement);

  private:
    Maybe<int32> _start;
    mutable Maybe<uint32> _itemCount;
    bool _isReversed {false};

  public:
    HTMLOListElement(dom::Document &document) noexcept;

#pragma region HTMLOListElement https://html.spec.whatwg.org/#htmlolistelement

    /// @see https://html.spec.whatwg.org/#dom-ol-reversed
    KRYS_NODISCARD bool Reversed() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-reversed
    void Reversed(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-start
    KRYS_NODISCARD int32 Start() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-start
    void Start(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-type
    void Type(dom::DOMString &&value) noexcept;

#pragma endregion

#pragma region HTMLOListElement Obsolete members - https://html.spec.whatwg.org/#HTMLOListElement-partial

    /// @see https://html.spec.whatwg.org/#dom-ol-compact
    KRYS_NODISCARD bool Compact() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-ol-compact
    void Compact(bool value) noexcept;

#pragma endregion

    /// @brief The value the list's numbering starts from: the start attribute, else the number of items for a
    /// reversed list, else 1.
    /// @note It doesn't trigger layout because it is called from rendering code.
    KRYS_NODISCARD int32 StartValue() const noexcept
    {
      return _start ? *_start : (_isReversed ? static_cast<int32>(ItemCount()) : 1);
    }
    KRYS_NODISCARD bool IsReversed() const noexcept
    {
      return _isReversed;
    }
    void ItemCountChanged() noexcept
    {
      _itemCount = null;
    }

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

  private:
    KRYS_NODISCARD uint32 ItemCount() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLOListElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLOListElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();