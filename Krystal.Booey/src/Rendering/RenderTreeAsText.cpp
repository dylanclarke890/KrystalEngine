#include "Krystal.Booey/Rendering/RenderTreeAsText.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderCounter.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderTableCell.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGImage.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceContainer.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGRoot.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGShape.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGGradientStop.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderTreeAsText.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/HexNumber.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Text/Unicode.hpp"

namespace krys::boo
{
  namespace
  {
    void WriteLayers(TextStream &ts, const RenderLayer &rootLayer, RenderLayer &layer,
                     const LayoutRect &paintRect, EnumSet<RenderAsTextFlag> behavior) noexcept;

    void PrintBorderStyle(TextStream &ts, const BorderStyle borderStyle) noexcept
    {
      switch (borderStyle)
      {
        case BorderStyle::None:   ts << "none"; break;
        case BorderStyle::Hidden: ts << "hidden"; break;
        case BorderStyle::Inset:  ts << "inset"; break;
        case BorderStyle::Groove: ts << "groove"; break;
        case BorderStyle::Ridge:  ts << "ridge"; break;
        case BorderStyle::Outset: ts << "outset"; break;
        case BorderStyle::Dotted: ts << "dotted"; break;
        case BorderStyle::Dashed: ts << "dashed"; break;
        case BorderStyle::Solid:  ts << "solid"; break;
        case BorderStyle::Double: ts << "double"; break;
      }

      ts << ' ';
    }

    KRYS_NODISCARD utf8_string GetTagName(dom::Node *n) noexcept
    {
      if (n->NodeType() == dom::NodeType::DOCUMENT_NODE)
      {
        return {};
      }
      if (n->NodeType() == dom::NodeType::COMMENT_NODE)
      {
        return u8"COMMENT";
      }
      return n->NodeName();
    }

    KRYS_NODISCARD bool ShouldEnableSubpixelPrecisionForTextDump(const dom::Document &) noexcept
    {
      // FIXME: Documents with rendered outermost <svg> elements in the layer-based SVG engine dump with
      // subpixel precision, once there's SVG.
      return false;
    }
  }

  utf8_string QuoteAndEscapeNonPrintables(utf8_stringview s) noexcept
  {
    StringBuilder result;
    result.Append('"');
    auto appendCodeUnit = [&](uint32 c)
    {
      if (c == '\\')
      {
        result.Append("\\\\");
      }
      else if (c == '"')
      {
        result.Append("\\\"");
      }
      else if (c == '\n' || c == text::NoBreakSpace)
      {
        result.Append(' ');
      }
      else
      {
        if (c >= 0x20 && c < 0x7F)
        {
          result.Append(static_cast<char>(c));
        }
        else
        {
          result.Append("\\x{", Hex(c, HexConversionMode::Uppercase), '}');
        }
      }
    };
    // Characters are escaped as UTF-16 code units, as the dumps expect.
    for (auto it = text::CodePointIterator(s); !it.AtEnd(); ++it)
    {
      char32 c = *it;
      if (c >= 0x10000)
      {
        appendCodeUnit(0xD800 + ((c - 0x10000) >> 10));
        appendCodeUnit(0xDC00 + ((c - 0x10000) & 0x3FF));
      }
      else
      {
        appendCodeUnit(c);
      }
    }
    result.Append('"');
    return krys::move(result).ToString();
  }

