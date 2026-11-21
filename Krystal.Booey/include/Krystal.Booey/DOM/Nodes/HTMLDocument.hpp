/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 * Copyright (C) 2004, 2006, 2007, 2008, 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo
{
  class QualifiedName;

  /// @brief A document whose type is "html", with the content type text/html.
  /// @see https://dom.spec.whatwg.org/#html-document
  class HTMLDocument : public Document
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLDocument);

  public:
    HTMLDocument() noexcept : Document(dom::DocumentFlags::IsHTMLDocument)
    {
    }

    explicit HTMLDocument(dom::DocumentFlags flags) noexcept
        : Document(flags | dom::DocumentFlags::IsHTMLDocument)
    {
    }

    /// @brief Whether attribute selectors match the values of @p attributeName case-sensitively on HTML
    /// elements in HTML documents.
    /// @see https://html.spec.whatwg.org/#case-sensitivity-of-selectors
    KRYS_NODISCARD static bool IsCaseSensitiveAttribute(const QualifiedName &attributeName) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLDocument)
  KRYS_NODISCARD static bool IsType(const krys::boo::Document &document) noexcept
  {
    return document.IsHTMLDocument();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
