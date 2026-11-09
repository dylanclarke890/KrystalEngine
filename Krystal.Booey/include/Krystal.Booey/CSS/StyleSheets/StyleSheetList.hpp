/*
 * (C) 1999-2003 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
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
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
  class ShadowRoot;
}

namespace krys::boo::css
{
  class StyleSheet;

  /// @brief The style sheets of a document or shadow root, in order. Live until its document or shadow root
  /// goes away; after that it keeps the sheets it had.
  /// @see https://drafts.csswg.org/cssom/#the-stylesheetlist-interface
  class StyleSheetList final : public RefCounted<StyleSheetList>
  {
    KRYS_NON_COPY_MOVABLE(StyleSheetList);

  private:
    WeakPtr<dom::Document> _document;
    WeakPtr<dom::ShadowRoot> _shadowRoot;
    List<RefPtr<StyleSheet>> _detachedStyleSheets;

    explicit StyleSheetList(dom::Document &document) noexcept;
    explicit StyleSheetList(dom::ShadowRoot &shadowRoot) noexcept;

  public:
    ~StyleSheetList() noexcept;

    KRYS_NODISCARD static Ref<StyleSheetList> Create(dom::Document &document) noexcept;
    KRYS_NODISCARD static Ref<StyleSheetList> Create(dom::ShadowRoot &shadowRoot) noexcept;

#pragma region StyleSheetList - https://drafts.csswg.org/cssom/#the-stylesheetlist-interface

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-item
    KRYS_NODISCARD StyleSheet *Item(uint32 index) noexcept;

    /// @see https://drafts.csswg.org/cssom/#dom-stylesheetlist-length
    KRYS_NODISCARD uint32 Length() const noexcept;

#pragma endregion

    KRYS_NODISCARD bool IsSupportedPropertyIndex(uint32 index) const noexcept
    {
      return index < Length();
    }

    /// @brief The document or shadow root whose sheets these are, or null once detached.
    KRYS_NODISCARD dom::Node *OwnerNode() const noexcept;

    /// @brief Keeps the current sheets and stops following the document or shadow root.
    void Detach() noexcept;

  private:
    KRYS_NODISCARD const List<RefPtr<StyleSheet>> &StyleSheets() const noexcept;
  };
}