  void RenderTreeAsText::WriteRenderObject(TextStream &ts, const RenderObject &o,
                                           EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    ts << o.RenderName();

    if (behavior.Contains(RenderAsTextFlag::ShowAddresses))
    {
      ts << ' ' << static_cast<const void *>(&o);
    }

    // FIXME: This should log even when value->value is zero.
    if (auto value = o.Style().UsedZIndex().TryValue(); value && value->Value)
    {
      ts << " zI: " << static_cast<int32>(value->Value);
    }

    if (o.GetNode())
    {
      utf8_string tagName = GetTagName(o.GetNode());
      // FIXME: Temporary hack to make tests pass by simulating the old generated content output.
      if (o.IsPseudoElement() || (o.Parent() && o.Parent()->IsPseudoElement()))
      {
        tagName.clear();
      }
      if (!tagName.empty())
      {
        ts << " {" << tagName << '}';
        // FIXME: Flag empty or unstyled Apple-style-spans (an editing artifact), once there's editing.
      }
    }

    bool enableSubpixelPrecisionForTextDump = ShouldEnableSubpixelPrecisionForTextDump(o.GetDocument());
    LayoutRect r;
    if (auto *text = DynamicDowncast<RenderText>(o))
    {
      r = LayoutRect(text->LinesBoundingBox());
    }
    else if (auto *br = DynamicDowncast<RenderLineBreak>(o); br && br->IsBR())
    {
      r = LayoutRect(br->LinesBoundingBox());
    }
    else if (auto *inlineFlow = DynamicDowncast<RenderInline>(o))
    {
      r = LayoutRect(inlineFlow->LinesBoundingBox());
    }
    else if (auto *cell = DynamicDowncast<RenderTableCell>(o))
    {
      // FIXME: Deliberately dump the "inner" box of table cells, since that is what current results reflect.
      // We'd like to clean up the results to dump both the outer box and the intrinsic padding so that both
      // bits of information are captured by the results.
      r = LayoutRect(cell->X(), cell->Y() + cell->IntrinsicPaddingBefore(), cell->Width(),
                     cell->Height() - cell->IntrinsicPaddingBefore() - cell->IntrinsicPaddingAfter());
    }
    else if (auto *box = DynamicDowncast<RenderBox>(o))
    {
      r = box->FrameRect();
    }
    // FIXME: Convert layout test results to report sub-pixel values, in the meantime using enclosingIntRect
    // for consistency with old results.
    if (enableSubpixelPrecisionForTextDump)
    {
      ts << ' ' << r;
    }
    else
    {
      ts << ' ' << EnclosingIntRect(r);
    }

    // FIXME: The SVG painting features of SVG renderers, once there's SVG.

    if (auto *renderElement = DynamicDowncast<RenderElement>(o))
    {
      // FIXME: The file name of file upload controls, once there are any.

      if (renderElement->Parent())
      {
        Color color = renderElement->Style().VisitedDependentColor(css::PropertyId::Color);
        if (!EqualIgnoringSemanticColor(
              renderElement->Parent()->Style().VisitedDependentColor(css::PropertyId::Color), color))
        {
          ts << " [color=" << SerialisationForRenderTreeAsText(color) << ']';
        }

        // Do not dump invalid or transparent backgrounds, since that is the default.
        Color backgroundColor =
          renderElement->Style().VisitedDependentColor(css::PropertyId::BackgroundColor);
        if (!EqualIgnoringSemanticColor(
              renderElement->Parent()->Style().VisitedDependentColor(css::PropertyId::BackgroundColor),
              backgroundColor)
            && backgroundColor != Color(Color::transparentBlack))
        {
          ts << " [bgcolor=" << SerialisationForRenderTreeAsText(backgroundColor) << ']';
        }

        Color textFillColor =
          renderElement->Style().VisitedDependentColor(css::PropertyId::WebkitTextFillColor);
        if (!EqualIgnoringSemanticColor(
              renderElement->Parent()->Style().VisitedDependentColor(css::PropertyId::WebkitTextFillColor),
              textFillColor)
            && textFillColor != color && textFillColor != Color(Color::transparentBlack))
        {
          ts << " [textFillColor=" << SerialisationForRenderTreeAsText(textFillColor) << ']';
        }

        // The text stroke properties (-webkit-text-stroke-*) were dropped.
      }

      auto *box = DynamicDowncast<RenderBoxModelObject>(o);
      if (!box || Is<RenderLineBreak>(*box))
      {
        return;
      }

      LayoutUnit borderTop = box->BorderTop();
      LayoutUnit borderRight = box->BorderRight();
      LayoutUnit borderBottom = box->BorderBottom();
      LayoutUnit borderLeft = box->BorderLeft();
      bool overridden = renderElement->Style().GetBorderImageWidth().OverridesBorderWidths();
      // FIXME: Subtract a fieldset's intrinsic border (intrinsicBorderForFieldset), once there are fieldsets.
      if (borderTop || borderRight || borderBottom || borderLeft)
      {
        ts << " [border:";

        auto printBorder = [&](const LayoutUnit &width, const BorderStyle &style, const style::Color &color)
        {
          if (!width)
          {
            ts << " none";
          }
          else
          {
            ts << " (" << width << "px ";
            PrintBorderStyle(ts, style);
            auto resolvedColor = renderElement->Style().ColorResolvingCurrentColor(color);
            ts << SerialisationForRenderTreeAsText(resolvedColor) << ')';
          }
        };

        const BorderValue *prevBorder = &renderElement->Style().BorderTop();
        printBorder(borderTop, renderElement->Style().BorderTopStyle(),
                    renderElement->Style().BorderTopColor());

        if (renderElement->Style().BorderRight() != *prevBorder || (overridden && borderRight != borderTop))
        {
          prevBorder = &renderElement->Style().BorderRight();
          printBorder(borderRight, renderElement->Style().BorderRightStyle(),
                      renderElement->Style().BorderRightColor());
        }

        if (renderElement->Style().BorderBottom() != *prevBorder
            || (overridden && borderBottom != borderRight))
        {
          prevBorder = &renderElement->Style().BorderBottom();
          printBorder(borderBottom, renderElement->Style().BorderBottomStyle(),
                      renderElement->Style().BorderBottomColor());
        }

        if (renderElement->Style().BorderLeft() != *prevBorder || (overridden && borderLeft != borderBottom))
        {
          prevBorder = &renderElement->Style().BorderLeft();
          printBorder(borderLeft, renderElement->Style().BorderLeftStyle(),
                      renderElement->Style().BorderLeftColor());
        }
        ts << ']';
      }

      // FIXME: The padding of MathML blocks, once there's MathML.
    }

    if (auto *cell = DynamicDowncast<RenderTableCell>(o))
    {
      ts << " [r=" << cell->RowIndex() << " c=" << cell->Col() << " rs=" << cell->RowSpan()
         << " cs=" << cell->ColSpan() << ']';
    }

    if (auto *listMarker = DynamicDowncast<RenderListMarker>(o))
    {
      auto markerText = listMarker->TextWithoutSuffix();
      if (!markerText.empty())
      {
        auto it = text::CodePointIterator(utf8_stringview {markerText});
        char32 first = *it;
        ++it;
        if (!it.AtEnd())
        {
          markerText = QuoteAndEscapeNonPrintables(markerText);
        }
        else
        {
          switch (first)
          {
            case text::Bullet:      markerText = u8"bullet"; break;
            case text::BlackSquare: markerText = u8"black square"; break;
            case text::WhiteBullet: markerText = u8"white bullet"; break;
            default:                markerText = QuoteAndEscapeNonPrintables(markerText);
          }
        }
        ts << ": " << markerText;
      }
    }

    WriteDebugInfo(ts, o, behavior);
  }

