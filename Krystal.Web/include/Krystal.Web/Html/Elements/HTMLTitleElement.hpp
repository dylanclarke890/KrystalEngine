#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmltitleelement
  class HTMLTitleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTitleElement);

  public:
    HTMLTitleElement(Document &document) noexcept;

#pragma region HTMLTitleElement - https://html.spec.whatwg.org/multipage/semantics.html#htmltitleelement

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-title-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-title-text
    ExceptionOr<void> Text(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnChildrenChanged() noexcept override;

  private:
    /// @brief Tells the accessibility cache the document's title may have changed.
    void TitleMayHaveChanged() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLTitleElement)

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLTitleElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return Is<krys::web::HTMLElement>(target)
           && Downcast<krys::web::HTMLElement>(target).IsHTMLTitleElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
