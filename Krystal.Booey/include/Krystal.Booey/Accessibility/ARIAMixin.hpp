#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::dom
{
  class Element;
}

// A `DOMString?` attribute reflecting the `aria-*` content attribute `attributeName`.
#define KRYS_ARIA_STRING_ATTRIBUTE(Name, attributeName)                                                      \
  KRYS_NODISCARD Maybe<dom::DOMString> Name() const noexcept                                                 \
  {                                                                                                          \
    return ReflectedString(dom::DOMStringAtom(attributeName));                                               \
  }                                                                                                          \
  void Name(Maybe<dom::DOMString> value) noexcept                                                            \
  {                                                                                                          \
    SetReflectedString(dom::DOMStringAtom(attributeName), krys::move(value));                                \
  }

// A `FrozenArray<Element>?` attribute reflecting the ID reference list `attributeName`.
#define KRYS_ARIA_ELEMENTS_ATTRIBUTE(Name, attributeName)                                                    \
  KRYS_NODISCARD Maybe<List<Ref<dom::Element>>> Name() const noexcept                                        \
  {                                                                                                          \
    return AttrAssociatedElements(dom::DOMStringAtom(attributeName));                                        \
  }                                                                                                          \
  void Name(const Maybe<List<Ref<dom::Element>>> &elements) noexcept                                         \
  {                                                                                                          \
    SetAttrAssociatedElements(dom::DOMStringAtom(attributeName), elements);                                  \
  }

namespace krys::boo::aria::mixins
{
  /// @brief The IDL attributes of an element's ARIA role, states and properties, each reflecting the content
  /// attribute of the same name (`ariaLabel` reflects `aria-label`). On an element they reflect its
  /// attributes; on its `ElementInternals`, the defaults a custom element gives itself.
  /// @tparam TTarget The reflected target: `dom::Element` or `html::ElementInternals`.
  /// @see https://w3c.github.io/aria/#idl-interface
  /// @see https://html.spec.whatwg.org/#accessibility-semantics
  template <typename TTarget>
  class ARIAMixin
  {
  public:
    /// @see https://w3c.github.io/aria/#dom-ariamixin-role
    KRYS_ARIA_STRING_ATTRIBUTE(Role, u8"role")

    /// @brief The element `aria-activedescendant` refers to.
    /// @see https://w3c.github.io/aria/#dom-ariamixin-ariaactivedescendantelement
    KRYS_NODISCARD RefPtr<dom::Element> AriaActiveDescendantElement() const noexcept;

    /// @see https://w3c.github.io/aria/#dom-ariamixin-ariaactivedescendantelement
    void AriaActiveDescendantElement(dom::Element *element) noexcept;

    // The states and properties.
    KRYS_ARIA_STRING_ATTRIBUTE(AriaAtomic, u8"aria-atomic")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaAutoComplete, u8"aria-autocomplete")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaBrailleLabel, u8"aria-braillelabel")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaBrailleRoleDescription, u8"aria-brailleroledescription")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaBusy, u8"aria-busy")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaChecked, u8"aria-checked")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaColCount, u8"aria-colcount")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaColIndex, u8"aria-colindex")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaColIndexText, u8"aria-colindextext")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaColSpan, u8"aria-colspan")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaControlsElements, u8"aria-controls")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaCurrent, u8"aria-current")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaDescribedByElements, u8"aria-describedby")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaDescription, u8"aria-description")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaDetailsElements, u8"aria-details")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaDisabled, u8"aria-disabled")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaErrorMessageElements, u8"aria-errormessage")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaExpanded, u8"aria-expanded")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaFlowToElements, u8"aria-flowto")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaHasPopup, u8"aria-haspopup")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaHidden, u8"aria-hidden")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaInvalid, u8"aria-invalid")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaKeyShortcuts, u8"aria-keyshortcuts")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaLabel, u8"aria-label")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaLabelledByElements, u8"aria-labelledby")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaLevel, u8"aria-level")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaLive, u8"aria-live")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaModal, u8"aria-modal")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaMultiLine, u8"aria-multiline")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaMultiSelectable, u8"aria-multiselectable")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaOrientation, u8"aria-orientation")
    KRYS_ARIA_ELEMENTS_ATTRIBUTE(AriaOwnsElements, u8"aria-owns")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaPlaceholder, u8"aria-placeholder")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaPosInSet, u8"aria-posinset")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaPressed, u8"aria-pressed")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaReadOnly, u8"aria-readonly")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRelevant, u8"aria-relevant")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRequired, u8"aria-required")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRoleDescription, u8"aria-roledescription")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRowCount, u8"aria-rowcount")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRowIndex, u8"aria-rowindex")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRowIndexText, u8"aria-rowindextext")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaRowSpan, u8"aria-rowspan")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaSelected, u8"aria-selected")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaSetSize, u8"aria-setsize")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaSort, u8"aria-sort")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaValueMax, u8"aria-valuemax")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaValueMin, u8"aria-valuemin")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaValueNow, u8"aria-valuenow")
    KRYS_ARIA_STRING_ATTRIBUTE(AriaValueText, u8"aria-valuetext")

  private:
    KRYS_NODISCARD const TTarget &ReflectedTarget() const noexcept
    {
      return static_cast<const TTarget &>(*this);
    }

    KRYS_NODISCARD TTarget &ReflectedTarget() noexcept
    {
      return static_cast<TTarget &>(*this);
    }

    /// @brief The `DOMString?` getter of a reflected attribute.
    /// @see https://html.spec.whatwg.org/#reflecting-content-attributes-in-idl-attributes:idl-domstring-2
    KRYS_NODISCARD Maybe<dom::DOMString> ReflectedString(dom::DOMStringAtom name) const noexcept;

    /// @brief The `DOMString?` setter of a reflected attribute.
    void SetReflectedString(dom::DOMStringAtom name, Maybe<dom::DOMString> value) noexcept;

    /// @brief The `FrozenArray<Element>?` getter of a reflected attribute.
    /// @see https://html.spec.whatwg.org/#attr-associated-elements
    KRYS_NODISCARD Maybe<List<Ref<dom::Element>>>
      AttrAssociatedElements(dom::DOMStringAtom name) const noexcept;

    /// @brief The `FrozenArray<Element>?` setter of a reflected attribute.
    void SetAttrAssociatedElements(dom::DOMStringAtom name,
                                   const Maybe<List<Ref<dom::Element>>> &elements) noexcept;
  };
}

#undef KRYS_ARIA_STRING_ATTRIBUTE
#undef KRYS_ARIA_ELEMENTS_ATTRIBUTE
