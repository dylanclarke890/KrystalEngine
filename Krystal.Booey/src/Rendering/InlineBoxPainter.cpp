#include "Krystal.Booey/Rendering/InlineBoxPainter.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/BackgroundPainter.hpp"
#include "Krystal.Booey/Rendering/BorderPainter.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <ranges>

namespace krys::boo
{
  InlineBoxPainter::InlineBoxPainter(const layout_integration::InlineContent &inlineContent,
                                     const inline_display::Box &box, PaintInfo &paintInfo,
                                     const LayoutPoint &paintOffset) noexcept
      : InlineBoxPainter(*inline_iterator::InlineBoxFor(inlineContent, box), paintInfo, paintOffset)
  {
  }

  InlineBoxPainter::InlineBoxPainter(const inline_iterator::InlineBox &inlineBox, PaintInfo &paintInfo,
                                     const LayoutPoint &paintOffset) noexcept
      : _inlineBox(inlineBox), _paintInfo(paintInfo), _paintOffset(paintOffset),
        _renderer(_inlineBox.Renderer()), _isFirstLineBox(_inlineBox.GetLineBox()->IsFirst()),
        _isRootInlineBox(_inlineBox.IsRootInlineBox()), _isHorizontal(_inlineBox.IsHorizontal())
  {
  }

  InlineBoxPainter::~InlineBoxPainter() noexcept = default;

  void InlineBoxPainter::Paint() noexcept
  {
    if (_paintInfo.Phase == PaintPhase::Outline || _paintInfo.Phase == PaintPhase::SelfOutline)
    {
      if (Renderer().Style().UsedVisibility() != Visibility::Visible || !Renderer().HasOutline()
          || _isRootInlineBox)
      {
        return;
      }

      // There are no continuations: blocks in inlines are laid out in the inline's lines, so the inline
      // paints its own outline.
      auto &inlineFlow = Downcast<RenderInline>(Renderer());
      if (_paintInfo.OutlineObjects)
      {
        _paintInfo.OutlineObjects->add(&inlineFlow);
      }
      return;
    }

    if (_paintInfo.Phase == PaintPhase::Mask)
    {
      PaintMask();
      return;
    }

    if (_paintInfo.Phase == PaintPhase::Accessibility)
    {
      // FIXME: Accessibility bounds come with accessibility.
      return;
    }

    PaintDecorations();
  }

  template <typename T>
  KRYS_NODISCARD static LayoutRect ClipRectForNinePieceImageStrip(const inline_iterator::InlineBox &box,
                                                                  const T &image,
                                                                  const LayoutRect &paintRect) noexcept
  {
    LayoutRect clipRect(paintRect);
    auto &style = box.Renderer().Style();
    LayoutBoxExtent outsets = style.ImageOutsets(image);
    auto closedEdges = box.ClosedEdges();
    if (box.IsHorizontal())
    {
      clipRect.SetY(paintRect.Y() - outsets.Top());
      clipRect.SetHeight(paintRect.Height() + outsets.Top() + outsets.Bottom());
      if (closedEdges.Left())
      {
        clipRect.SetX(paintRect.X() - outsets.Left());
        clipRect.SetWidth(paintRect.Width() + outsets.Left());
      }
      if (closedEdges.Right())
      {
        clipRect.SetWidth(clipRect.Width() + outsets.Right());
      }
    }
    else
    {
      clipRect.SetX(paintRect.X() - outsets.Left());
      clipRect.SetWidth(paintRect.Width() + outsets.Left() + outsets.Right());
      if (closedEdges.Top())
      {
        clipRect.SetY(paintRect.Y() - outsets.Top());
        clipRect.SetHeight(paintRect.Height() + outsets.Top());
      }
      if (closedEdges.Bottom())
      {
        clipRect.SetHeight(clipRect.Height() + outsets.Bottom());
      }
    }
    return clipRect;
  }

