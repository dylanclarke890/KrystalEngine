/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Simon Hausmann <hausmann@kde.org>
 * Copyright (C) 2007-2016 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Mixins/UrlDecomposition.hpp"

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/#htmlanchorelement
  class HTMLAnchorElement : public HTMLElement, public html::UrlDecomposition
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLAnchorElement);

  private:
    UniquePtr<dom::DOMTokenList> _relList;
    /// @brief Whether the shift key was down when the mouse was last pressed on the link, for a drag it
    /// starts.
    bool _wasShiftKeyDownOnMouseDown {false};

  public:
    /// @brief Links start a selection only when editable.
    KRYS_NODISCARD bool CanStartSelection() const noexcept final;

    HTMLAnchorElement(Document &document) noexcept;

    ~HTMLAnchorElement() noexcept override;

    /// @brief Follows the link, if it is one.
    /// @see https://html.spec.whatwg.org/#the-a-element:activation-behaviour
    void ActivationBehavior(Event &event) noexcept override;

    /// @brief Follows a focused link on Enter (as a simulated click), and on a click of the middle button.
    void DefaultEventHandler(Event &event) noexcept override;

    /// @brief Whether the link is followed by a click with or without @p shiftKey: always outside editable
    /// content, and inside it as `Settings::EditableLinkBehavior` says.
    KRYS_NODISCARD bool TreatLinkAsLive(bool shiftKey) const noexcept;

    /// @brief Whether the element is a link that acts as one for the last mouse press on it.
    KRYS_NODISCARD bool IsLiveLink() const noexcept;

    /// @brief A link is interactive content.
    KRYS_NODISCARD bool IsInteractiveContent() const noexcept override;

#pragma region HTMLAnchorElement - https://html.spec.whatwg.org/#htmlanchorelement

    /// @see https://html.spec.whatwg.org/#dom-a-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-target
    void Target(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    KRYS_NODISCARD DOMString Download() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-download
    void Download(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    KRYS_NODISCARD dom::USVString Ping() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-ping
    void Ping(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rel
    void Rel(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-hreflang
    KRYS_NODISCARD DOMString HrefLang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-hreflang
    void HrefLang(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-type
    void Type(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    KRYS_NODISCARD DOMString Text() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-text
    ExceptionOr<void> Text(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    KRYS_NODISCARD DOMString ReferrerPolicy() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-referrerpolicy
    void ReferrerPolicy(DOMString value) noexcept;

#pragma endregion

#pragma region HTMLHyperlinkElementUtils - https://html.spec.whatwg.org/#htmlhyperlinkelementutils

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString Href() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    void Href(dom::USVString value) noexcept;

    /// @brief The `href` stringifier.
    /// @see https://html.spec.whatwg.org/#dom-hyperlink-href
    KRYS_NODISCARD dom::USVString ToString() const noexcept;

#pragma endregion

#pragma region HTMLAnchorElement Obsolete members - https://html.spec.whatwg.org/#HTMLAnchorElement-partial

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    KRYS_NODISCARD DOMString Coords() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-coords
    void Coords(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-charset
    void Charset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    KRYS_NODISCARD DOMString Rev() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-rev
    void Rev(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    KRYS_NODISCARD DOMString Shape() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-a-shape
    void Shape(DOMString value) noexcept;

#pragma endregion

  protected:
    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    KRYS_NODISCARD net::Url FullUrl() const noexcept final;

    void SetFullUrl(const net::Url &url) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLAnchorElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLAnchorElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLAnchorElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
