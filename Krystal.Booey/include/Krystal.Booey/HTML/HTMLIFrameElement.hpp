#pragma once

#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"

namespace krys::boo::html
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
    KRYS_NODISCARD dom::ExceptionOr<USVString> Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-src
    dom::ExceptionOr<void> Src(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-srcdoc
    KRYS_NODISCARD dom::DOMString SrcDoc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-srcdoc
    void SrcDoc(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    KRYS_NODISCARD dom::DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-img-sizes
    void Sizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see  https://html.spec.whatwg.org/#dom-iframe-sandbox
    KRYS_NODISCARD DOMTokenList &Sandbox() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allow
    KRYS_NODISCARD dom::DOMString Allow() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allow
    void Allow(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allowfullscreen
    KRYS_NODISCARD bool AllowFullscreen() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-allowfullscreen
    void AllowFullscreen(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    void Width(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    void Height(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-referrerpolicy
    KRYS_NODISCARD dom::DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-referrerpolicy
    void ReferrerPolicy(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-loading
    KRYS_NODISCARD dom::DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-loading
    void Loading(dom::DOMString &&value) noexcept;

    // TODO(HTMLIFRAMEELEMENT, HTML): Implement the contentDocument, contentWindow and getSVGDocument()
    // methods/attributes.
    // readonly attribute Document? contentDocument;
    // readonly attribute WindowProxy? contentWindow;
    // Document? getSVGDocument();

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members - https://html.spec.whatwg.org/#HTMLIFrameElement-partial

    /// @see https://html.spec.whatwg.org/#dom-iframe-align
    KRYS_NODISCARD dom::DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-align
    void Align(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-scrolling
    KRYS_NODISCARD dom::DOMString Scrolling() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-scrolling
    void Scrolling(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-frameborder
    KRYS_NODISCARD dom::DOMString FrameBorder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-frameborder
    void FrameBorder(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-longdesc
    KRYS_NODISCARD dom::ExceptionOr<USVString> LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-longdesc
    dom::ExceptionOr<void> LongDesc(USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginwidth
    KRYS_NODISCARD dom::DOMString MarginWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginwidth
    void MarginWidth(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginheight
    KRYS_NODISCARD dom::DOMString MarginHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-marginheight
    void MarginHeight(dom::DOMString &&value) noexcept;

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLIFrameElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLIFrameElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();