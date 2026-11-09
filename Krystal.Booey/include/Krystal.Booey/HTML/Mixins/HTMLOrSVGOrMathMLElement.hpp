/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/FocusOptions.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html::mixins
{
  /// @brief The members HTML, SVG and MathML elements share: focus and the attributes behind it.
  /// @see https://html.spec.whatwg.org/#htmlorsvgormathmlelement
  /// @note `dataset` is `Element::Dataset()`.
  class HTMLOrSVGOrMathMLElement
  {
  public:
    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD static int32 TabIndex(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    static void TabIndex(dom::Element &element, int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD static bool Autofocus(const dom::Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    static void Autofocus(dom::Element &element, bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-focus
    static void Focus(dom::Element &element, const dom::FocusOptions &options = {}) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-blur
    static void Blur(dom::Element &element) noexcept;
  };
}
