#pragma once

#include "Krystal.Booey/Dom/Types/USVString.hpp"
#include "Krystal.Booey/Html/Elements/HTMLElement.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmliframeelement
  class HTMLIFrameElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLIFrameElement);

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return false;
    }

    HTMLIFrameElement(Document &document) noexcept;

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsReplaced(const RenderStyle * = nullptr) const noexcept override
    {
      return true;
    }

#pragma region HTMLIFrameElement - https://html.spec.whatwg.org/multipage/iframe-embed-object.html#htmliframeelement

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-srcdoc
    KRYS_NODISCARD DOMString SrcDoc() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-srcdoc
    void SrcDoc(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-sizes
    KRYS_NODISCARD DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/embedded-content.html#dom-img-sizes
    void Sizes(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-allow
    KRYS_NODISCARD DOMString Allow() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-allow
    void Allow(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-allowfullscreen
    KRYS_NODISCARD bool AllowFullscreen() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-allowfullscreen
    void AllowFullscreen(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    KRYS_NODISCARD DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-width
    void Width(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    KRYS_NODISCARD DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-iframe-height
    void Height(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-loading
    KRYS_NODISCARD DOMString Loading() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/iframe-embed-object.html#dom-iframe-loading
    void Loading(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLIFrameElement Obsolete members - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLIFrameElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-align
    KRYS_NODISCARD DOMString Align() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-align
    void Align(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-scrolling
    KRYS_NODISCARD DOMString Scrolling() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-scrolling
    void Scrolling(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-frameborder
    KRYS_NODISCARD DOMString FrameBorder() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-frameborder
    void FrameBorder(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-longdesc
    KRYS_NODISCARD dom::USVString LongDesc() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-longdesc
    void LongDesc(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-marginwidth
    KRYS_NODISCARD DOMString MarginWidth() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-marginwidth
    void MarginWidth(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-marginheight
    KRYS_NODISCARD DOMString MarginHeight() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-iframe-marginheight
    void MarginHeight(DOMString value) noexcept;

#pragma endregion

  protected:
    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLIFrameElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLIFrameElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
