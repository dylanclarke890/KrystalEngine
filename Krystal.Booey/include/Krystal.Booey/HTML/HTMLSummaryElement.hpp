#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::html
{
  class HTMLDetailsElement;

  /// @brief The caption of a `details` element: activating its details' active summary opens or closes it.
  /// @see https://html.spec.whatwg.org/#the-summary-element
  class HTMLSummaryElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSummaryElement);

  public:
    explicit HTMLSummaryElement(dom::Document &document) noexcept;

    /// @brief Whether the element is its details' summary: the first `summary` child of its `details`
    /// parent, or the default summary of a `details` element without one.
    /// @see https://html.spec.whatwg.org/#summary-for-its-parent-details
    KRYS_NODISCARD bool IsActiveSummary() const noexcept;

    /// @see https://html.spec.whatwg.org/#the-summary-element:activation-behaviour
    void ActivationBehavior(dom::Event &event) noexcept final;

    void DefaultEventHandler(dom::Event &event) noexcept final;

  protected:
#pragma region Extensibility Hooks

    void OnInsert() noexcept final;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept final;

#pragma endregion

  private:
    KRYS_NODISCARD RefPtr<HTMLDetailsElement> DetailsElement() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSummaryElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLSummaryElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSummaryElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
