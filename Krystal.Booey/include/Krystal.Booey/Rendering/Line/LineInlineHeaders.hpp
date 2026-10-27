#pragma once

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"
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
