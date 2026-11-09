/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class StyleSheetContents;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::style
{
  class RuleSet;

  /// @brief The user agent style sheets and the rule sets built from them, shared by every document. Sheets
  /// for SVG, MathML and some HTML features are added when the first element that needs them is styled.
  class UserAgentStyle
  {
  public:
    static RuleSet *DefaultStyle;
    static RuleSet *DefaultQuirksStyle;
    static RuleSet *DefaultPrintStyle;
    static uint32 DefaultStyleVersion;

    static css::StyleSheetContents *DefaultStyleSheet;
    static css::StyleSheetContents *QuirksStyleSheet;
    static css::StyleSheetContents *SVGStyleSheet;
    static css::StyleSheetContents *MathMLStyleSheet;
    static css::StyleSheetContents *MathMLCoreExtrasStyleSheet;
    static css::StyleSheetContents *MathMLFontSizeMathStyleSheet;
    static css::StyleSheetContents *MediaQueryStyleSheet;
    static css::StyleSheetContents *HorizontalFormControlsStyleSheet;
    static css::StyleSheetContents *PopoverStyleSheet;
    static css::StyleSheetContents *CounterStylesStyleSheet;

    static void InitDefaultStyleSheet() noexcept;
    static void EnsureDefaultStyleSheetsForElement(const dom::Element &element) noexcept;

  private:
    static void AddToDefaultStyle(css::StyleSheetContents &sheet) noexcept;
  };
}
