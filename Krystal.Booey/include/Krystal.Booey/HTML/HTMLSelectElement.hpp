#pragma once

#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlselectelement
  class HTMLSelectElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectElement);

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override
    {
      return false;
    }
    KRYS_NODISCARD bool CanStartSelection() const noexcept final
    {
      return false;
    }

    HTMLSelectElement(dom::Document &document) noexcept;

#pragma region HTMLSelectElement - https://html.spec.whatwg.org/#htmlselectelement

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    KRYS_NODISCARD bool Multiple() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-multiple
    void Multiple(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-select-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-size
    KRYS_NODISCARD uint32 Size() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-size
    void Size(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-select-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @brief The index of the first selected option in the list of options, or -1 if none is.
    /// @see https://html.spec.whatwg.org/#dom-select-selectedindex
    KRYS_NODISCARD int32 SelectedIndex() const noexcept;

    /// @brief Whether the select is drawn as a drop-down (a menu list) rather than a list box.
    KRYS_NODISCARD bool UsesMenuList() const noexcept;

    // TODO(HTMLSELECT, HTML): Select api.
    // [SameObject] readonly attribute HTMLOptionsCollection options;
    // [CEReactions] attribute unsigned long length;
    // getter HTMLOptionElement? item(unsigned long index);
    // HTMLOptionElement? namedItem(dom::DOMString name);
    // [CEReactions] undefined add((HTMLOptionElement or HTMLOptGroupElement) element, optional (HTMLElement
    // or long)? before = null);
    //
    // [CEReactions] undefined remove(); // ChildNode overload
    // [CEReactions] undefined remove(long index);
    // [CEReactions] setter undefined (unsigned long index, HTMLOptionElement? option);
    //
    // [SameObject] readonly attribute HTMLCollection selectedOptions;
    // attribute long selectedIndex;
    // attribute dom::DOMString value;
    //
    // undefined showPicker();

#pragma endregion

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsRequiredFormControl() const noexcept final
    {
      return IsRequired();
    }

    KRYS_NODISCARD bool IsOptionalFormControl() const noexcept final
    {
      return !IsRequiredFormControl();
    }

    KRYS_NODISCARD bool ValueMissing() const noexcept final;

    bool AppendFormData(DOMFormData &formData) noexcept final;

    /// @see https://html.spec.whatwg.org/#the-select-element:concept-form-reset-control
    void Reset() noexcept final;

  private:
    /// @see https://html.spec.whatwg.org/#placeholder-label-option
    KRYS_NODISCARD bool HasPlaceholderLabelOption() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSelectElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return Is<krys::boo::html::HTMLElement>(target)
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLSelectElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();