#pragma once

#include "Krystal.Web/Html/Elements/HTMLFormControlElement.hpp"

namespace krys::web
{
  class HTMLCollection;
  class HTMLLegendElement;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmlfieldsetelement
  class HTMLFieldSetElement : public HTMLFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFieldSetElement);

  private:
    List<WeakPtr<HTMLElement>> _invalidDescendants;
    // A [SameObject] collection, held weakly as it holds the fieldset.
    WeakPtr<HTMLCollection> _elements;

  public:
    HTMLFieldSetElement(Document &document) noexcept;

    ~HTMLFieldSetElement() noexcept override;

#pragma region HTMLFieldSetElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmlfieldsetelement

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-type
    KRYS_NODISCARD static const DOMStringAtom &Type() noexcept
    {
      static const DOMStringAtom type {u8"fieldset"};
      return type;
    }

    /// @brief The fieldset's listed elements.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-fieldset-elements
    KRYS_NODISCARD Ref<HTMLCollection> Elements() noexcept;

#pragma endregion

    /// @brief The fieldset's first legend child, if any.
    KRYS_NODISCARD HTMLLegendElement *Legend() const noexcept;

    /// @brief Records an invalid control in the fieldset, or forgets one, restyling the fieldset's :valid and
    /// :invalid as the first comes or the last goes.
    void AddInvalidDescendant(const HTMLElement &invalidFormControlElement) noexcept;

    void RemoveInvalidDescendant(const HTMLElement &formControlElement) noexcept;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept final;

    KRYS_NODISCARD bool IsActuallyDisabled() const noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD DOMString FormControlType() const noexcept final
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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLFieldSetElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLFieldSetElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLFieldSetElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
