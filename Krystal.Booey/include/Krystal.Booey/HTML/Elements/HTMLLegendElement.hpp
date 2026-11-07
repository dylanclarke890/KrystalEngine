#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLFormElement;

  /// @see https://html.spec.whatwg.org/#htmllegendelement
  class HTMLLegendElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLegendElement);

  public:
    HTMLLegendElement(dom::Document &document) noexcept;

    /// @brief The element whose command the legend's access key stands for.
    /// @see https://html.spec.whatwg.org/#the-legend-element's-accesskey-delegatee
    KRYS_NODISCARD RefPtr<dom::Element> AccessKeyDelegatee() const noexcept;

    /// @brief Runs its delegatee's access key action; without one, the action of any other element.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

#pragma region HTMLLegendElement - https://html.spec.whatwg.org/#htmllegendelement

    /// @brief The form of the fieldset the legend is a child of, if any.
    /// @see https://html.spec.whatwg.org/#dom-legend-form
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-legend-form
    KRYS_NODISCARD RefPtr<HTMLFormElement> FormForBindings() const noexcept;

#pragma endregion

#pragma region HTMLLegendElement Obsolete members - https://html.spec.whatwg.org/#HTMLLegendElement-partial

    /// @see https://html.spec.whatwg.org/#dom-legend-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-legend-align
    void Align(dom::DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLLegendElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLLegendElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLLegendElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
