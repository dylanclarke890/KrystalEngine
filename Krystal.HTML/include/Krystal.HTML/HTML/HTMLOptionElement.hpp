#pragma once

#include "Krystal.HTML/HTML/HTMLElement.hpp"

namespace Krys::HTML
{
  class HTMLSelectElement;

  /// @see https://html.spec.whatwg.org/#htmloptionelement
  class HTMLOptionElement : public HTMLElement
  {
    friend class FormControlAlgorithms;

    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLOptionElement);

  private:
    /// @see https://html.spec.whatwg.org/#cached-nearest-ancestor-select-element
    RawPtr<HTMLSelectElement> _cachedNearestSelectElement {nullptr};

    /// @see https://html.spec.whatwg.org/#concept-option-selectedness
    bool _selectedness {false};

  public:
    HTMLOptionElement(Document &document) noexcept;

#pragma region HTMLOptionElement - https://html.spec.whatwg.org/#htmloptionelement

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-disabled
    void Disabled(bool value) noexcept;

    // TODO(HTMLOPTION, HTML): Implement the form API.
    // readonly attribute HTMLFormElement? form;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    KRYS_NODISCARD DOMString Label() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-label
    void Label(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    KRYS_NODISCARD bool DefaultSelected() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-defaultselected
    void DefaultSelected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    KRYS_NODISCARD bool Selected() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-selected
    void Selected(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    KRYS_NODISCARD DOMString Value() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-value
    void Value(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-option-text
    void Text(DOMString &&value) noexcept;

    // TODO(HTMLOPTION, HTML): Implement the index attribute.
    // readonly attribute long index;

#pragma endregion

#pragma region Extension Hooks

    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLOptionElement)
  static bool IsType(const Krys::HTML::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<Krys::HTML::HTMLElement>(target).IsHTMLOptionElement();
  }
  static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLOptionElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();