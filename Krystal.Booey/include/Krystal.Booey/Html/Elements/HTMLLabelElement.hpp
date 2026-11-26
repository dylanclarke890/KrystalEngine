#pragma once

#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo
{
  class HTMLFormElement;

  /// @see https://html.spec.whatwg.org/multipage/forms.html#htmllabelelement
  class HTMLLabelElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLLabelElement);

  private:
    bool _processingClick {false};

  public:
    HTMLLabelElement(Document &document) noexcept;

#pragma region HTMLLabelElement - https://html.spec.whatwg.org/multipage/forms.html#htmllabelelement

    /// @brief The form owner of the labeled control, if it has one.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-label-form
    KRYS_NODISCARD HTMLFormElement *Form() const noexcept;

    /// @brief The labeled control's form owner, retargeted against the label.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-label-form
    KRYS_NODISCARD RefPtr<Element> FormForBindings() const noexcept;

    /// @brief The labeled control: the element the `for` attribute names, or the first labelable
    /// descendant.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#labeled-control
    KRYS_NODISCARD RefPtr<HTMLElement> Control() const noexcept;

    /// @brief Runs the labeled control's access key action, if it has one.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

    /// @brief The labeled control, retargeted against the label: a control in a shadow tree shows as its
    /// host.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-label-control
    KRYS_NODISCARD RefPtr<HTMLElement> ControlForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-label-htmlfor
    KRYS_NODISCARD DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-label-htmlfor
    void HtmlFor(DOMString value) noexcept;

#pragma endregion

    void SetActive(bool down,
                   style::InvalidationScope invalidationScope = style::InvalidationScope::All) noexcept final;

    void SetHovered(bool over, style::InvalidationScope invalidationScope = style::InvalidationScope::All,
                    HitTestRequest request = {}) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

  private:
    /// @brief Whether @p event targets interactive content inside the label (other than its control), which
    /// the label's activation leaves alone.
    KRYS_NODISCARD bool IsEventTargetedAtInteractiveDescendants(Event &event) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLLabelElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLLabelElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLLabelElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
