#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"

namespace krys::boo::html
{
  class HTMLPictureElement;

  /// @see https://html.spec.whatwg.org/#htmlsourceelement
  // FIXME: Media element sources (sourceWasAdded, error events), once there are media elements.
  class HTMLSourceElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLSourceElement);

  private:
    mutable Maybe<css::mq::MediaQueryList> _cachedParsedMediaAttribute;
    // The <picture> the source was inserted into, while it's its parent.
    HTMLPictureElement *_insertedIntoPicture {nullptr};
    // Whether the source precedes the picture's images, so changes to it can change their selected source.
    bool _shouldCallSourcesChanged {false};

  public:
    HTMLSourceElement(dom::Document &document) noexcept;

#pragma region HTMLSourceElement - https://html.spec.whatwg.org/#htmlsourceelement

    /// @see https://html.spec.whatwg.org/#dom-source-src
    KRYS_NODISCARD dom::USVString Src() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-src
    void Src(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-type
    void Type(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    KRYS_NODISCARD dom::USVString SrcSet() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-srcset
    void SrcSet(dom::USVString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    KRYS_NODISCARD dom::DOMString Sizes() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-sizes
    void Sizes(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    KRYS_NODISCARD dom::DOMString Media() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-media
    void Media(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    KRYS_NODISCARD uint32 Width() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-width
    void Width(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    KRYS_NODISCARD uint32 Height() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-source-height
    void Height(uint32 value) noexcept;

#pragma endregion

    /// @brief The `media` attribute, parsed.
    KRYS_NODISCARD const css::mq::MediaQueryList &
      ParsedMediaAttribute(dom::Document &document) const noexcept;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;
    void OnInsert() noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLSourceElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLSourceElement();
  }
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    auto *element = krys::DynamicDowncast<krys::boo::html::HTMLElement>(target);
    return element && element->IsHTMLSourceElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
