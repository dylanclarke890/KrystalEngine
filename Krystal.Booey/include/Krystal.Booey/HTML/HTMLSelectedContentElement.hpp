#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLSelectElement;

  /// @see https://html.spec.whatwg.org/#htmlselectedcontentelement
  class HTMLSelectedContentElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSelectedContentElement);

  private:
    /// @see https://html.spec.whatwg.org/#selectedcontent-disabled
    bool _disabled {false};

  public:
    HTMLSelectedContentElement(dom::Document &document) noexcept;

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
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;

#pragma endregion

  private:
    /// @return The nearest ancestor select.
    /// @see https://html.spec.whatwg.org/#recalculate-a-selectedcontent-element's-disabledness
    HTMLSelectElement *RecalculateDisabledness() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSelectedContentElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement()
           && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLSelectedContentElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSelectedContentElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