  void WriteDebugInfo(TextStream &ts, const RenderObject &object, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    if (behavior.Contains(RenderAsTextFlag::ShowIDAndClass))
    {
      if (auto *element = DynamicDowncast<dom::Element>(object.GetNode()))
      {
        if (element->HasAttribute(u8"id"))
        {
          ts << " id=\"" << element->Id() << '"';
        }

        if (element->HasAttribute(u8"class"))
        {
          ts << " class=\"";
          auto classNames = element->ClassNames();
          for (size_t i = 0; i < classNames.size(); ++i)
          {
            if (i > 0)
            {
              ts << ' ';
            }
            ts << classNames[i].View();
          }
          ts << '"';
        }
      }
    }

    if (behavior.Contains(RenderAsTextFlag::ShowLayoutState))
    {
      bool needsLayout = object.SelfNeedsLayout() || object.NeedsOutOfFlowMovementLayout()
                         || object.OutOfFlowChildNeedsLayout() || object.NormalChildNeedsLayout();
      if (needsLayout)
      {
        ts << " (needs layout:";
      }

      bool havePrevious = false;
      if (object.SelfNeedsLayout())
      {
        ts << " self";
        havePrevious = true;
      }

      if (object.NeedsOutOfFlowMovementLayout())
      {
        if (havePrevious)
        {
          ts << ',';
        }
        havePrevious = true;
        ts << " positioned movement";
      }

      if (object.NormalChildNeedsLayout())
      {
        if (havePrevious)
        {
          ts << ',';
        }
        havePrevious = true;
        ts << " child";
      }

      if (object.OutOfFlowChildNeedsLayout())
      {
        if (havePrevious)
        {
          ts << ',';
        }
        ts << " positioned child";
      }

      if (needsLayout)
      {
        ts << ')';
      }
    }

    if (behavior.Contains(RenderAsTextFlag::ShowOverflow))
    {
      if (auto *box = DynamicDowncast<RenderBox>(object))
      {
        if (box->HasRenderOverflow())
        {
          LayoutRect layoutOverflow = box->LayoutOverflowRect();
          ts << " (layout overflow " << layoutOverflow.X().ToInt() << ',' << layoutOverflow.Y().ToInt() << ' '
             << layoutOverflow.Width().ToInt() << 'x' << layoutOverflow.Height().ToInt() << ')';

          if (box->HasVisualOverflow())
          {
            LayoutRect visualOverflow = box->VisualOverflowRect();
            ts << " (visual overflow " << visualOverflow.X().ToInt() << ',' << visualOverflow.Y().ToInt()
               << ' ' << visualOverflow.Width().ToInt() << 'x' << visualOverflow.Height().ToInt() << ')';
          }
        }
      }

      // FIXME: The visual overflow of SVG renderers, once there's SVG.
    }
  }

