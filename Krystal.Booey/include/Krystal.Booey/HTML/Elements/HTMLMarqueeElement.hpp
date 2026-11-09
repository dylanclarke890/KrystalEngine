#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class WebAnimation;
}

namespace krys::boo::html
{
  class HTMLDivElement;

  /// @brief A marquee: its contents, in a mover in its shadow tree, scroll across it, bounce in it or slide
  /// into it, as a Web Animation of the mover's transform, one loop at a time, as Chrome moves them.
  /// @see https://html.spec.whatwg.org/#the-marquee-element
  class HTMLMarqueeElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLMarqueeElement);

  private:
    WeakPtr<HTMLDivElement> _mover;
    /// @brief The current loop's animation.
    RefPtr<WebAnimation> _player;
    /// @brief The animation frame request that starts the next loop; 0 for none.
    int32 _continueCallbackRequestId {0};
    /// @brief The marquee current loop index.
    int32 _currentLoopIndex {0};

  public:
    HTMLMarqueeElement(dom::Document &document) noexcept;
    ~HTMLMarqueeElement() noexcept override;
    KRYS_NODISCARD static Ref<HTMLMarqueeElement> Create(dom::Document &document) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-behavior
    KRYS_NODISCARD dom::DOMString Behavior() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-behavior
    void Behavior(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-bgcolor
    KRYS_NODISCARD dom::DOMString BgColor() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-bgcolor
    void BgColor(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-direction
    KRYS_NODISCARD dom::DOMString Direction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-direction
    void Direction(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-height
    KRYS_NODISCARD dom::DOMString Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-height
    void Height(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-width
    KRYS_NODISCARD dom::DOMString Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-width
    void Width(dom::DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-hspace
    KRYS_NODISCARD uint32 HSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-hspace
    void HSpace(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-vspace
    KRYS_NODISCARD uint32 VSpace() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-vspace
    void VSpace(uint32 value) noexcept;

    /// @brief The marquee loop count: -1 (the default) loops forever.
    /// @see https://html.spec.whatwg.org/#dom-marquee-loop
    KRYS_NODISCARD int32 Loop() const noexcept;

    /// @brief Sets the loop count, if @p value is positive or -1; other values are ignored.
    /// @see https://html.spec.whatwg.org/#dom-marquee-loop
    void Loop(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-scrollamount
    KRYS_NODISCARD uint32 ScrollAmount() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-scrollamount
    void ScrollAmount(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-scrolldelay
    KRYS_NODISCARD uint32 ScrollDelay() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-scrolldelay
    void ScrollDelay(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-truespeed
    KRYS_NODISCARD bool TrueSpeed() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-marquee-truespeed
    void TrueSpeed(bool value) noexcept;

    /// @brief Turns the marquee on: it moves again, from where it stopped.
    /// @see https://html.spec.whatwg.org/#dom-marquee-start
    void Start() noexcept;

    /// @brief Turns the marquee off: it stops where it is.
    /// @see https://html.spec.whatwg.org/#dom-marquee-stop
    void Stop() noexcept;

    /// @brief The marquee scroll interval, in milliseconds.
    /// @see https://html.spec.whatwg.org/#marquee-scroll-interval
    KRYS_NODISCARD uint32 ScrollInterval() const noexcept;

    /// @brief The shadow tree's element whose transform moves the contents.
    KRYS_NODISCARD HTMLDivElement *Mover() const noexcept;

  protected:
    void DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept override;
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

  private:
    /// @brief Plays the next loop, if the marquee loops again, or plays on a paused one.
    void ContinueAnimation() noexcept;
    /// @brief Whether the marquee has loops left: a slide loops once unless it says otherwise.
    KRYS_NODISCARD bool ShouldContinue() const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLMarqueeElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLMarqueeElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLMarqueeElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
