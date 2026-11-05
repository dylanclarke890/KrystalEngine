#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Internals/InlineStyleSheetOwner.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmlstyleelement
  class HTMLStyleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLStyleElement);

  private:
    UniquePtr<dom::DOMTokenList> _blocking;
    InlineStyleSheetOwner _styleSheetOwner;
    bool _loadedSheet {false};
    bool _createdByParser {false};
    bool _isRenderBlocking {false};

  public:
    HTMLStyleElement(dom::Document &document) noexcept;
    ~HTMLStyleElement() noexcept override;

#pragma region HTMLStyleElement - https://html.spec.whatwg.org/#htmlstyleelement

    /// @see https://html.spec.whatwg.org/#dom-style-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-media
    void Media(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-blocking
    KRYS_NODISCARD dom::DOMTokenList &Blocking() noexcept;

#pragma endregion

#pragma region HTMLStyleElement Obsolete members - https://html.spec.whatwg.org/#HTMLStyleElement-partial

    /// @see https://html.spec.whatwg.org/#dom-style-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-style-type
    void Type(dom::DOMString value) noexcept;

#pragma endregion

#pragma region LinkStyle - https://drafts.csswg.org/cssom/#the-linkstyle-interface

    /// @brief The associated CSS style sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnChildrenChanged() noexcept override;
    void FinishParsingChildren() noexcept override;

  public:
    /// @brief Blocks rendering on the element if it's potentially render-blocking (its `blocking` attribute
    /// has `render`, or the parser made it) and its `media` matches the environment.
    /// @see https://html.spec.whatwg.org/#update-a-style-block
    void PotentiallyBlockRendering(const css::mq::MediaQueryList &mediaQueries) noexcept;
    /// @see https://html.spec.whatwg.org/#unblock-rendering
    void UnblockRendering() noexcept;

    bool SheetLoaded() noexcept override;
    void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept override;
    void StartLoadingDynamicSheet() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLStyleElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLStyleElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLStyleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();