  namespace
  {
    void WriteTextRuns(TextStream &ts, const RenderText &textRenderer) noexcept
    {
      auto writeTextRun = [&](const inline_iterator::TextBox &textRun)
      {
        ts << Indent;

        auto rect = textRun.VisualRectIgnoringBlockDirection();
        auto x = static_cast<int32>(rect.X());
        auto y = static_cast<int32>(rect.Y());
        // FIXME: Use non-logical width.
        auto logicalWidth =
          static_cast<int32>(std::ceil(rect.X() + (textRun.IsHorizontal() ? rect.Width() : rect.Height())))
          - x;
        // FIXME: Table cell adjustment is temporary until results can be updated.
        if (auto *tableCell = DynamicDowncast<RenderTableCell>(*textRenderer.ContainingBlock()))
        {
          y -= tableCell->IntrinsicPaddingBefore().Floor();
        }

        ts << "text run at (" << x << ',' << y << ") width " << logicalWidth;
        if (!textRun.IsLeftToRightDirection())
        {
          ts << " RTL";
        }
        ts << ": " << QuoteAndEscapeNonPrintables(textRun.OriginalText());
        if (textRun.HasHyphen())
        {
          ts << " + hyphen string "
             << QuoteAndEscapeNonPrintables(textRenderer.Style().HyphenString().View());
        }
        ts << '\n';
      };

      for (auto &run : inline_iterator::TextBoxesFor(textRenderer))
      {
        writeTextRun(run);
      }
    }
  }

