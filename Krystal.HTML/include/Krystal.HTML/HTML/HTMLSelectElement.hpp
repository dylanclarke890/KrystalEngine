#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmlselectelement
  class HTMLSelectElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectElement);

  public:
    HTMLSelectElement(Document &document) noexcept;

#pragma region HTMLSelectElement - https://html.spec.whatwg.org/#htmlselectelement

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    void Autocomplete(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLSELECT, HTML): Implement the form API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    KRYS_NODISCARD bool Multiple() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    void Name(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-size
    KRYS_NODISCARD uint32 Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-size
    void Size(uint32 value) noexcept;

    // TODO(HTMLSELECT, HTML): Implement the type attribute.
    // @see https://html.spec.whatwg.org/#dom-select-type
    // KRYS_NODISCARD DOMString Type() const noexcept;
    // readonly attribute DOMString type;
    //
    // TODO(HTMLSELECT, HTML): Select api.
    // [SameObject] readonly attribute HTMLOptionsCollection options;
    // [CEReactions] attribute unsigned long length;
    // getter HTMLOptionElement? item(unsigned long index);
    // HTMLOptionElement? namedItem(DOMString name);
    // [CEReactions] undefined add((HTMLOptionElement or HTMLOptGroupElement) element, optional (HTMLElement
    // or long)? before = null);
    //
    // [CEReactions] undefined remove(); // ChildNode overload
    // [CEReactions] undefined remove(long index);
    // [CEReactions] setter undefined (unsigned long index, HTMLOptionElement? option);
    //
    // [SameObject] readonly attribute HTMLCollection selectedOptions;
    // attribute long selectedIndex;
    // attribute DOMString value;
    //
    // TODO(HTMLSELECT, HTML): Implement validity API.
    // readonly attribute boolean willValidate;
    // readonly attribute ValidityState validity;
    // readonly attribute DOMString validationMessage;
    // boolean checkValidity();
    // boolean reportValidity();
    // undefined setCustomValidity(DOMString error);
    //
    // undefined showPicker();
    //
    // readonly attribute NodeList labels;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLSelectElement)
  static bool IsType(const Krys::HTML::Element &target) noexcept
  {
    return Is<Krys::HTML::HTMLElement>(target)
           && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLSelectElement();
  }

  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();