#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/Loader/FetchOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Element;
  class Event;
  class HTMLElement;
}

namespace krys::net
{
  class Url;
}

namespace krys::boo::html
{
  /// @brief What `<a>` and `<area>` share as hyperlinks: their URL, their target and following them.
  /// @see https://html.spec.whatwg.org/multipage/links.html#links-created-by-a-and-area-elements
  class HyperlinkAlgorithms
  {
  public:
    /// @brief @p element's `href` attribute, completed against its node document's base URL; null if it has
    /// none.
    KRYS_NODISCARD static net::Url Href(const HTMLElement &element) noexcept;

    /// @brief Sets @p element's `href` attribute to @p url.
    static void SetHref(HTMLElement &element, const net::Url &url) noexcept;

    /// @brief The browsing context @p element's link navigates: its `target`, or the document's base target,
    /// with `_blank` for a target with dangling markup.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD static DOMStringAtom EffectiveTarget(const HTMLElement &element) noexcept;

    /// @brief Whether @p event is the keydown of the Enter key.
    KRYS_NODISCARD static bool IsEnterKeyKeydownEvent(const Event &event) noexcept;

    /// @brief Whether @p element's `rel` attribute has the keyword @p relation (ASCII case-insensitively).
    KRYS_NODISCARD static bool HasRel(const HTMLElement &element, DOMStringView relation) noexcept;

    /// @brief The referrer policy @p subject's link is followed with: `no-referrer` for `rel=noreferrer`.
    /// @see https://html.spec.whatwg.org/multipage/links.html#hyperlink-referrer-policy
    KRYS_NODISCARD static ReferrerPolicy HyperlinkReferrerPolicy(const HTMLElement &subject) noexcept;

    /// @brief The activation behaviour of `<a>` and `<area>`: follows the link, with the coordinates clicked
    /// on a server-side image map.
    /// @see https://html.spec.whatwg.org/#the-a-element:activation-behaviour
    static void ActivationBehavior(HTMLElement &element, Event &event) noexcept;

    /// @brief Follows @p subject's link, as @p event (a click, or the Enter key's simulated click) asks: its
    /// frame's loader navigates to its URL, followed by @p hyperlinkSuffix, in its target.
    /// @see https://html.spec.whatwg.org/multipage/links.html#following-hyperlinks-2
    static void FollowTheHyperlink(HTMLElement &subject, Event &event,
                                   const Maybe<utf8_string> &hyperlinkSuffix = null) noexcept;
  };
}