  void Write(TextStream &ts, const RenderObject &renderer, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    if (auto *svgShape = DynamicDowncast<LegacyRenderSVGShape>(renderer))
    {
      Write(ts, *svgShape, behavior);
      return;
    }
    if (auto *svgGradientStop = DynamicDowncast<RenderSVGGradientStop>(renderer))
    {
      WriteSVGGradientStop(ts, *svgGradientStop, behavior);
      return;
    }
    if (auto *svgResourceContainer = DynamicDowncast<LegacyRenderSVGResourceContainer>(renderer))
    {
      WriteSVGResourceContainer(ts, *svgResourceContainer, behavior);
      return;
    }
    if (auto *svgContainer = DynamicDowncast<LegacyRenderSVGContainer>(renderer))
    {
      WriteSVGContainer(ts, *svgContainer, behavior);
      return;
    }
    if (auto *svgRoot = DynamicDowncast<LegacyRenderSVGRoot>(renderer))
    {
      Write(ts, *svgRoot, behavior);
      return;
    }
    if (auto *text = DynamicDowncast<RenderSVGText>(renderer))
    {
      WriteSVGText(ts, *text, behavior);
      return;
    }
    if (auto *inlineText = DynamicDowncast<RenderSVGInlineText>(renderer))
    {
      WriteSVGInlineText(ts, *inlineText, behavior);
      return;
    }
    if (auto *svgImage = DynamicDowncast<LegacyRenderSVGImage>(renderer))
    {
      WriteSVGImage(ts, *svgImage, behavior);
      return;
    }

    ts << Indent;
    RenderTreeAsText::WriteRenderObject(ts, renderer, behavior);
    ts << '\n';

    TextStream::IndentScope indentScope(ts);
    if (auto *textRenderer = DynamicDowncast<RenderText>(renderer))
    {
      WriteTextRuns(ts, *textRenderer);
      return;
    }

    for (auto &child : ChildrenOfType<RenderObject>(Downcast<RenderElement>(renderer)))
    {
      if (child.HasLayer())
      {
        continue;
      }
      Write(ts, child, behavior);
    }

    // FIXME: The render trees of frames (RenderWidget), once there are any.
  }

  namespace
  {
    enum LayerPaintPhase
    {
      LayerPaintPhaseAll = 0,
      LayerPaintPhaseBackground = -1,
      LayerPaintPhaseForeground = 1
    };

    template <typename TDumpRect>
    void WriteLayerUsingGeometryType(TextStream &ts, const RenderLayer &layer, const TDumpRect &layerBounds,
                                     const TDumpRect &backgroundClipRect, const TDumpRect &clipRect,
                                     LayerPaintPhase paintPhase, EnumSet<RenderAsTextFlag> behavior) noexcept
    {
      ts << Indent << "layer ";

      if (behavior.Contains(RenderAsTextFlag::ShowAddresses))
      {
        ts << static_cast<const void *>(&layer) << ' ';
        if (auto *scrollableArea = layer.GetScrollableArea())
        {
          ts << "scrollableArea " << static_cast<const void *>(scrollableArea) << ' ';
        }
      }

      ts << layerBounds;

      if (!layerBounds.IsEmpty())
      {
        if (!backgroundClipRect.Contains(layerBounds))
        {
          ts << " backgroundClip " << backgroundClipRect;
        }
        if (!clipRect.Contains(layerBounds))
        {
          ts << " clip " << clipRect;
        }
      }

      if (layer.Renderer().HasNonVisibleOverflow())
      {
        if (auto *scrollableArea = layer.GetScrollableArea())
        {
          if (scrollableArea->GetScrollOffset().X())
          {
            ts << " scrollX " << scrollableArea->GetScrollOffset().X();
          }
          if (scrollableArea->GetScrollOffset().Y())
          {
            ts << " scrollY " << scrollableArea->GetScrollOffset().Y();
          }
          if (layer.GetRenderBox()
              && RoundToInt(layer.GetRenderBox()->ClientWidth()) != scrollableArea->ScrollWidth())
          {
            ts << " scrollWidth " << scrollableArea->ScrollWidth();
          }
          if (layer.GetRenderBox()
              && RoundToInt(layer.GetRenderBox()->ClientHeight()) != scrollableArea->ScrollHeight())
          {
            ts << " scrollHeight " << scrollableArea->ScrollHeight();
          }
        }
      }

      if (paintPhase == LayerPaintPhaseBackground)
      {
        ts << " layerType: background only";
      }
      else if (paintPhase == LayerPaintPhaseForeground)
      {
        ts << " layerType: foreground only";
      }

      // FIXME: Composited layers (ShowCompositedLayers), once there's compositing.

      if (layer.IsolatesBlending())
      {
        ts << " isolatesBlending";
      }
      if (layer.HasBlendMode())
      {
        ts << " blendMode: " << BlendModeName(layer.GetBlendMode());
      }

      ts << '\n';
    }