  void InlineBoxPainter::PaintDecorations() noexcept
  {
    if (!_paintInfo.ShouldPaintWithinRoot(Renderer())
        || Renderer().Style().UsedVisibility() != Visibility::Visible
        || _paintInfo.Phase != PaintPhase::Foreground)
    {
      return;
    }

    if (!_isRootInlineBox && !Renderer().HasVisibleBoxDecorations())
    {
      return;
    }

    auto &styleToUse = Style();
    // You can use p::first-line to specify a background. If so, the root inline boxes for
    // a line may actually have to paint a background.
    if (_isRootInlineBox && (!_isFirstLineBox || &styleToUse == &Renderer().Style()))
    {
      return;
    }

    // Move x/y to our coordinates.
    auto localRect = LayoutRect {_inlineBox.VisualRect()};
    LayoutPoint adjustedPaintoffset = _paintOffset + localRect.Location();
    GraphicsContext &context = _paintInfo.Context();
    LayoutRect paintRect = LayoutRect(adjustedPaintoffset, localRect.Size());
    // Shadow comes first and is behind the background and border.
    if (!BackgroundPainter::BoxShadowShouldBeAppliedToBackground(Renderer(), adjustedPaintoffset,
                                                                 BleedAvoidance::None, _inlineBox.Iterator()))
    {
      PaintBoxShadow(style::ShadowStyle::Normal, paintRect);
    }

    auto color = styleToUse.VisitedDependentColor(css::PropertyId::BackgroundColor, _paintInfo.PaintBehavior);
    auto compositeOp = Renderer().GetDocument().CompositeOperatorForBackgroundColor(color, Renderer());

    color = styleToUse.ColorByApplyingColorFilter(color);

    PaintFillLayers(color, styleToUse.GetBackgroundLayers(), paintRect, compositeOp);
    PaintBoxShadow(style::ShadowStyle::Inset, paintRect);

    // :first-line cannot be used to put borders on a line. Always paint borders with our
    // non-first-line style.
    if (_isRootInlineBox || !Renderer().Style().HasVisibleBorderDecoration())
    {
      return;
    }

    auto &borderImage = Renderer().Style().GetBorderImage();
    auto borderImageSource = borderImage.Source.TryStyleImage();
    bool hasBorderImage =
      borderImageSource && borderImageSource->CanRender(&Renderer(), styleToUse.UsedZoom());
    if (hasBorderImage && !borderImageSource->IsLoaded(&Renderer()))
    {
      return; // Don't paint anything while we wait for the image to load.
    }

    BorderPainter borderPainter {Renderer(), _paintInfo};

    if (!hasBorderImage || !_inlineBox.IsSplit())
    {
      auto closedEdges = _inlineBox.ClosedEdges();
      borderPainter.PaintBorder(paintRect, styleToUse, BleedAvoidance::None, closedEdges);
      return;
    }

    // We have a border image that spans multiple lines.
    // We need to adjust tx and ty by the width of all previous lines.
    // Think of border image painting on inlines as though you had one long line, a single continuous
    // strip. Even though that strip has been broken up across multiple lines, you still paint it
    // as though you had one single line. This means each line has to pick up the image where
    // the previous line left off.
    // FIXME: What the heck do we do with RTL here? The math we're using is obviously not right,
    // but it isn't even clear how this should work at all.
    LayoutUnit logicalOffsetOnLine;
    for (auto box = _inlineBox.NextInlineBoxLineLeftward(); box; box.TraverseInlineBoxLineLeftward())
    {
      logicalOffsetOnLine += box->LogicalWidth();
    }
    LayoutUnit totalLogicalWidth = logicalOffsetOnLine;
    for (auto box = _inlineBox.Iterator(); box; box.TraverseInlineBoxLineRightward())
    {
      totalLogicalWidth += box->LogicalWidth();
    }

    LayoutUnit stripX = adjustedPaintoffset.X() - (IsHorizontal() ? logicalOffsetOnLine : 0_lu);
    LayoutUnit stripY = adjustedPaintoffset.Y() - (IsHorizontal() ? 0_lu : logicalOffsetOnLine);
    LayoutUnit stripWidth = IsHorizontal() ? totalLogicalWidth : localRect.Width();
    LayoutUnit stripHeight = IsHorizontal() ? localRect.Height() : totalLogicalWidth;

    LayoutRect clipRect = ClipRectForNinePieceImageStrip(_inlineBox, borderImage, paintRect);
    GraphicsContextStateSaver stateSaver(context);
    context.Clip(clipRect);
    borderPainter.PaintBorder(LayoutRect(stripX, stripY, stripWidth, stripHeight), styleToUse);
  }

