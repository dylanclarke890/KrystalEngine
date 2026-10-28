#include "Krystal.Booey/Rendering/MathML/RenderMathMLMenclose.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Graphics/LayoutRoundedRect.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>
#include <numbers>

namespace krys::boo
{
  using Notation = mathml::MathMLMencloseElement::MencloseNotationFlag;

  namespace
  {
    // The MathML in HTML5 implementation note suggests drawing the left part of longdiv with a parenthesis.
    // For now, we use a Bezier curve and this somewhat arbitrary value.
    constexpr uint16 LongDivLeftSpace = 10;
  }

  RenderMathMLMenclose::RenderMathMLMenclose(mathml::MathMLMencloseElement &element,
                                             RenderStyle &&style) noexcept
      : RenderMathMLRow(Type::MathMLMenclose, element, krys::move(style))
  {
    krys_debug_assert(IsRenderMathMLMenclose());
  }

  RenderMathMLMenclose::~RenderMathMLMenclose() noexcept = default;

  // This arbitrary thickness value is used for the parameter \xi_8 from the MathML in HTML5 implementation
  // note. For now, we take:
  // - OverbarVerticalGap = UnderbarVerticalGap = 3\xi_8
  // - OverbarRuleThickness = UnderbarRuleThickness = \xi_8
  // - OverbarExtraAscender = UnderbarExtraAscender = \xi_8
  // FIXME: OverBar and UnderBar parameters should be read from the MATH tables.
  // See https://bugs.webkit.org/show_bug.cgi?id=122297
  LayoutUnit RenderMathMLMenclose::RuleThickness() const noexcept
  {
    return LayoutUnit(0.05f * Style().GetFontCascade().Size());
  }

  RenderMathMLMenclose::SpaceAroundContent
    RenderMathMLMenclose::GetSpaceAroundContent(LayoutUnit contentWidth,
                                                LayoutUnit contentHeight) const noexcept
  {
    SpaceAroundContent space;
    space.Right = 0;
    space.Top = 0;
    space.Bottom = 0;
    space.Left = 0;

    LayoutUnit thickness = RuleThickness();
    // In the MathML in HTML5 implementation note, the "left" notation is described as follows:
    // - left side is 3\xi_8 padding + \xi_8 border + \xi_8 margin = 5\xi_8
    // - top space is Overbar Vertical Gap + Overbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    // - bottom space is Underbar Vertical Gap + Underbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    // The "right" notation is symmetric.
    if (HasNotation(Notation::Left))
    {
      space.Left = std::max(space.Left, 5 * thickness);
    }
    if (HasNotation(Notation::Right))
    {
      space.Right = std::max(space.Right, 5 * thickness);
    }
    if (HasNotation(Notation::Left) || HasNotation(Notation::Right))
    {
      LayoutUnit extraSpace = 4 * thickness;
      space.Top = std::max(space.Top, extraSpace);
      space.Bottom = std::max(space.Bottom, extraSpace);
    }

    // In the MathML in HTML5 implementation note, the "top" notation is described as follows:
    // - left and right space are 4\xi_8
    // - top side is Vertical Gap + Rule Thickness + Extra Ascender = 3\xi_8 + \xi_8 + \xi_8 = 5\xi_8
    // The "bottom" notation is symmetric.
    if (HasNotation(Notation::Top))
    {
      space.Top = std::max(space.Top, 5 * thickness);
    }
    if (HasNotation(Notation::Bottom))
    {
      space.Bottom = std::max(space.Bottom, 5 * thickness);
    }
    if (HasNotation(Notation::Top) || HasNotation(Notation::Bottom))
    {
      LayoutUnit extraSpace = 4 * thickness;
      space.Left = std::max(space.Left, extraSpace);
      space.Right = std::max(space.Right, extraSpace);
    }

    // For longdiv, we use our own rules for now:
    // - top space is like "top" notation
    // - bottom space is like "bottom" notation
    // - right space is like "right" notation
    // - left space is longDivLeftSpace * \xi_8
    if (HasNotation(Notation::LongDiv))
    {
      space.Top = std::max(space.Top, 5 * thickness);
      space.Bottom = std::max(space.Bottom, 5 * thickness);
      space.Left = std::max(space.Left, LongDivLeftSpace * thickness);
      space.Right = std::max(space.Right, 4 * thickness);
    }

    // In the MathML in HTML5 implementation note, the "rounded" notation is described as follows:
    // - top/bottom/left/right side have 3\xi_8 padding + \xi_8 border + \xi_8 margin = 5\xi_8
    if (HasNotation(Notation::RoundedBox))
    {
      LayoutUnit extraSpace = 5 * thickness;
      space.Left = std::max(space.Left, extraSpace);
      space.Right = std::max(space.Right, extraSpace);
      space.Top = std::max(space.Top, extraSpace);
      space.Bottom = std::max(space.Bottom, extraSpace);
    }

    // In the MathML in HTML5 implementation note, the "rounded" notation is described as follows:
    // - top/bottom/left/right spaces are \xi_8/2
    if (HasNotation(Notation::UpDiagonalStrike) || HasNotation(Notation::DownDiagonalStrike))
    {
      LayoutUnit extraSpace = thickness / 2;
      space.Left = std::max(space.Left, extraSpace);
      space.Right = std::max(space.Right, extraSpace);
      space.Top = std::max(space.Top, extraSpace);
      space.Bottom = std::max(space.Bottom, extraSpace);
    }

    // In the MathML in HTML5 implementation note, the "circle" notation is described as follows:
    // - We draw the ellipse of axes the axes of symmetry of this ink box
    // - The radii of the ellipse are \sqrt{2}contentWidth/2 and \sqrt{2}contentHeight/2
    // - The thickness of the ellipse is \xi_8
    // - We add extra margin of \xi_8
    // Then for example the top space is \sqrt{2}contentHeight/2 - contentHeight/2 + \xi_8/2 + \xi_8.
    if (HasNotation(Notation::Circle))
    {
      LayoutUnit extraSpace {(contentWidth * (std::numbers::sqrt2_v<float> - 1) + 3 * thickness) / 2};
      space.Left = std::max(space.Left, extraSpace);
      space.Right = std::max(space.Right, extraSpace);
      extraSpace = (contentHeight * (std::numbers::sqrt2_v<float> - 1) + 3 * thickness) / 2;
      space.Top = std::max(space.Top, extraSpace);
      space.Bottom = std::max(space.Bottom, extraSpace);
    }

    // In the MathML in HTML5 implementation note, the "vertical" and "horizontal" notations do not add space
    // around the content.

    return space;
  }