    void WriteLayer(TextStream &ts, const RenderLayer &layer, const LayoutRect &layerBounds,
                    const LayoutRect &backgroundClipRect, const LayoutRect &clipRect,
                    LayerPaintPhase paintPhase = LayerPaintPhaseAll,
                    EnumSet<RenderAsTextFlag> behavior = {}) noexcept
    {
      if (ShouldEnableSubpixelPrecisionForTextDump(layer.Renderer().GetDocument()))
      {
        WriteLayerUsingGeometryType<LayoutRect>(ts, layer, layerBounds, backgroundClipRect, clipRect,
                                                paintPhase, behavior);
        return;
      }

      WriteLayerUsingGeometryType<IntRect>(ts, layer, SnappedIntRect(layerBounds),
                                           SnappedIntRect(backgroundClipRect), SnappedIntRect(clipRect),
                                           paintPhase, behavior);
    }

    void WriteLayerRenderers(TextStream &ts, const RenderLayer &layer, LayerPaintPhase paintPhase,
                             EnumSet<RenderAsTextFlag> behavior) noexcept
    {
      if (paintPhase != LayerPaintPhaseBackground)
      {
        TextStream::IndentScope indentScope(ts);
        Write(ts, layer.Renderer(), behavior);
      }
    }

    KRYS_NODISCARD LayoutSize MaxLayoutOverflow(const RenderBox *box) noexcept
    {
      LayoutRect overflowRect = box->LayoutOverflowRect();
      return LayoutSize(overflowRect.MaxX(), overflowRect.MaxY());
    }

