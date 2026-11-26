#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  class HTMLFormElement;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmllegendelement
  class HTMLLegendElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLegendElement);

  public:
    HTMLLegendElement(Document &document) noexcept;

    /// @brief The element whose command the legend's access key stands for.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#the-legend-element's-accesskey-delegatee
    KRYS_NODISCARD RefPtr<Element> AccessKeyDelegatee() const noexcept;

    /// @brief Runs its delegatee's access key action; without one, the action of any other element.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

#pragma region HTMLLegendElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmllegendelement

    /// @brief The form of the fieldset the legend is a child of, if any.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-legend-form
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @brief The fieldset's form, retargeted against the legend.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-legend-form
    KRYS_NODISCARD RefPtr<Element> FormForBindings() const noexcept;

#pragma endregion

#pragma region HTMLLegendElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLLegendElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-legend-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-legend-align
    void Align(DOMString value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLLegendElement)
  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLLegendElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLLegendElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
