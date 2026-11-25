#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQueryForward.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Internals/InlineStyleSheetOwner.hpp"

namespace krys::boo
{
  class CSSStyleSheet;

  /// @brief The `style` element: a style sheet written in the document, which its InlineStyleSheetOwner
  /// keeps in step with the element's text, `type` and `media`.
  /// @see https://html.spec.whatwg.org/multipage/semantics.html#the-style-element
  class HTMLStyleElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLStyleElement);

  private:
    UniquePtr<DOMTokenList> _blocking;
    html::InlineStyleSheetOwner _styleSheetOwner;
    /// @brief Whether the parser made the element, which makes it potentially render-blocking.
    bool _createdByParser {false};
    /// @brief Whether the element is blocking rendering.
    bool _isRenderBlocking {false};

  public:
    HTMLStyleElement(Document &document) noexcept;

    ~HTMLStyleElement() noexcept override;

#pragma region HTMLStyleElement - https://html.spec.whatwg.org/multipage/semantics.html#htmlstyleelement

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-style-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-style-disabled
    void Disabled(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-style-media
    KRYS_NODISCARD DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-style-media
    void Media(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics.html#dom-style-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

#pragma endregion

#pragma region HTMLStyleElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLStyleElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-style-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-style-type
    void Type(DOMString value) noexcept;

#pragma endregion

#pragma region LinkStyle - https://drafts.csswg.org/cssom/#the-linkstyle-interface

    /// @brief The associated CSS style sheet, if any.
    /// @see https://drafts.csswg.org/cssom/#dom-linkstyle-sheet
    KRYS_NODISCARD CSSStyleSheet *Sheet() const noexcept;

#pragma endregion

  protected:
    /// @brief Keeps the sheet in step with `type`, `media` and `title`, and stops blocking rendering when
    /// `blocking` loses `render` (unless the parser made the element).
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnChildrenChanged() noexcept override;

    void FinishParsingChildren() noexcept override;

  public:
    /// @brief Blocks rendering on the element if it's potentially render-blocking (its `blocking` attribute
    /// has `render`, or the parser made it) and @p mediaQueries, its `media`, match the environment.
    /// @see https://html.spec.whatwg.org/multipage/semantics.html#update-a-style-block
    void PotentiallyBlockRendering(const css::mq::MediaQueryList &mediaQueries) noexcept;

    /// @brief Stops blocking rendering on the element, if it was.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#unblock-rendering
    void UnblockRendering() noexcept;

    /// @brief Takes the element off its scope's pending sheets, unless the sheet is still loading.
    /// @return Whether the sheet had loaded.
    bool SheetLoaded() noexcept override;

    /// @brief Fires `load`, or `error` if @p errorOccurred, once the sheet and its imports have loaded, and
    /// stops blocking rendering.
    void NotifyLoadedSheetAndAllCriticalSubresources(bool errorOccurred) noexcept override;

    /// @brief Makes the scope wait for the sheet again, as one of its imports has started loading.
    void StartLoadingDynamicSheet() noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLStyleElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLStyleElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::HTMLElement>(target);
    return element && element->IsHTMLStyleElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
