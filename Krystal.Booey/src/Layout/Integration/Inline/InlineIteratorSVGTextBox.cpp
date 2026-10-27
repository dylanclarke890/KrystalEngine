#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"

namespace krys::boo::inline_iterator
{
  SVGTextBox::SVGTextBox(PathVariant &&path) noexcept : TextBox(krys::move(path))
  {
  }

  FloatRect SVGTextBox::CalculateBoundariesIncludingSVGTransform() const noexcept
  {
    FloatRect textRect;

    float scalingFactor = Renderer().ScalingFactor();
    krys_debug_assert(scalingFactor);

    float baseline = Renderer().ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;
    for (auto &fragment : TextFragments())
    {
      auto fragmentRect = FloatRect {fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height};

      AffineTransform fragmentTransform;
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      textRect.Unite(fragmentRect);
    }
    return textRect;
  }

  LayoutRect SVGTextBox::LocalSelectionRect(uint32 start, uint32 end) const noexcept
  {
    auto [clampedStart, clampedEnd] = SelectableRange().Clamp(start, end);

    if (clampedStart >= clampedEnd)
    {
      return LayoutRect();
    }

    auto &style = Renderer().Style();

    AffineTransform fragmentTransform;
    FloatRect selectionRect;
    uint32 fragmentStartPosition = 0;
    uint32 fragmentEndPosition = 0;

    for (auto &fragment : TextFragments())
    {
      fragmentStartPosition = clampedStart;
      fragmentEndPosition = clampedEnd;
      if (!MapStartEndPositionsIntoFragmentCoordinates(Start(), fragment, fragmentStartPosition,
                                                       fragmentEndPosition))
      {
        continue;
      }

      FloatRect fragmentRect = SelectionRectForTextFragment(
        Renderer(), Direction(), fragment, fragmentStartPosition, fragmentEndPosition, style);
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      selectionRect.Unite(fragmentRect);
    }

    return EnclosingIntRect(selectionRect);
  }

  const List<SVGTextFragment> &SVGTextBox::TextFragments() const noexcept
  {
    return SwitchOn(_pathVariant, [&](const auto &path) -> const List<SVGTextFragment> &
                    { return path.SvgTextFragments(); });
  }

  const SVGInlineTextBox *SVGTextBox::GetLegacyInlineBox() const noexcept
  {
    return Downcast<SVGInlineTextBox>(TextBox::GetLegacyInlineBox());
  }

  SVGTextBoxIterator::SVGTextBoxIterator(Box::PathVariant &&path) noexcept : TextBoxIterator(krys::move(path))
  {
  }

  SVGTextBoxIterator::SVGTextBoxIterator(const Box &box) noexcept : TextBoxIterator(box)
  {
  }

  SVGTextBoxIterator FirstSVGTextBoxFor(const RenderSVGInlineText &text) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(text))
    {
      auto box = lineLayout->TextBoxesFor(text);
      if (!box)
      {
        return {};
      }
      return SVGTextBoxIterator {*box};
    }

    return SVGTextBoxIterator {BoxLegacyPath {text.FirstLegacyTextBox()}};
  }

  BoxRange<SVGTextBoxIterator> SVGTextBoxesFor(const RenderSVGInlineText &text) noexcept
  {
    return BoxRange<SVGTextBoxIterator> {FirstSVGTextBoxFor(text)};
  }

  SVGTextBoxIterator SVGTextBoxFor(const SVGInlineTextBox *box) noexcept
  {
    return SVGTextBoxIterator {BoxLegacyPath {box}};
  }

  SVGTextBoxIterator SVGTextBoxFor(const layout_integration::InlineContent &inlineContent,
                                   size_t boxIndex) noexcept
  {
    auto &box = inlineContent.DisplayContent().BoxList[boxIndex];
    if (!box.IsText() || !box.LayoutBox().RendererForIntegration()->IsRenderSVGInlineText())
    {
      return {};
    }
    return SVGTextBoxIterator {BoxModernPath {inlineContent, boxIndex}};
  }

  SVGTextBox::Key MakeKey(const SVGTextBox &textBox) noexcept
  {
    return {&textBox.Renderer(), textBox.Start()};
  }

  BoxRange<BoxIterator> BoxesFor(const RenderSVGText &svgText) noexcept
  {
    if (auto *lineLayout = svgText.InlineLayout())
    {
      return BoxRange<BoxIterator> {BoxIterator {*lineLayout->FirstRootInlineBox()}};
    }

    return BoxRange<BoxIterator> {BoxIterator {BoxLegacyPath {svgText.LegacyRootBox()}}};
  }
}
