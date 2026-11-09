/**
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

#include "Krystal.Booey/CSS/StyleSheets/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheet.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Core/Assert.hpp"

namespace krys::boo::css
{
  StyleSheetList::StyleSheetList(dom::Document &document) noexcept : _document(CreateWeakPtr(&document))
  {
  }

  StyleSheetList::StyleSheetList(dom::ShadowRoot &shadowRoot) noexcept
      : _shadowRoot(CreateWeakPtr(&shadowRoot))
  {
  }

  StyleSheetList::~StyleSheetList() noexcept = default;

  Ref<StyleSheetList> StyleSheetList::Create(dom::Document &document) noexcept
  {
    return AdoptRef(*new StyleSheetList(document));
  }

  Ref<StyleSheetList> StyleSheetList::Create(dom::ShadowRoot &shadowRoot) noexcept
  {
    return AdoptRef(*new StyleSheetList(shadowRoot));
  }

  const List<RefPtr<StyleSheet>> &StyleSheetList::StyleSheets() const noexcept
  {
    if (auto *document = _document.get())
    {
      return document->StyleScope().StyleSheetsForStyleSheetList();
    }
    if (auto *shadowRoot = _shadowRoot.get())
    {
      return shadowRoot->StyleScope().StyleSheetsForStyleSheetList();
    }
    return _detachedStyleSheets;
  }

  dom::Node *StyleSheetList::OwnerNode() const noexcept
  {
    if (auto *document = _document.get())
    {
      return document;
    }
    return _shadowRoot.get();
  }

  void StyleSheetList::Detach() noexcept
  {
    if (auto *document = _document.get())
    {
      krys_debug_assert(!_shadowRoot);
      _detachedStyleSheets = document->StyleScope().StyleSheetsForStyleSheetList();
      _document = nullptr;
    }
    else if (auto *shadowRoot = _shadowRoot.get())
    {
      krys_debug_assert(!_document);
      _detachedStyleSheets = shadowRoot->StyleScope().StyleSheetsForStyleSheetList();
      _shadowRoot = nullptr;
    }
    else
    {
      krys_unreachable();
    }
  }

  uint32 StyleSheetList::Length() const noexcept
  {
    return static_cast<uint32>(StyleSheets().size());
  }

  StyleSheet *StyleSheetList::Item(uint32 index) noexcept
  {
    auto &sheets = StyleSheets();
    return index < sheets.size() ? sheets[index].get() : nullptr;
  }
}
