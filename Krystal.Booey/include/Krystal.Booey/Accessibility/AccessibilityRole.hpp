/*
 * Copyright (C) 2025 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"

// Roles for frames, remote frames and models are left out: Booey has no subframes or model elements.
// clang-format off
#define KRYS_AX_ROLES(macro) \
  macro(Application) \
  macro(ApplicationAlert) \
  macro(ApplicationAlertDialog) \
  macro(ApplicationDialog) \
  macro(ApplicationLog) \
  macro(ApplicationMarquee) \
  macro(ApplicationStatus) \
  macro(ApplicationTimer) \
  macro(Audio) \
  macro(Blockquote) \
  macro(Button) \
  macro(Canvas) \
  macro(Caption) \
  macro(Cell) \
  macro(Checkbox) \
  macro(Code) \
  macro(ColorWell) \
  macro(Column) \
  macro(ColumnHeader) \
  macro(ComboBox) \
  macro(DateTime) \
  macro(Definition) \
  macro(Deletion) \
  macro(DescriptionList) \
  macro(DescriptionListDetail) \
  macro(DescriptionListTerm) \
  macro(Details) \
  macro(Directory) \
  macro(Document) \
  macro(DocumentArticle) \
  macro(DocumentMath) \
  macro(DocumentNote) \
  macro(Emphasis) \
  macro(Feed) \
  macro(Figure) \
  macro(Footnote) \
  macro(Form) \
  macro(Generic) \
  macro(GraphicsDocument) \
  macro(GraphicsObject) \
  macro(GraphicsSymbol) \
  macro(Grid) \
  macro(GridCell) \
  macro(Group) \
  macro(Heading) \
  macro(HorizontalRule) \
  macro(Ignored) \
  macro(Inline) \
  macro(Image) \
  macro(ImageMap) \
  macro(Insertion) \
  macro(Label) \
  macro(LandmarkBanner) \
  macro(LandmarkComplementary) \
  macro(LandmarkContentInfo) \
  macro(LandmarkDocRegion) \
  macro(LandmarkMain) \
  macro(LandmarkNavigation) \
  macro(LandmarkRegion) \
  macro(LandmarkSearch) \
  macro(Legend) \
  macro(LineBreak) \
  macro(Link) \
  macro(List) \
  macro(ListBox) \
  macro(ListBoxOption) \
  macro(ListItem) \
  macro(ListMarker) \
  macro(Mark) \
  macro(MathElement) \
  macro(Menu) \
  macro(MenuBar) \
  macro(MenuItem) \
  macro(MenuItemCheckbox) \
  macro(MenuItemRadio) \
  macro(MenuListPopup) \
  macro(MenuListOption) \
  macro(Meter) \
  macro(Paragraph) \
  macro(PopUpButton) \
  macro(Pre) \
  macro(Presentational) \
  macro(ProgressIndicator) \
  macro(RadioButton) \
  macro(RadioGroup) \
  macro(RowHeader) \
  macro(Row) \
  macro(RowGroup) \
  macro(RubyInline) \
  macro(RubyText) \
  macro(ScrollArea) \
  macro(ScrollBar) \
  macro(SearchField) \
  macro(SectionFooter) \
  macro(SectionHeader) \
  macro(Slider) \
  macro(SliderThumb) \
  macro(SpinButton) \
  macro(SpinButtonPart) \
  macro(Splitter) \
  macro(StaticText) \
  macro(Strong) \
  macro(Subscript) \
  macro(Suggestion) \
  macro(Summary) \
  macro(Superscript) \
  macro(Switch) \
  macro(SVGRoot) \
  macro(SVGText) \
  macro(SVGTSpan) \
  macro(SVGTextPath) \
  macro(TabGroup) \
  macro(TabList) \
  macro(TabPanel) \
  macro(Tab) \
  macro(Table) \
  macro(TableHeaderContainer) \
  macro(Term) \
  macro(TextArea) \
  macro(TextField) \
  macro(TextGroup) \
  macro(Time) \
  macro(Tree) \
  macro(TreeGrid) \
  macro(TreeItem) \
  macro(ToggleButton) \
  macro(Toolbar) \
  macro(Unknown) \
  macro(UserInterfaceTooltip) \
  macro(Video) \
  macro(WebApplication) \
  macro(WebArea)
// clang-format on

namespace krys::boo::ax
{
  /// @brief An accessibility object's role: what kind of thing it is to assistive technology.
  enum class AccessibilityRole : uint8
  {
#define KRYS_AX_DEFINE_ROLE(name) name,
    KRYS_AX_ROLES(KRYS_AX_DEFINE_ROLE)
#undef KRYS_AX_DEFINE_ROLE
  };

  /// @brief @p role's enumerator name, as accessibility tree dumps show it.
  KRYS_NODISCARD utf8_stringview RoleToString(AccessibilityRole role) noexcept;
}
