/**
 * Copyright (C) 2006, 2007 Apple Inc. All rights reserved
 * Copyright (C) 2008 Torch Mobile Inc. All rights reserved. (http://www.torchmobile.com/)
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

#include "Krystal.Booey/Rendering/FormControls/RenderTextControlMultiLine.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTextAreaElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  RenderTextControlMultiLine::RenderTextControlMultiLine(html::HTMLTextAreaElement &element,
                                                         RenderStyle &&style) noexcept
      : RenderTextControl(Type::TextControlMultiLine, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTextControlMultiLine());
  }

  RenderTextControlMultiLine::~RenderTextControlMultiLine() noexcept = default;

  html::HTMLTextAreaElement &RenderTextControlMultiLine::TextAreaElement() const noexcept
  {
    return Downcast<html::HTMLTextAreaElement>(RenderTextControl::TextFormControlElement());
  }

  bool RenderTextControlMultiLine::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                               const HitTestLocation &locationInContainer,
                                               const LayoutPoint &accumulatedOffset,
                                               HitTestAction hitTestAction) noexcept
  {
    if (!RenderTextControl::NodeAtPoint(request, result, locationInContainer, accumulatedOffset,
                                        hitTestAction))
    {
      return false;
    }

    const LayoutPoint adjustedPoint(accumulatedOffset + Location());
    if (IsPointInOverflowControl(result, locationInContainer.Point(), adjustedPoint))
    {
      return true;
    }

    if (result.InnerNode() == &TextAreaElement() || result.InnerNode() == InnerTextElement().get())
    {
      HitInnerTextElement(result, locationInContainer.Point(), accumulatedOffset);
    }

    return true;
  }

  float RenderTextControlMultiLine::GetAverageCharWidth() noexcept
  {
    // Since Lucida Grande is the default font, we want this to match the width
    // of Courier New, the default font for textareas in IE, Firefox and Safari Win.
    // 1229 is the avgCharWidth value in the OS/2 table for Courier New.
    if (Style().GetFontCascade().FirstFamily() == StringAtom(u8"Lucida Grande"))
    {
      return ScaleEmToUnits(1'229);
    }

    return RenderTextControl::GetAverageCharWidth();
  }

  LayoutUnit RenderTextControlMultiLine::PreferredContentLogicalWidth(float charWidth) const noexcept
  {
    float width = std::ceilf(charWidth * TextAreaElement().Cols());

    auto overflow = GetWritingMode().IsHorizontal() ? Style().OverflowY() : Style().OverflowX();

    // We are able to have a vertical scrollbar if the overflow style is scroll or auto
    if ((overflow == Overflow::Scroll) || (overflow == Overflow::Auto))
    {
      width += ScrollbarThickness();
    }

    return LayoutUnit(width);
  }

  LayoutUnit
    RenderTextControlMultiLine::ComputeControlLogicalHeight(LayoutUnit lineHeight,
                                                            LayoutUnit nonContentHeight) const noexcept
  {
    return lineHeight * static_cast<int32>(TextAreaElement().Rows()) + nonContentHeight;
  }

  void RenderTextControlMultiLine::LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept
  {
    RenderTextControl::LayoutExcludedChildren(relayoutChildren);
    html::HTMLElement *placeholder = TextFormControlElement().PlaceholderElement();
    RenderElement *placeholderRenderer = placeholder ? placeholder->Renderer() : nullptr;
    if (!placeholderRenderer)
    {
      return;
    }
    if (auto *placeholderBox = DynamicDowncast<RenderBox>(placeholderRenderer))
    {
      placeholderBox->MutableStyle().SetLogicalWidth(style::PreferredSize::Fixed {
        (ContentBoxLogicalWidth() - placeholderBox->BorderAndPaddingLogicalWidth()).ToFloat()});
      placeholderBox->LayoutIfNeeded();
      placeholderBox->SetX(BorderLeft() + PaddingLeft());
      placeholderBox->SetY(BorderTop() + PaddingTop());
    }
  }
}
