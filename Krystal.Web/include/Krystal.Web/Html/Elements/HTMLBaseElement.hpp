#pragma once

#include "Krystal.Web/Dom/Types/USVString.hpp"
#include "Krystal.Web/Html/Elements/HTMLElement.hpp"

namespace krys::web
{
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#htmlbaseelement
  class HTMLBaseElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLBaseElement);

  public:
    HTMLBaseElement(Document &document) noexcept;

#pragma region HTMLBaseElement - https://html.spec.whatwg.org/multipage/semantics.html#htmlbaseelement

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-base-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-base-href
    void Href(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-base-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-base-target
    void Target(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLBaseElement)
  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsHTMLBaseElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