    void WriteLayers(TextStream &ts, const RenderLayer &rootLayer, RenderLayer &layer,
                     const LayoutRect &paintRect, EnumSet<RenderAsTextFlag> behavior) noexcept
    {
      // FIXME: Apply overflow to the root layer to not break every test. Complete hack. Sigh.
      LayoutRect paintDirtyRect(paintRect);
      if (&rootLayer == &layer)
      {
        paintDirtyRect.SetWidth(std::max<LayoutUnit>(paintDirtyRect.Width(),
                                                     rootLayer.GetRenderBox()->LayoutOverflowRect().MaxX()));
        paintDirtyRect.SetHeight(std::max<LayoutUnit>(paintDirtyRect.Height(),
                                                      rootLayer.GetRenderBox()->LayoutOverflowRect().MaxY()));
        layer.SetSize(layer.Size().ExpandedTo(
          SnappedIntSize(MaxLayoutOverflow(layer.GetRenderBox()), LayoutPoint(0, 0))));
        layer.SetNeedsPositionUpdate();
      }

      // Calculate the clip rects we should use.
      LayoutSize offsetFromRoot = layer.OffsetFromAncestor(&rootLayer);
      RenderLayer::ClipRectsContext clipRectsContext(&rootLayer, PaintingClipRects,
                                                     RenderLayer::ClipRectTemporaryOptions);
      auto rects = layer.CalculateRects(clipRectsContext, offsetFromRoot, paintDirtyRect);

      // Ensure our lists are up-to-date.
      layer.UpdateLayerListsIfNeeded();
      layer.UpdateDescendantDependentFlags();

      bool shouldPaint =
        (behavior.Contains(RenderAsTextFlag::ShowAllLayers))
          ? true
          : layer.IntersectsDamageRect(rects.LayerBounds(), rects.DirtyBackgroundRect().Rect(), &rootLayer,
                                       layer.OffsetFromAncestor(&rootLayer));
      auto negativeZOrderLayers = layer.NegativeZOrderLayers();
      bool paintsBackgroundSeparately = negativeZOrderLayers.size() > 0;
      if (shouldPaint && paintsBackgroundSeparately)
      {
        WriteLayer(ts, layer, rects.LayerBounds(), rects.DirtyBackgroundRect().Rect(),
                   rects.DirtyForegroundRect().Rect(), LayerPaintPhaseBackground, behavior);
        WriteLayerRenderers(ts, layer, LayerPaintPhaseBackground, behavior);
      }

      if (negativeZOrderLayers.size())
      {
        if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
        {
          ts << Indent << " negative z-order list (" << static_cast<uint64>(negativeZOrderLayers.size())
             << ")\n";
          ts.IncreaseIndent();
        }

        for (auto *currLayer : negativeZOrderLayers)
        {
          WriteLayers(ts, rootLayer, *currLayer, paintDirtyRect, behavior);
        }

        if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
        {
          ts.DecreaseIndent();
        }
      }

      if (shouldPaint)
      {
        WriteLayer(ts, layer, rects.LayerBounds(), rects.DirtyBackgroundRect().Rect(),
                   rects.DirtyForegroundRect().Rect(),
                   paintsBackgroundSeparately ? LayerPaintPhaseForeground : LayerPaintPhaseAll, behavior);

        // FIXME: The layer's fragments (ShowLayerFragments, collectFragments), once there's fragmentation.

        WriteLayerRenderers(
          ts, layer, paintsBackgroundSeparately ? LayerPaintPhaseForeground : LayerPaintPhaseAll, behavior);
      }

      auto normalFlowLayers = layer.NormalFlowLayers();
      if (normalFlowLayers.size())
      {
        if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
        {
          ts << Indent << " normal flow list (" << static_cast<uint64>(normalFlowLayers.size()) << ")\n";
          ts.IncreaseIndent();
        }

        for (auto *currLayer : normalFlowLayers)
        {
          WriteLayers(ts, rootLayer, *currLayer, paintDirtyRect, behavior);
        }

        if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
        {
          ts.DecreaseIndent();
        }
      }

      auto positiveZOrderLayers = layer.PositiveZOrderLayers();
      if (positiveZOrderLayers.size())
      {
        size_t layerCount = positiveZOrderLayers.size();

        if (layerCount)
        {
          if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
          {
            ts << Indent << " positive z-order list (" << static_cast<uint64>(layerCount) << ")\n";
            ts.IncreaseIndent();
          }

          for (auto *currLayer : positiveZOrderLayers)
          {
            WriteLayers(ts, rootLayer, *currLayer, paintDirtyRect, behavior);
          }

          if (behavior.Contains(RenderAsTextFlag::ShowLayerNesting))
          {
            ts.DecreaseIndent();
          }
        }
      }
    }

    KRYS_NODISCARD TextStream CreateTextStream(const dom::Document &document) noexcept
    {
      auto formattingFlags = [&document]() -> EnumSet<TextStream::Formatting>
      {
        if (ShouldEnableSubpixelPrecisionForTextDump(document))
        {
          return {TextStream::Formatting::SVGStyleRect};
        }
        return {TextStream::Formatting::SVGStyleRect, TextStream::Formatting::LayoutUnitsAsIntegers};
      };

      return TextStream {TextStream::LineMode::MultipleLine, formattingFlags()};
    }

