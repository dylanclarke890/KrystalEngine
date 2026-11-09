/**
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 * Copyright (C) 2003, 2004, 2006, 2007, 2008, 2009, 2010, 2011 Apple Inc. All right reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2013 ChangSeok Oh <shivamidow@gmail.com>
 * Copyright (C) 2013 Adobe Systems Inc. All right reserved.
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

#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyInlineIterator.hpp"
#include "Krystal.Booey/Rendering/Line/LineInfo.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo
{
  enum class WhitespacePosition : bool
  {
    LeadingWhitespace,
    TrailingWhitespace
  };

  KRYS_NODISCARD inline const RenderStyle &LineStyle(const RenderObject &renderer,
                                                     const LineInfo &lineInfo) noexcept
  {
    return lineInfo.IsFirstLine() ? renderer.FirstLineStyle() : renderer.Style();
  }

  KRYS_NODISCARD inline bool RequiresLineBoxForContent(const RenderInline &flow,
                                                       const LineInfo &lineInfo) noexcept
  {
    RenderElement *parent = flow.Parent();
    if (flow.GetDocument().InNoQuirksMode())
    {
      const RenderStyle &flowStyle = LineStyle(flow, lineInfo);
      const RenderStyle &parentStyle = LineStyle(*parent, lineInfo);
      if (flowStyle.GetLineHeight() != parentStyle.GetLineHeight()
          || flowStyle.GetVerticalAlign() != parentStyle.GetVerticalAlign()
          || !parentStyle.MetricsOfPrimaryFont().HasIdenticalAscentDescentAndLineGap(
            flowStyle.MetricsOfPrimaryFont()))
      {
        return true;
      }
    }
    return false;
  }

  KRYS_NODISCARD inline bool ShouldCollapseWhiteSpace(const RenderStyle *style, const LineInfo &lineInfo,
                                                      WhitespacePosition whitespacePosition) noexcept
  {
    // CSS2 16.6.1
    // If a space (U+0020) at the beginning of a line has 'white-space' set to 'normal', 'nowrap', or
    // 'pre-line', it is removed. If a space (U+0020) at the end of a line has 'white-space' set to 'normal',
    // 'nowrap', or 'pre-line', it is also removed. If spaces (U+0020) or tabs (U+0009) at the end of a line
    // have 'white-space' set to 'pre-wrap', UAs may visually collapse them.
    return style->CollapseWhiteSpace()
           || (whitespacePosition == WhitespacePosition::TrailingWhitespace
               && style->GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve
               && style->GetTextWrapMode() == TextWrapMode::Wrap && !lineInfo.IsEmpty());
  }

  KRYS_NODISCARD inline bool
    RequiresLineBox(const LegacyInlineIterator &it, const LineInfo &lineInfo = LineInfo(),
                    WhitespacePosition whitespacePosition = WhitespacePosition::LeadingWhitespace) noexcept
  {
    bool rendererIsEmptyInline = false;
    if (auto *inlineRenderer = DynamicDowncast<RenderInline>(*it.Renderer()))
    {
      if (!RequiresLineBoxForContent(*inlineRenderer, lineInfo))
      {
        return false;
      }
      rendererIsEmptyInline = IsEmptyInline(*inlineRenderer);
    }

    if (!ShouldCollapseWhiteSpace(&it.Renderer()->Style(), lineInfo, whitespacePosition))
    {
      return true;
    }

    char32 current = it.Current();
    auto preservesNewline =
      !it.Renderer()->IsRenderSVGInlineText() && it.Renderer()->Style().PreserveNewline();
    bool notJustWhitespace = current != U' ' && current != U'\t' && current != text::SoftHyphen
                             && (current != U'\n' || preservesNewline);
    return notJustWhitespace || rendererIsEmptyInline;
  }
}
