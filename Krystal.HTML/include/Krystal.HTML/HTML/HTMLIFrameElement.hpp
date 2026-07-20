#pragma once

#include "Krystal.HTML/DOM/DOMTokenList.hpp"
#include "Krystal.HTML/HTML/HTMLElement.hpp"
#include "Krystal.HTML/DOM/Types/USVString.hpp"

namespace Krys::HTML
{
  /// @see https://html.spec.whatwg.org/#htmliframeelement
  class HTMLIFrameElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLIFrameElement);

  private:
    UniquePtr<DOMTokenList> _sandbox;

  public:
    HTMLIFrameElement(Document &document) noexcept;

#pragma region HTMLIFrameElement - https://html.spec.whatwg.org/#htmliframeelement

    /// @see https://html.spec.whatwg.org/#dom-iframe-src
    KRYS_NODISCARD ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-src
    ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-srcdoc
    KRYS_NODISCARD DOMString SrcDoc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-srcdoc
    void SrcDoc(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-name
    void Name(DOMString &&value) noexcept;

    /// @see  https://html.spec.whatwg.org/#dom-iframe-sandbox
    KRYS_NODISCARD DOMTokenList &Sandbox() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allow
    KRYS_NODISCARD DOMString Allow() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allow
    void Allow(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allowfullscreen
    KRYS_NODISCARD bool AllowFullscreen() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allowfullscreen
    void AllowFullscreen(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    void Width(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    void Height(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-referrerpolicy
    void ReferrerPolicy(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-loading
    KRYS_NODISCARD DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-loading
    void Loading(DOMString &&value) noexcept;

    // TODO(HTMLIFRAMEELEMENT, HTML): Implement the contentDocument, contentWindow and getSVGDocument()
    // methods/attributes.
    // readonly attribute Document? contentDocument;
    // readonly attribute WindowProxy? contentWindow;
    // Document? getSVGDocument();

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members - https://html.spec.whatwg.org/#HTMLIFrameElement-partial

    /// @see https://html.spec.whatwg.org/#dom-iframe-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-align
    void Align(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-scrolling
    KRYS_NODISCARD DOMString Scrolling() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-scrolling
    void Scrolling(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-frameborder
    KRYS_NODISCARD DOMString FrameBorder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-frameborder
    void FrameBorder(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-longdesc
    KRYS_NODISCARD ExceptionOr<USVString> LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-longdesc
    ExceptionOr<void> LongDesc(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginwidth
    KRYS_NODISCARD DOMString MarginWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginwidth
    void MarginWidth(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginheight
    KRYS_NODISCARD DOMString MarginHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginheight
    void MarginHeight(DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::HTMLIFrameElement)
  KRYS_NODISCARD static bool IsType(const Krys::HTML::HTMLElement &target) noexcept
  {
    return target.IsHTMLIFrameElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();