  void RenderMathMLMenclose::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());

    LayoutUnit preferredWidth = PreferredLogicalWidthOfRowItems();
    SpaceAroundContent space = GetSpaceAroundContent(preferredWidth, 0);
    preferredWidth += space.Left + space.Right;
    _maxPreferredLogicalWidth = _minPreferredLogicalWidth = preferredWidth;

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::CalculatePreferredLogicalWidth);
    ApplySizeToMathContent(LayoutPhase::CalculatePreferredLogicalWidth, sizes);

    AdjustPreferredLogicalWidthsForBorderAndPadding();

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderMathMLMenclose::LayoutBlock(RelayoutChildren relayoutChildren, LayoutUnit) noexcept
  {
    krys_debug_assert(NeedsLayout());

    InsertPositionedChildrenIntoContainingBlock();

    if (relayoutChildren == RelayoutChildren::No && SimplifiedLayout())
    {
      return;
    }

    LayoutFloatingChildren();

    RecomputeLogicalWidth();
    ComputeAndSetBlockDirectionMarginsOfChildren();

    LayoutUnit contentWidth, contentAscent, contentDescent;
    StretchVerticalOperatorsAndLayoutChildren();
    GetContentBoundingBox(contentWidth, contentAscent, contentDescent);
    LayoutRowItems(contentWidth, contentAscent);

    SpaceAroundContent space = GetSpaceAroundContent(contentWidth, contentAscent + contentDescent);
    SetLogicalWidth(space.Left + contentWidth + space.Right);
    SetLogicalHeight(space.Top + contentAscent + contentDescent + space.Bottom);
    ShiftInFlowChildren(space.Left, space.Top);

    _contentRect = LayoutRect(space.Left, space.Top, contentWidth, contentAscent + contentDescent);

    auto sizes = SizeAppliedToMathContentFor(LayoutPhase::Layout);
    auto shift = ApplySizeToMathContent(LayoutPhase::Layout, sizes);
    ShiftInFlowChildren(shift, 0);

    AdjustLayoutForBorderAndPadding();
    _contentRect.MoveBy(LayoutPoint(BorderLeft() + PaddingLeft(), BorderAndPaddingBefore()));

    LayoutOutOfFlowBoxes(relayoutChildren);
  }

  namespace
  {
    // GraphicsContext::DrawLine does not seem appropriate to draw menclose lines.
    // To avoid unexpected behaviors and inconsistency with other notations, we just use StrokePath.
    void DrawLine(PaintInfo &info, LayoutUnit xStart, LayoutUnit yStart, LayoutUnit xEnd,
                  LayoutUnit yEnd) noexcept
    {
      Path line;
      line.MoveTo(LayoutPoint(xStart, yStart));
      line.AddLineTo(LayoutPoint(xEnd, yEnd));
      info.Context().StrokePath(line);
    }
  }

  void RenderMathMLMenclose::Paint(PaintInfo &info, const LayoutPoint &paintOffset) noexcept
  {
    RenderMathMLRow::Paint(info, paintOffset);

    if (info.Context().PaintingDisabled() || info.Phase != PaintPhase::Foreground
        || Style().UsedVisibility() != Visibility::Visible)
    {
      return;
    }

    LayoutUnit thickness = RuleThickness();

    // Make a copy of the PaintInfo because ApplyTransform will modify its rect.
    PaintInfo paintInfo(info);
    GraphicsContextStateSaver stateSaver(paintInfo.Context());

    paintInfo.Context().SetStrokeThickness(thickness);
    paintInfo.Context().SetStrokeStyle(StrokeStyle::SolidStroke);
    paintInfo.Context().SetStrokeColor(Style().VisitedDependentColorWithColorFilter(css::PropertyId::Color));
    paintInfo.Context().SetFillColor(Color::transparentBlack);
    paintInfo.ApplyTransform(AffineTransform().Translate(FloatPoint(paintOffset + Location())));

    // In the MathML in HTML5 implementation note, the "left" notation is described as follows:
    // - center of the left vertical bar is at 3\xi_8 padding + \xi_8 border/2 = 7\xi_8/2
    // - top space is Overbar Vertical Gap + Overbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    // - bottom space is Underbar Vertical Gap + Underbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    if (HasNotation(Notation::Left))
    {
      LayoutUnit x = _contentRect.X() - 7 * thickness / 2;
      LayoutUnit yStart = _contentRect.Y() - 4 * thickness;
      LayoutUnit yEnd = _contentRect.MaxY() + 4 * thickness;
      DrawLine(paintInfo, x, yStart, x, yEnd);
    }

    // In the MathML in HTML5 implementation note, the "right" notation is described as follows:
    // - center of the right vertical bar is at 3\xi_8 padding + \xi_8 border/2 = 7\xi_8/2
    // - top space is Overbar Vertical Gap + Overbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    // - bottom space is Underbar Vertical Gap + Underbar Rule Thickness = 3\xi_8 + \xi_8 = 4\xi_8
    if (HasNotation(Notation::Right))
    {
      LayoutUnit x = _contentRect.MaxX() + 7 * thickness / 2;
      LayoutUnit yStart = _contentRect.Y() - 4 * thickness;
      LayoutUnit yEnd = _contentRect.MaxY() + 4 * thickness;
      DrawLine(paintInfo, x, yStart, x, yEnd);
    }

    // In the MathML in HTML5 implementation note, the "vertical" notation is horizontally centered.
    if (HasNotation(Notation::VerticalStrike))
    {
      LayoutUnit x = _contentRect.X() + (_contentRect.Width() - thickness) / 2;
      LayoutUnit yStart = _contentRect.Y();
      LayoutUnit yEnd = _contentRect.MaxY();
      DrawLine(paintInfo, x, yStart, x, yEnd);
    }

    // In the MathML in HTML5 implementation note, the "top" notation is described as follows:
    // - middle of the top horizontal bar is at Vertical Gap + Rule Thickness / 2 = 7\xi_8/2
    // - left and right spaces have size 4\xi_8
    if (HasNotation(Notation::Top))
    {
      LayoutUnit y = _contentRect.Y() - 7 * thickness / 2;
      LayoutUnit xStart = _contentRect.X() - 4 * thickness;
      LayoutUnit xEnd = _contentRect.MaxX() + 4 * thickness;
      DrawLine(paintInfo, xStart, y, xEnd, y);
    }

    // In the MathML in HTML5 implementation note, the "bottom" notation is described as follows:
    // - middle of the bottom horizontal bar is at Vertical Gap + Rule Thickness / 2 = 7\xi_8/2
    // - left and right spaces have size 4\xi_8
    if (HasNotation(Notation::Bottom))
    {
      LayoutUnit y = _contentRect.MaxY() + 7 * thickness / 2;
      LayoutUnit xStart = _contentRect.X() - 4 * thickness;
      LayoutUnit xEnd = _contentRect.MaxX() + 4 * thickness;
      DrawLine(paintInfo, xStart, y, xEnd, y);
    }

    // In the MathML in HTML5 implementation note, the "vertical" notation is vertically centered.
    if (HasNotation(Notation::HorizontalStrike))
    {
      LayoutUnit y = _contentRect.Y() + (_contentRect.Height() - thickness) / 2;
      LayoutUnit xStart = _contentRect.X();
      LayoutUnit xEnd = _contentRect.MaxX();
      DrawLine(paintInfo, xStart, y, xEnd, y);
    }

    // In the MathML in HTML5 implementation note, the "updiagonalstrike" goes from the bottom left corner
    // to the top right corner.
    if (HasNotation(Notation::UpDiagonalStrike))
    {
      DrawLine(paintInfo, _contentRect.X(), _contentRect.MaxY(), _contentRect.MaxX(), _contentRect.Y());
    }

    // In the MathML in HTML5 implementation note, the "downdiagonalstrike" goes from the top left corner
    // to the bottom right corner.
    if (HasNotation(Notation::DownDiagonalStrike))
    {
      DrawLine(paintInfo, _contentRect.X(), _contentRect.Y(), _contentRect.MaxX(), _contentRect.MaxY());
    }

    // In the MathML in HTML5 implementation note, the "roundedbox" has radii size 3\xi_8 and is obtained
    // by inflating the content box by 3\xi_8 + \xi_8/2 = 7\xi_8/2
    if (HasNotation(Notation::RoundedBox))
    {
      LayoutSize radiiSize(3 * thickness, 3 * thickness);
      LayoutRoundedRect::Radii radii(radiiSize, radiiSize, radiiSize, radiiSize);
      LayoutRoundedRect roundedRect(_contentRect, radii);
      roundedRect.Inflate(7 * thickness / 2);
      Path path;
      path.AddRoundedRect(roundedRect);
      paintInfo.Context().StrokePath(path);
    }

    // For longdiv, we use our own rules for now:
    // - top space is like "top" notation
    // - bottom space is like "bottom" notation
    // - right space is like "right" notation
    // - left space is longDivLeftSpace * \xi_8
    // - We subtract half of the thickness from these spaces to obtain "top", "bottom", "left"
    //   and "right" coordinates.
    // - The top bar is drawn from "right" to "left" and positioned at vertical offset "top".
    // - The left part is draw as a quadratic Bezier curve with end points going from "top" to
    //   "bottom" and positioned at horizontal offset "left".
    // - In order to force the curvature of the left part, we use a middle point that is vertically
    //   centered and shifted towards the right by longDivLeftSpace * \xi_8
    if (HasNotation(Notation::LongDiv))
    {
      LayoutUnit top = _contentRect.Y() - 7 * thickness / 2;
      LayoutUnit bottom = _contentRect.MaxY() + 7 * thickness / 2;
      LayoutUnit left = _contentRect.X() - LongDivLeftSpace * thickness + thickness / 2;
      LayoutUnit right = _contentRect.MaxX() + 4 * thickness;
      LayoutUnit midX = left + LongDivLeftSpace * thickness;
      LayoutUnit midY = (top + bottom) / 2;
      Path path;
      path.MoveTo(LayoutPoint(right, top));
      path.AddLineTo(LayoutPoint(left, top));
      path.AddQuadCurveTo(LayoutPoint(midX, midY), LayoutPoint(left, bottom));
      paintInfo.Context().StrokePath(path);
    }

    // In the MathML in HTML5 implementation note, the "circle" notation is described as follows:
    // - The center and axes are the same as the content bounding box.
    // - The width of the bounding box is \xi_8/2 + contentWidth * \sqrt{2} + \xi_8/2
    // - The height is \xi_8/2 + contentHeight * \sqrt{2} + \xi_8/2
    if (HasNotation(Notation::Circle))
    {
      LayoutRect ellipseRect;
      ellipseRect.SetWidth(_contentRect.Width() * std::numbers::sqrt2_v<float> + thickness);
      ellipseRect.SetHeight(_contentRect.Height() * std::numbers::sqrt2_v<float> + thickness);
      ellipseRect.SetX(_contentRect.X() - (ellipseRect.Width() - _contentRect.Width()) / 2);
      ellipseRect.SetY(_contentRect.Y() - (ellipseRect.Height() - _contentRect.Height()) / 2);
      Path path;
      path.AddEllipseInRect(ellipseRect);
      paintInfo.Context().StrokePath(path);
    }
  }
}