  void InlineBoxPainter::PaintMask() noexcept
  {
    if (!_paintInfo.ShouldPaintWithinRoot(Renderer())
        || Renderer().Style().UsedVisibility() != Visibility::Visible || _paintInfo.Phase != PaintPhase::Mask)
    {
      return;
    }

    // Move x/y to our coordinates.
    auto localRect = LayoutRect {_inlineBox.VisualRect()};
    LayoutPoint adjustedPaintOffset = _paintOffset + localRect.Location();

    auto &maskBorder = Renderer().Style().GetMaskBorder();
    auto maskBorderSource = maskBorder.Source.TryStyleImage();

    // Figure out if we need to push a transparency layer to render our mask.
    bool pushTransparencyLayer = false;
    bool compositedMask = Renderer().HasLayer() && Renderer().Layer()->HasCompositedMask();
    bool flattenCompositingLayers =
      Renderer().View().FrameView().GetPaintBehavior().Contains(PaintBehavior::FlattenCompositingLayers);
    CompositeOperator compositeOp = CompositeOperator::SourceOver;
    if (!compositedMask || flattenCompositingLayers)
    {
      if ((maskBorderSource && style::HasImageInAnyLayer(Renderer().Style().GetMaskLayers()))
          || Renderer().Style().GetMaskLayers().UsedLength() > 1)
      {
        pushTransparencyLayer = true;
      }

      compositeOp = CompositeOperator::DestinationIn;
      if (pushTransparencyLayer)
      {
        _paintInfo.Context().SetCompositeOperation(CompositeOperator::DestinationIn);
        _paintInfo.Context().BeginTransparencyLayer(1.0f);
        compositeOp = CompositeOperator::SourceOver;
      }
    }

    LayoutRect paintRect = LayoutRect(adjustedPaintOffset, localRect.Size());

    PaintFillLayers(Color(), Renderer().Style().GetMaskLayers(), paintRect, compositeOp);

    bool hasBoxImage =
      maskBorderSource && maskBorderSource->CanRender(&Renderer(), Renderer().Style().UsedZoom());
    if (!hasBoxImage || !maskBorderSource->IsLoaded(&Renderer()))
    {
      if (pushTransparencyLayer)
      {
        _paintInfo.Context().EndTransparencyLayer();
      }
      return; // Don't paint anything while we wait for the image to load.
    }

    BorderPainter borderPainter {Renderer(), _paintInfo};

    if (!_inlineBox.IsSplit())
    {
      borderPainter.PaintNinePieceImage(LayoutRect(adjustedPaintOffset, localRect.Size()), Renderer().Style(),
                                        maskBorder, compositeOp);
    }
    else
    {
      // We have a mask image that spans multiple lines.
      // We need to adjust _tx and _ty by the width of all previous lines.
      LayoutUnit logicalOffsetOnLine;
      for (auto box = _inlineBox.NextInlineBoxLineLeftward(); box; box.TraverseInlineBoxLineLeftward())
      {
        logicalOffsetOnLine += box->LogicalWidth();
      }
      LayoutUnit totalLogicalWidth = logicalOffsetOnLine;
      for (auto box = _inlineBox.Iterator(); box; box.TraverseInlineBoxLineRightward())
      {
        totalLogicalWidth += box->LogicalWidth();
      }
      LayoutUnit stripX = adjustedPaintOffset.X() - (IsHorizontal() ? logicalOffsetOnLine : 0_lu);
      LayoutUnit stripY = adjustedPaintOffset.Y() - (IsHorizontal() ? 0_lu : logicalOffsetOnLine);
      LayoutUnit stripWidth = IsHorizontal() ? totalLogicalWidth : localRect.Width();
      LayoutUnit stripHeight = IsHorizontal() ? localRect.Height() : totalLogicalWidth;

      LayoutRect clipRect = ClipRectForNinePieceImageStrip(_inlineBox, maskBorder, paintRect);
      GraphicsContextStateSaver stateSaver(_paintInfo.Context());
      _paintInfo.Context().Clip(clipRect);
      borderPainter.PaintNinePieceImage(LayoutRect(stripX, stripY, stripWidth, stripHeight),
                                        Renderer().Style(), maskBorder, compositeOp);
    }

    if (pushTransparencyLayer)
    {
      _paintInfo.Context().EndTransparencyLayer();
    }
  }

  template <typename TLayers>
  void InlineBoxPainter::PaintFillLayers(const Color &color, const TLayers &fillLayers,
                                         const LayoutRect &rect, CompositeOperator op) noexcept
  {
    for (auto &layer : fillLayers.UsedValues() | std::views::reverse)
    {
      PaintFillLayer(color,
                     FillLayerToPaint<typename TLayers::value_type> {
                       .Layer = layer, .IsLast = &layer == &fillLayers.UsedLast()},
                     rect, op);
    }
  }

