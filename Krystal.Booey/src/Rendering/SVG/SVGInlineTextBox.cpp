#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"

#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include <algorithm>

namespace krys::boo
{
  SVGInlineTextBox::SVGInlineTextBox(RenderSVGInlineText &renderer) noexcept : LegacyInlineTextBox(renderer)
  {
  }

  SVGInlineTextBox *SVGInlineTextBox::NextTextBox() const noexcept
  {
    return Downcast<SVGInlineTextBox>(LegacyInlineTextBox::NextTextBox());
  }

  void SVGInlineTextBox::DirtyOwnLineBoxes() noexcept
  {
    LegacyInlineTextBox::DirtyLineBoxes();

    _textFragments = {};
  }

  void SVGInlineTextBox::DirtyLineBoxes() noexcept
  {
    DirtyOwnLineBoxes();

    // And clear any following text fragments as the text on which they
    // depend may now no longer exist, or glyph positions may be wrong
    for (auto *nextBox = NextTextBox(); nextBox; nextBox = nextBox->NextTextBox())
    {
      nextBox->DirtyOwnLineBoxes();
    }
  }

  bool SVGInlineTextBox::MapStartEndPositionsIntoFragmentCoordinates(const SVGTextFragment &fragment,
                                                                     uint32 &startPosition,
                                                                     uint32 &endPosition) const noexcept
  {
    uint32 startFragment = fragment.CharacterOffset - Start();
    uint32 endFragment = startFragment + fragment.Length;

    // Find intersection between the intervals: [startFragment..endFragment) and [startPosition..endPosition)
    startPosition = std::max(startFragment, startPosition);
    endPosition = std::min(endFragment, endPosition);

    if (startPosition >= endPosition)
    {
      return false;
    }

    startPosition -= startFragment;
    endPosition -= startFragment;

    return true;
  }

  FloatRect SVGInlineTextBox::CalculateBoundaries() const noexcept
  {
    FloatRect textRect;

    float scalingFactor = Renderer().ScalingFactor();
    krys_debug_assert(scalingFactor);

    float baseline = Renderer().ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;

    AffineTransform fragmentTransform;
    for (const auto &fragment : _textFragments)
    {
      FloatRect fragmentRect(fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height);
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      textRect.Unite(fragmentRect);
    }

    return textRect;
  }

  void SVGInlineTextBox::SetTextFragments(List<SVGTextFragment> &&fragments) noexcept
  {
    _textFragments = krys::move(fragments);
  }
}
