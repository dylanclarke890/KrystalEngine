/*
 * Copyright (C) 2006, 2007 Rob Buis
 * Copyright (C) 2013 Apple, Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  class CSSStyleSheet;
}

namespace krys::boo::style
{
  class Scope;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::html
{

  /// @brief The style sheet of an element whose sheet is its own text (a `style` element): creates the sheet
  /// from the element's child text and keeps it, and the element's place among its scope's style sheet
  /// candidates, up to date.
  /// @note The element's children are still being parsed while `dom::Element::IsFinishedParsingChildren()` is
  /// false.
  /// @see https://html.spec.whatwg.org/#update-a-style-block
  class InlineStyleSheetOwner
  {
    KRYS_NON_COPY_MOVABLE(InlineStyleSheetOwner);

  private:
    bool _loading {false};
    dom::DOMStringAtom _contentType {dom::DOMStringAtom::Null()};
    dom::DOMStringAtom _media {dom::DOMStringAtom::Null()};
    RefPtr<css::CSSStyleSheet> _sheet;
    WeakPtr<style::Scope> _styleScope;

  public:
    InlineStyleSheetOwner() noexcept;
    ~InlineStyleSheetOwner() noexcept;

    void SetContentType(dom::DOMStringAtom contentType) noexcept
    {
      _contentType = contentType;
    }

    void SetMedia(dom::DOMStringAtom media) noexcept
    {
      _media = media;
    }

    KRYS_NODISCARD css::CSSStyleSheet *Sheet() const noexcept;

    KRYS_NODISCARD bool IsLoading() const noexcept;
    bool SheetLoaded(dom::Element &element) noexcept;
    void StartLoadingDynamicSheet(dom::Element &element) noexcept;

    void InsertedIntoDocument(dom::Element &element) noexcept;
    void RemovedFromDocument(dom::Element &element) noexcept;
    void ClearDocumentData(dom::Element &element) noexcept;
    void ChildrenChanged(dom::Element &element) noexcept;
    void FinishParsingChildren(dom::Element &element) noexcept;

    KRYS_NODISCARD style::Scope *StyleScope() noexcept;

  private:
    void CreateSheet(dom::Element &element, const dom::DOMString &text) noexcept;
    void CreateSheetFromTextContents(dom::Element &element) noexcept;
    void ClearSheet() noexcept;
  };
}
