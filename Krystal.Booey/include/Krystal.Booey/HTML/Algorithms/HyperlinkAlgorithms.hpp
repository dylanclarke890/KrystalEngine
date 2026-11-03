#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Element;
  class Event;
}

namespace krys::boo::url
{
  class URL;
}

namespace krys::boo::html
{
  class HTMLElement;

  /// @brief What `<a>` and `<area>` share as hyperlinks: their URL, their target and following them.
  /// @see https://html.spec.whatwg.org/#links-created-by-a-and-area-elements
  class HyperlinkAlgorithms
  {
  public:
    /// @brief @p element's `href` attribute, completed against its node document's base URL; null if it has
    /// none.
    KRYS_NODISCARD static url::URL Href(const HTMLElement &element) noexcept;

    /// @brief Sets @p element's `href` attribute to @p url.
    static void SetHref(HTMLElement &element, const url::URL &url) noexcept;

    /// @brief The browsing context @p element's link navigates: its `target`, or the document's base target,
    /// with `_blank` for a target with dangling markup.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD static dom::DOMStringAtom EffectiveTarget(const HTMLElement &element) noexcept;

    /// @brief Whether @p event is the keydown of the Enter key.
    KRYS_NODISCARD static bool IsEnterKeyKeydownEvent(const dom::Event &event) noexcept;

    /// @brief Whether @p element's `rel` attribute has the keyword @p relation (ASCII case-insensitively).
    KRYS_NODISCARD static bool HasRel(const HTMLElement &element, dom::DOMStringView relation) noexcept;

    /// @brief The referrer policy @p subject's link is followed with: `no-referrer` for `rel=noreferrer`.
    /// @see https://html.spec.whatwg.org/#hyperlink-referrer-policy
    KRYS_NODISCARD static ReferrerPolicy HyperlinkReferrerPolicy(const HTMLElement &subject) noexcept;

    /// @brief Whether a window @p element's link (or form) opens to @p url in @p target gets no opener.
    /// @see https://html.spec.whatwg.org/#get-an-element's-noopener
    KRYS_NODISCARD static bool GetNoopener(const HTMLElement &element, const url::URL &url,
                                           dom::DOMStringView target) noexcept;

    /// @brief The activation behaviour of `<a>` and `<area>`: follows the link, with the coordinates clicked
    /// on a server-side image map.
    /// @see https://html.spec.whatwg.org/#the-a-element:activation-behaviour
    static void ActivationBehavior(HTMLElement &element, dom::Event &event) noexcept;

    /// @brief Follows @p subject's link, as @p event (a click, or the Enter key's simulated click) asks: its
    /// frame's loader navigates to its URL, followed by @p hyperlinkSuffix, in its target.
    /// @see https://html.spec.whatwg.org/#following-hyperlinks-2
    static void FollowTheHyperlink(HTMLElement &subject, dom::Event &event,
                                   const Maybe<utf8_string> &hyperlinkSuffix = null) noexcept;
  };
}
