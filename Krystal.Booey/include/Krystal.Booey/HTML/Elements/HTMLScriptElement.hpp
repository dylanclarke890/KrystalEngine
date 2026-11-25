#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"

namespace krys::boo::html
{
  class TreeBuilder;
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/scripting.html#htmlscriptelement
  class HTMLScriptElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLScriptElement);

    friend class html::TreeBuilder;

  private:
    UniquePtr<DOMTokenList> _blocking;
    Document *_parserDocument {nullptr};
    bool _forceAsync : 1 {true};
    bool _alreadyStarted : 1 {false};

  public:
    HTMLScriptElement(Document &document) noexcept;

    ~HTMLScriptElement() noexcept override;

#pragma region HTMLScriptElement - https://html.spec.whatwg.org/multipage/scripting.html#htmlscriptelement

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-src
    void Src(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-nomodule
    KRYS_NODISCARD bool NoModule() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-nomodule
    void NoModule(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-async
    KRYS_NODISCARD bool Async() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-async
    void Async(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-defer
    KRYS_NODISCARD bool Defer() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-defer
    void Defer(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-blocking
    KRYS_NODISCARD DOMTokenList &Blocking() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-crossorigin
    KRYS_NODISCARD Maybe<DOMString> CrossOrigin() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-crossorigin
    void CrossOrigin(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-integrity
    KRYS_NODISCARD DOMString Integrity() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-integrity
    void Integrity(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-fetchpriority
    KRYS_NODISCARD DOMString FetchPriority() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-fetchpriority
    void FetchPriority(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-text
    void Text(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/scripting.html#dom-script-supports
    KRYS_NODISCARD static bool Supports(DOMStringView type) noexcept;

#pragma endregion

#pragma region HTMLScriptElement Obsolete members  - https://html.spec.whatwg.org/multipage/obsolete.html#HTMLScriptElement-partial

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-charset
    void Charset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-event
    KRYS_NODISCARD DOMString Event() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-event
    void Event(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-htmlfor
    KRYS_NODISCARD DOMString HtmlFor() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/obsolete.html#dom-script-htmlfor
    void HtmlFor(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void OnClone(Node &copy, bool subtree) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLScriptElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return Is<krys::boo::HTMLElement>(target)
           && Downcast<krys::boo::HTMLElement>(target).IsHTMLScriptElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLScriptElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