    KRYS_NODISCARD utf8_string ExternalRepresentation(RenderBox &renderer,
                                                      EnumSet<RenderAsTextFlag> behavior) noexcept
    {
      auto ts = CreateTextStream(renderer.GetDocument());
      if (!renderer.HasLayer())
      {
        return ts.Release();
      }

      RenderLayer &layer = *renderer.Layer();
      WriteLayers(ts, layer, layer, layer.Rect(), behavior);
      // FIXME: The caret or selection (writeSelection), once there's editing.
      return ts.Release();
    }
  }

  TextStream CreateTextStream(const RenderView &view) noexcept
  {
    return CreateTextStream(view.GetDocument());
  }

  utf8_string ExternalRepresentation(LocalFrame *frame, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    krys_debug_assert(frame);
    krys_debug_assert(frame->Document());

    auto *document = frame->Document();
    if (!behavior.Contains(RenderAsTextFlag::DontUpdateLayout))
    {
      // FIXME: Update the layout of the subframes too (updateLayoutAndStyleIfNeededRecursive), once there are
      // frames.
      (void)document->UpdateLayout();
    }

    auto *renderer = document->GetRenderView();
    if (!renderer)
    {
      return {};
    }

    // FIXME: Lay out for printing (PrintingMode), once there's printing.

    return ExternalRepresentation(*renderer, behavior);
  }

  void ExternalRepresentationForLocalFrame(TextStream &ts, LocalFrame &frame,
                                           EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    krys_debug_assert(frame.Document());

    if (auto *root = frame.Document()->GetRenderView())
    {
      if (auto *layer = root->Layer())
      {
        WriteLayers(ts, *layer, *layer, layer->Rect(), behavior);
      }
    }
  }

  utf8_string ExternalRepresentation(dom::Element *element, EnumSet<RenderAsTextFlag> behavior) noexcept
  {
    krys_debug_assert(element);

    // This function doesn't support printing mode.
    krys_debug_assert(!behavior.Contains(RenderAsTextFlag::PrintingMode));

    if (!behavior.Contains(RenderAsTextFlag::DontUpdateLayout))
    {
      (void)element->NodeDocument().UpdateLayout();
    }

    auto *renderer = DynamicDowncast<RenderBox>(element->Renderer());
    if (!renderer)
    {
      return {};
    }

    behavior.Add(RenderAsTextFlag::ShowAllLayers);
    return ExternalRepresentation(*renderer, behavior);
  }

  namespace
  {
    void WriteCounterValuesFromChildren(TextStream &stream, const RenderElement *parent,
                                        bool &isFirstCounter) noexcept
    {
      if (!parent)
      {
        return;
      }
      for (auto &counter : ChildrenOfType<RenderCounter>(*parent))
      {
        if (!isFirstCounter)
        {
          stream << " ";
        }
        isFirstCounter = false;
        stream << counter.Text();
      }
    }
  }

  utf8_string CounterValueForElement(dom::Element *element) noexcept
  {
    // Make sure the element is not freed during the layout.
    auto elementRef = ShareRef(*element);
    (void)element->NodeDocument().UpdateLayout();
    auto stream = CreateTextStream(element->NodeDocument());
    bool isFirstCounter = true;
    // The counter renderers should be children of :before or :after pseudo-elements.
    if (auto *before = element->BeforePseudoElement())
    {
      WriteCounterValuesFromChildren(stream, before->Renderer(), isFirstCounter);
    }
    if (auto *after = element->AfterPseudoElement())
    {
      WriteCounterValuesFromChildren(stream, after->Renderer(), isFirstCounter);
    }
    return stream.Release();
  }

  utf8_string MarkerTextForListItem(dom::Element *element) noexcept
  {
    // Make sure the element is not freed during the layout.
    auto protectedElement = ShareRef(*element);
    (void)element->NodeDocument().UpdateLayout();

    auto *renderer = DynamicDowncast<RenderListItem>(element->Renderer());
    if (!renderer)
    {
      return {};
    }
    return utf8_string(renderer->MarkerTextWithoutSuffix());
  }
}
