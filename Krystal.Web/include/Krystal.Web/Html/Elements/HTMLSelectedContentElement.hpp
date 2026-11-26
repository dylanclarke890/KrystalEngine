#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  class HTMLSelectElement;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmlselectedcontentelement
  class HTMLSelectedContentElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectedContentElement);

  private:
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#selectedcontent-disabled
    bool _disabled {false};

  public:
    HTMLSelectedContentElement(Document &document) noexcept;

    /// @brief Whether the element is in an option, another selectedcontent, or a select inside another
    /// select, so that it mirrors no option.
    KRYS_NODISCARD bool Disabled() const noexcept
    {
      return _disabled;
    }

  protected:
#pragma region Extension Hooks

    void OnInsert() noexcept override;

    void OnPostConnection() noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

#pragma endregion

  private:
    /// @return The nearest ancestor select.
    /// @see https://html.spec.whatwg.org/#recalculate-a-selectedcontent-element's-disabledness
    HTMLSelectElement *RecalculateDisabledness() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLSelectedContentElement)

  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsHTMLSelectedContentElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectedContentElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
