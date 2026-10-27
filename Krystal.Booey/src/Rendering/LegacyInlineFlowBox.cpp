#include "Krystal.Booey/Rendering/LegacyInlineFlowBox.hpp"

#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Style/Values/Borders/Shadow.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo
{
  namespace
  {
    void SetHasTextDescendantsOnAncestors(LegacyInlineFlowBox *box) noexcept
    {
      while (box && !box->HasTextDescendants())
      {
        box->SetHasTextDescendants();
        box = box->Parent();
      }
    }
  }

  void LegacyInlineFlowBox::AddToLine(LegacyInlineBox *child) noexcept
  {
    krys_debug_assert(!child->Parent());
    krys_debug_assert(!child->NextOnLine());
    krys_debug_assert(!child->PreviousOnLine());

    child->SetParent(this);
    if (!_firstChild)
    {
      _firstChild = child;
      _lastChild = child;
    }
    else
    {
      _lastChild->SetNextOnLine(child);
      child->SetPreviousOnLine(_lastChild);
      _lastChild = child;
    }
    child->SetIsFirstLine(IsFirstLine());
    child->SetIsHorizontal(IsHorizontal());
    if (child->IsInlineTextBox())
    {
      if (child->Renderer().Parent() == &Renderer())
      {
        _hasTextChildren = true;
      }
      SetHasTextDescendantsOnAncestors(this);
    }
    else if (auto *blockFlow = DynamicDowncast<LegacyInlineFlowBox>(*child))
    {
      if (blockFlow->HasTextDescendants())
      {
        SetHasTextDescendantsOnAncestors(this);
      }
    }

    const RenderStyle &childStyle = child->LineStyle();
    if (child->IsInlineTextBox())
    {
      bool hasMarkers = false;
      if (auto *textBox = DynamicDowncast<LegacyInlineTextBox>(*child))
      {
        hasMarkers = textBox->HasMarkers();
      }
      if (childStyle.UsedLetterSpacing() < 0 || childStyle.HasTextShadow()
          || !childStyle.GetTextEmphasisStyle().IsNone() || childStyle.HasPositiveStrokeWidth() || hasMarkers
          || !childStyle.GetTextUnderlineOffset().IsAuto()
          || !childStyle.GetTextDecorationThickness().IsAuto()
          || !childStyle.GetTextUnderlinePosition().IsAuto())
      {
        child->ClearKnownToHaveNoOverflow();
      }
    }
    else if (child->BoxModelObject()->HasSelfPaintingLayer())
    {
      child->ClearKnownToHaveNoOverflow();
    }
    else if (childStyle.HasOutlineInVisualOverflow())
    {
      child->ClearKnownToHaveNoOverflow();
    }

    if (LineStyle().HasOutlineInVisualOverflow())
    {
      ClearKnownToHaveNoOverflow();
    }

    if (KnownToHaveNoOverflow())
    {
      auto *flowBox = DynamicDowncast<LegacyInlineFlowBox>(*child);
      if (flowBox && !flowBox->KnownToHaveNoOverflow())
      {
        ClearKnownToHaveNoOverflow();
      }
    }

    if (auto *renderInline = DynamicDowncast<RenderInline>(child->Renderer());
        renderInline && renderInline->HasSelfPaintingLayer())
    {
      _hasSelfPaintInlineBox = true;
    }
  }

  void LegacyInlineFlowBox::RemoveChild(LegacyInlineBox *child) noexcept
  {
    if (!IsDirty())
    {
      DirtyLineBoxes();
    }

    if (child == _firstChild)
    {
      _firstChild = child->NextOnLine();
    }
    if (child == _lastChild)
    {
      _lastChild = child->PreviousOnLine();
    }
    if (child->NextOnLine())
    {
      child->NextOnLine()->SetPreviousOnLine(child->PreviousOnLine());
    }
    if (child->PreviousOnLine())
    {
      child->PreviousOnLine()->SetNextOnLine(child->NextOnLine());
    }

    child->SetParent(nullptr);
  }

  void LegacyInlineFlowBox::DeleteLine() noexcept
  {
    LegacyInlineBox *child = FirstChild();
    LegacyInlineBox *next = nullptr;
    while (child)
    {
      krys_debug_assert(this == child->Parent());
      next = child->NextOnLine();
      child->SetParent(nullptr);
      child->DeleteLine();
      child = next;
    }
    _firstChild = nullptr;
    _lastChild = nullptr;

    RemoveLineBoxFromRenderObject();
    delete this;
  }

  void LegacyInlineFlowBox::RemoveLineBoxFromRenderObject() noexcept
  {
    Downcast<RenderInline>(Renderer()).LegacyLineBoxes().RemoveLineBox(this);
  }

  void LegacyInlineFlowBox::AdjustPosition(float dx, float dy) noexcept
  {
    LegacyInlineBox::AdjustPosition(dx, dy);
    for (auto *child = FirstChild(); child; child = child->NextOnLine())
    {
      child->AdjustPosition(dx, dy);
    }
    if (_overflow)
    {
      _overflow->Move(LayoutUnit(dx), LayoutUnit(dy));
    }
  }

  inline void LegacyInlineFlowBox::AddTextBoxVisualOverflow(LegacyInlineTextBox &textBox,
                                                            GlyphOverflowAndFallbackFontsMap &textBoxDataMap,
                                                            LayoutRect &logicalVisualOverflow) noexcept
  {
    if (textBox.KnownToHaveNoOverflow())
    {
      return;
    }

    const auto &lineStyle = LineStyle();
    auto writingMode = lineStyle.GetWritingMode();

    auto it = textBoxDataMap.find(&textBox);
    GlyphOverflow *glyphOverflow = it == textBoxDataMap.end() ? nullptr : &it->second.second;
    bool isFlippedLine = writingMode.IsLineInverted();

    auto topGlyphEdge =
      glyphOverflow ? (isFlippedLine ? glyphOverflow->Bottom : glyphOverflow->Top) : LayoutUnit();
    auto bottomGlyphEdge =
      glyphOverflow ? (isFlippedLine ? glyphOverflow->Top : glyphOverflow->Bottom) : LayoutUnit();
    auto leftGlyphEdge = glyphOverflow ? glyphOverflow->Left : LayoutUnit();
    auto rightGlyphEdge = glyphOverflow ? glyphOverflow->Right : LayoutUnit();

    auto viewportSize = textBox.Renderer().View().FrameView().Size();
    LayoutUnit strokeOverflow(std::ceil(lineStyle.UsedStrokeWidth(viewportSize) / 2.0f));
    auto topGlyphOverflow = -strokeOverflow - topGlyphEdge;
    auto bottomGlyphOverflow = strokeOverflow + bottomGlyphEdge;
    auto leftGlyphOverflow = -strokeOverflow - leftGlyphEdge;
    auto rightGlyphOverflow = strokeOverflow + rightGlyphEdge;

    // If letter-spacing is negative, we should factor that into right layout overflow. (Even in RTL,
    // letter-spacing is applied to the right, so this is not an issue with left overflow.
    rightGlyphOverflow -= std::min(0, static_cast<int>(lineStyle.GetFontCascade().LetterSpacing()));

    auto [textShadowLogicalTop, textShadowLogicalBottom] = style::ShadowBlockDirectionExtent(
      lineStyle.GetTextShadow(), writingMode, lineStyle.UsedZoomForLength());

    auto childOverflowLogicalTop =
      std::min<LayoutUnit>(textShadowLogicalTop + topGlyphOverflow, topGlyphOverflow);
    auto childOverflowLogicalBottom =
      std::max<LayoutUnit>(textShadowLogicalBottom + bottomGlyphOverflow, bottomGlyphOverflow);

    auto [textShadowLogicalLeft, textShadowLogicalRight] = style::ShadowInlineDirectionExtent(
      lineStyle.GetTextShadow(), writingMode, lineStyle.UsedZoomForLength());

    auto childOverflowLogicalLeft =
      std::min<LayoutUnit>(textShadowLogicalLeft + leftGlyphOverflow, leftGlyphOverflow);
    auto childOverflowLogicalRight =
      std::max<LayoutUnit>(textShadowLogicalRight + rightGlyphOverflow, rightGlyphOverflow);

    auto logicalTopVisualOverflow =
      std::min(LayoutUnit(textBox.LogicalTop()) + childOverflowLogicalTop, logicalVisualOverflow.Y());
    auto logicalBottomVisualOverflow = std::max(
      LayoutUnit(textBox.LogicalBottom()) + childOverflowLogicalBottom, logicalVisualOverflow.MaxY());
    auto logicalLeftVisualOverflow =
      std::min(LayoutUnit(textBox.LogicalLeft()) + childOverflowLogicalLeft, logicalVisualOverflow.X());
    auto logicalRightVisualOverflow =
      std::max(LayoutUnit(textBox.LogicalRight()) + childOverflowLogicalRight, logicalVisualOverflow.MaxX());

    logicalVisualOverflow = LayoutRect(logicalLeftVisualOverflow, logicalTopVisualOverflow,
                                       logicalRightVisualOverflow - logicalLeftVisualOverflow,
                                       logicalBottomVisualOverflow - logicalTopVisualOverflow);

    textBox.SetLogicalOverflowRect(logicalVisualOverflow);
  }

  void LegacyInlineFlowBox::ComputeOverflow(LayoutUnit lineTop, LayoutUnit lineBottom,
                                            GlyphOverflowAndFallbackFontsMap &textBoxDataMap) noexcept
  {
    // If we know we have no overflow, we can just bail.
    if (KnownToHaveNoOverflow())
    {
      return;
    }

    _overflow = nullptr;
    // Visual overflow just includes overflow for stuff we need to repaint ourselves. Self-painting layers are
    // ignored. Layout overflow is used to determine scrolling extent, so it still includes child layers and
    // also factors in transforms, relative positioning, etc.
    LayoutRect logicalVisualOverflow(
      EnclosingLayoutRect(LogicalFrameRectIncludingLineHeight(lineTop, lineBottom)));

    for (auto *child = FirstChild(); child; child = child->NextOnLine())
    {
      if (Is<RenderText>(child->Renderer()))
      {
        auto &textBox = Downcast<LegacyInlineTextBox>(*child);
        LayoutRect textBoxOverflow(EnclosingLayoutRect(textBox.LogicalFrameRect()));
        AddTextBoxVisualOverflow(textBox, textBoxDataMap, textBoxOverflow);
        logicalVisualOverflow.Unite(textBoxOverflow);
      }
      else if (Is<RenderInline>(child->Renderer()))
      {
        auto &flow = Downcast<LegacyInlineFlowBox>(*child);
        flow.ComputeOverflow(lineTop, lineBottom, textBoxDataMap);
        if (!flow.Renderer().HasSelfPaintingLayer())
        {
          logicalVisualOverflow.Unite(flow.LogicalVisualOverflowRect(lineTop, lineBottom));
        }
      }
    }
    SetOverflowFromLogicalRects(logicalVisualOverflow, lineTop, lineBottom);
  }

  void LegacyInlineFlowBox::SetVisualOverflow(const LayoutRect &rect, LayoutUnit lineTop,
                                              LayoutUnit lineBottom) noexcept
  {
    LayoutRect frameBox = EnclosingLayoutRect(FrameRectIncludingLineHeight(lineTop, lineBottom));
    if (frameBox.Contains(rect) || rect.IsEmpty())
    {
      return;
    }

    if (!_overflow)
    {
      _overflow = CreateUnique<RenderOverflow>(frameBox, frameBox);
    }

    _overflow->SetVisualOverflow(rect);
  }

  void LegacyInlineFlowBox::SetOverflowFromLogicalRects(const LayoutRect &logicalVisualOverflow,
                                                        LayoutUnit lineTop, LayoutUnit lineBottom) noexcept
  {
    LayoutRect visualOverflow(IsHorizontal() ? logicalVisualOverflow
                                             : logicalVisualOverflow.TransposedRect());
    SetVisualOverflow(visualOverflow, lineTop, lineBottom);
  }

  LegacyInlineBox *LegacyInlineFlowBox::FirstLeafDescendant() const noexcept
  {
    LegacyInlineBox *leaf = nullptr;
    for (auto *child = FirstChild(); child && !leaf; child = child->NextOnLine())
    {
      leaf = child->IsLeaf() ? child : Downcast<LegacyInlineFlowBox>(*child).FirstLeafDescendant();
    }
    return leaf;
  }

  LegacyInlineBox *LegacyInlineFlowBox::LastLeafDescendant() const noexcept
  {
    LegacyInlineBox *leaf = nullptr;
    for (auto *child = LastChild(); child && !leaf; child = child->PreviousOnLine())
    {
      leaf = child->IsLeaf() ? child : Downcast<LegacyInlineFlowBox>(*child).LastLeafDescendant();
    }
    return leaf;
  }

  RenderObject::HighlightState LegacyInlineFlowBox::SelectionState() const noexcept
  {
    return RenderObject::HighlightState::None;
  }
}