  template <typename TLayer>
  void InlineBoxPainter::PaintFillLayer(const Color &color, const FillLayerToPaint<TLayer> &fillLayer,
                                        const LayoutRect &rect, CompositeOperator op) noexcept
  {
    RefPtr image = fillLayer.Layer.GetImage().TryStyleImage();
    bool hasFillImage = image && image->CanRender(&Renderer(), Renderer().Style().UsedZoom());
    bool hasFillImageOrBorderRadius = hasFillImage || Renderer().Style().HasBorderRadius();

    BackgroundPainter backgroundPainter {Renderer(), _paintInfo};

    if (!hasFillImageOrBorderRadius || !_inlineBox.IsSplit() || _isRootInlineBox)
    {
      backgroundPainter.PaintFillLayer(color, fillLayer, rect, BleedAvoidance::None, _inlineBox.Iterator(),
                                       {}, op);
      return;
    }

    if (Renderer().Style().GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
    {
      GraphicsContextStateSaver stateSaver(_paintInfo.Context());
      _paintInfo.Context().Clip(
        {FloatPoint(rect.Location()), _inlineBox.VisualRectIgnoringBlockDirection().Size()});
      backgroundPainter.PaintFillLayer(color, fillLayer, rect, BleedAvoidance::None, _inlineBox.Iterator(),
                                       {}, op);
      return;
    }

    // We have a fill image that spans multiple lines.
    // We need to adjust tx and ty by the width of all previous lines.
    // Think of background painting on inlines as though you had one long line, a single continuous
    // strip. Even though that strip has been broken up across multiple lines, you still paint it
    // as though you had one single line. This means each line has to pick up the background where
    // the previous line left off.
    LayoutUnit logicalOffsetOnLine;
    LayoutUnit totalLogicalWidth;
    if (Renderer().GetWritingMode().IsBidiLTR())
    {
      for (auto box = _inlineBox.NextInlineBoxLineLeftward(); box; box.TraverseInlineBoxLineLeftward())
      {
        logicalOffsetOnLine += box->LogicalWidth();
      }
      totalLogicalWidth = logicalOffsetOnLine;
      for (auto box = _inlineBox.Iterator(); box; box.TraverseInlineBoxLineRightward())
      {
        totalLogicalWidth += box->LogicalWidth();
      }
    }
    else
    {
      for (auto box = _inlineBox.NextInlineBoxLineRightward(); box; box.TraverseInlineBoxLineRightward())
      {
        logicalOffsetOnLine += box->LogicalWidth();
      }
      totalLogicalWidth = logicalOffsetOnLine;
      for (auto box = _inlineBox.Iterator(); box; box.TraverseInlineBoxLineLeftward())
      {
        totalLogicalWidth += box->LogicalWidth();
      }
    }
    LayoutRect backgroundImageStrip {
      rect.X() - (IsHorizontal() ? logicalOffsetOnLine : 0_lu),
      rect.Y() - (IsHorizontal() ? 0_lu : logicalOffsetOnLine),
      IsHorizontal() ? totalLogicalWidth : LayoutUnit(_inlineBox.VisualRectIgnoringBlockDirection().Width()),
      IsHorizontal() ? LayoutUnit(_inlineBox.VisualRectIgnoringBlockDirection().Height())
                     : totalLogicalWidth};

    GraphicsContextStateSaver stateSaver(_paintInfo.Context());
    _paintInfo.Context().Clip(FloatRect {rect});
    backgroundPainter.PaintFillLayer(color, fillLayer, rect, BleedAvoidance::None, _inlineBox.Iterator(),
                                     backgroundImageStrip, op);
  }

  void InlineBoxPainter::PaintBoxShadow(style::ShadowStyle shadowStyle, const LayoutRect &paintRect) noexcept
  {
    BackgroundPainter backgroundPainter {Renderer(), _paintInfo};

    if (!_inlineBox.IsSplit() || _isRootInlineBox)
    {
      backgroundPainter.PaintBoxShadow(paintRect, Style(), shadowStyle);
      return;
    }

    // FIXME: We can do better here in the multi-line case. We want to push a clip so that the shadow doesn't
    // protrude incorrectly at the edges, and we want to possibly include shadows cast from the
    // previous/following lines
    auto closedEdges = _inlineBox.ClosedEdges();
    backgroundPainter.PaintBoxShadow(paintRect, Style(), shadowStyle, closedEdges);
  }

  const RenderStyle &InlineBoxPainter::Style() const noexcept
  {
    return _isFirstLineBox ? Renderer().FirstLineStyle() : Renderer().Style();
  }
}
