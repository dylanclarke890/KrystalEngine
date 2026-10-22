#pragma once

#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"

namespace krys::boo::dom
{
  class HTMLCollection;
}

namespace krys::boo::html
{
  class HTMLLegendElement;

  /// @see https://html.spec.whatwg.org/#htmlfieldsetelement
  class HTMLFieldSetElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFieldSetElement);

  private:
    List<WeakPtr<HTMLElement>> _invalidDescendants;

  public:
    HTMLFieldSetElement(dom::Document &document) noexcept;
    ~HTMLFieldSetElement() noexcept override;

#pragma region HTMLFieldSetElement - https://html.spec.whatwg.org/#htmlfieldsetelement

    /// @see https://html.spec.whatwg.org/#dom-fieldset-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fieldset-type
    KRYS_NODISCARD dom::DOMStringAtom Type() const noexcept
    {
      return u8"fieldset";
    }

    /// @brief The fieldset's listed elements.
    /// @see https://html.spec.whatwg.org/#dom-fieldset-elements
    KRYS_NODISCARD Ref<dom::HTMLCollection> Elements() noexcept;

#pragma endregion

    /// @brief The fieldset's first legend child, if any.
    KRYS_NODISCARD HTMLLegendElement *Legend() const noexcept;

    void AddInvalidDescendant(const HTMLElement &invalidFormControlElement) noexcept;
    void RemoveInvalidDescendant(const HTMLElement &formControlElement) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;
    KRYS_NODISCARD bool IsActuallyDisabled() const noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final
    {
      return u8"fieldset";
    }

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept final;
    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept final;

  protected:
    KRYS_NODISCARD bool ComputeWillValidate() const noexcept final
    {
      return false;
    }

    void DisabledStateChanged() noexcept final;
    void OnChildrenChanged() noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFieldSetElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLFieldSetElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLFieldSetElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
