#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderingContext.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextBoxPainter.hpp"
#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD utf8_string ApplySVGWhitespaceRules(utf8_stringview string,
                                                       bool preserveWhiteSpace) noexcept
    {
      utf8_string newString;
      newString.reserve(string.size());
      if (preserveWhiteSpace)
      {
        // Spec: When xml:space="preserve", the SVG user agent will do the following using a
        // copy of the original character data content. It will convert all newline and tab
        // characters into space characters. Then, it will draw all space characters, including
        // leading, trailing and multiple contiguous space characters.
        for (auto character : string)
        {
          newString.push_back(character == u8'\t' || character == u8'\n' || character == u8'\r' ? u8' '
                                                                                                : character);
        }
        return newString;
      }

      // Spec: When xml:space="default", the SVG user agent will do the following using a
      // copy of the original character data content. First, it will remove all newline
      // characters. Then it will convert all tab characters into space characters.
      // Then, it will strip off all leading and trailing space characters.
      // Then, all contiguous space characters will be consolidated.
      for (auto character : string)
      {
        if (character == u8'\n' || character == u8'\r')
        {
          continue;
        }
        newString.push_back(character == u8'\t' ? u8' ' : character);
      }
      return newString;
    }
  }

  RenderSVGInlineText::RenderSVGInlineText(dom::Text &textNode, const utf8_string &string) noexcept
      : RenderText(Type::SVGInlineText, textNode, ApplySVGWhitespaceRules(string, false)),
        _layoutAttributes(*this)
  {
    krys_debug_assert(IsRenderSVGInlineText());
  }

  RenderSVGInlineText::~RenderSVGInlineText() noexcept = default;

  void RenderSVGInlineText::WillBeDestroyed() noexcept
  {
    RemoveAndDestroyLegacyTextBoxes();
    RenderText::WillBeDestroyed();
  }

  utf8_string RenderSVGInlineText::OriginalText() const noexcept
  {
    return TextNode().Data();
  }

  void RenderSVGInlineText::SetTextInternal(utf8_string newText, bool force) noexcept
  {
    RenderText::SetTextInternal(newText, force);
    _legacyLineBoxes.DirtyForTextChange(*this);
    if (!force)
    {
      SetRenderedText(newText);
    }
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      textAncestor->SubtreeTextDidChange(this);
    }
  }

  void RenderSVGInlineText::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderText::StyleDidChange(diff, oldStyle);
    UpdateScaledFont();

    bool newPreserves = Style().GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve;
    bool oldPreserves = oldStyle ? oldStyle->GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve : false;
    if (oldPreserves && !newPreserves)
    {
      SetText(ApplySVGWhitespaceRules(OriginalText(), false), true);
      return;
    }

    if (!oldPreserves && newPreserves)
    {
      SetText(ApplySVGWhitespaceRules(OriginalText(), true), true);
      return;
    }

    if (diff != style::DifferenceResult::Layout)
    {
      return;
    }

    // The text metrics may be influenced by style changes.
    if (auto *textAncestor = RenderSVGText::LocateRenderSVGTextAncestor(*this))
    {
      textAncestor->SetNeedsLayout();
    }
  }

  UniquePtr<LegacyInlineTextBox> RenderSVGInlineText::CreateTextBox() noexcept
  {
    auto box = CreateUnique<SVGInlineTextBox>(*this);
    box->SetHasVirtualLogicalHeight();
    return box;
  }

  FloatRect RenderSVGInlineText::FloatLinesBoundingBox() const noexcept
  {
    FloatRect boundingBox;
    for (auto &box : inline_iterator::SVGTextBoxesFor(*this))
    {
      boundingBox.Unite(box.CalculateBoundariesIncludingSVGTransform());
    }

    return boundingBox;
  }

  IntRect RenderSVGInlineText::LinesBoundingBox() const noexcept
  {
    return EnclosingIntRect(FloatLinesBoundingBox());
  }

  bool RenderSVGInlineText::CharacterStartsNewTextChunk(int32 position) const noexcept
  {
    krys_debug_assert(position >= 0);
    krys_debug_assert(position < static_cast<int32>(Text().size()));

    // Each <textPath> element starts a new text chunk, regardless of any x/y values.
    if (!position && Parent()->IsRenderSVGTextPath() && !PreviousSibling())
    {
      return true;
    }

    auto it = _layoutAttributes.CharacterDataMap().find(static_cast<uint32>(position + 1));
    if (it == _layoutAttributes.CharacterDataMap().end())
    {
      return false;
    }

    return !SVGTextLayoutAttributes::IsEmptyValue(it->second.X)
           || !SVGTextLayoutAttributes::IsEmptyValue(it->second.Y);
  }

  namespace
  {
    KRYS_NODISCARD int32 OffsetForPositionInFragment(const inline_iterator::SVGTextBox &textBox,
                                                     const SVGTextFragment &fragment, float position) noexcept
    {
      float scalingFactor = textBox.Renderer().ScalingFactor();
      krys_debug_assert(scalingFactor);

      TextRun textRun =
        ConstructTextRun(textBox.Renderer().Text(), textBox.Direction(), textBox.Style(), fragment);

      // Eventually handle lengthAdjust="spacingAndGlyphs".
      // FIXME: Handle vertical text.
      AffineTransform fragmentTransform;
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        textRun.SetHorizontalGlyphStretch(static_cast<float>(fragmentTransform.XScale()));
      }

      const bool includePartialGlyphs = true;
      return static_cast<int32>(fragment.CharacterOffset - textBox.Start())
             + textBox.Renderer().ScaledFont().OffsetForPosition(textRun, position * scalingFactor,
                                                                 includePartialGlyphs);
    }
  }

  dom::PositionWithAffinity RenderSVGInlineText::PositionForPoint(const LayoutPoint &point,
                                                                  HitTestSource) noexcept
  {
    if (!inline_iterator::LineLeftmostTextBoxFor(*this) || Text().empty())
    {
      return CreatePositionWithAffinity(0, Affinity::Downstream);
    }

    float baseline = _scaledFont.MetricsOfPrimaryFont().Ascent();

    RenderBlock *containingBlock = ContainingBlock();
    krys_debug_assert(containingBlock);

    // Map local point to absolute point, as the character origins stored in the text fragments use absolute
    // coordinates.
    FloatPoint absolutePoint(point);
    absolutePoint.MoveBy(containingBlock->Location());

    float closestDistance = std::numeric_limits<float>::max();
    float closestDistancePosition = 0;
    const SVGTextFragment *closestDistanceFragment = nullptr;
    inline_iterator::SVGTextBoxIterator closestDistanceBox;

    AffineTransform fragmentTransform;
    for (auto &box : inline_iterator::SVGTextBoxesFor(*this))
    {
      auto &fragments = box.TextFragments();

      for (auto &fragment : fragments)
      {
        FloatRect fragmentRect(fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height);
        fragment.BuildFragmentTransform(fragmentTransform);
        if (!fragmentTransform.IsIdentity())
        {
          fragmentRect = fragmentTransform.MapRect(fragmentRect);
        }

        float distance = std::pow(fragmentRect.X() - absolutePoint.X(), 2.f)
                         + std::pow(fragmentRect.Y() + fragmentRect.Height() / 2 - absolutePoint.Y(), 2.f);

        if (distance < closestDistance)
        {
          closestDistance = distance;
          closestDistanceBox = inline_iterator::SVGTextBoxIterator {box};
          closestDistanceFragment = &fragment;
          closestDistancePosition = fragmentRect.X();
        }
      }
    }

    if (!closestDistanceFragment)
    {
      return CreatePositionWithAffinity(0, Affinity::Downstream);
    }

    int32 offset = OffsetForPositionInFragment(*closestDistanceBox, *closestDistanceFragment,
                                               absolutePoint.X() - closestDistancePosition);
    return CreatePositionWithAffinity(offset + static_cast<int32>(closestDistanceBox->Start()),
                                      offset > 0 ? Affinity::Upstream : Affinity::Downstream);
  }

  void RenderSVGInlineText::UpdateScaledFont() noexcept
  {
    if (ComputeNewScaledFontForStyle(*this, Style(), _scalingFactor, _scaledFont))
    {
      _canUseSimplifiedTextMeasuring = null;
    }
  }

  float RenderSVGInlineText::ComputeScalingFactorForRenderer(const RenderObject &renderer) noexcept
  {
    return SVGRenderingContext::CalculateScreenFontSizeScalingFactor(renderer);
  }

  bool RenderSVGInlineText::ComputeNewScaledFontForStyle(const RenderObject &renderer,
                                                         const RenderStyle &style, float &scalingFactor,
                                                         FontCascade &scaledFont) noexcept
  {
    // Alter font-size to the right on-screen value to avoid scaling the glyphs themselves, except when
    // GeometricPrecision is specified.
    scalingFactor = ComputeScalingFactorForRenderer(renderer);
    if (!scalingFactor)
    {
      scalingFactor = 1;
      scaledFont = style.GetFontCascade();
      return false;
    }

    if (style.GetFontDescription().GetTextRenderingMode() == TextRenderingMode::GeometricPrecision)
    {
      scalingFactor = 1;
    }

    auto fontDescription = style.GetFontDescription();

    // FIXME: We need to better handle the case when we compute very small fonts below (below 1pt).
    fontDescription.SetComputedSize(style::ComputedFontSizeFromSpecifiedSizeForSVGInlineText(
      fontDescription.SpecifiedSize(), fontDescription.IsAbsoluteSize(), scalingFactor,
      renderer.GetDocument()));

    // SVG controls its own glyph orientation, so don't allow writing-mode
    // to affect it.
    if (fontDescription.Orientation() != FontOrientation::Horizontal)
    {
      fontDescription.SetOrientation(FontOrientation::Horizontal);
    }

    scaledFont = FontCascade(krys::move(fontDescription));
    scaledFont.Update(ShareRefPtr<FontSelector>(&renderer.GetDocument().GetFontSelector()));
    return true;
  }

  void RenderSVGInlineText::DeleteLegacyLineBoxes() noexcept
  {
    _legacyLineBoxes.DeleteAll();
  }

  void RenderSVGInlineText::RemoveAndDestroyLegacyTextBoxes() noexcept
  {
    if (!RenderTreeBeingDestroyed())
    {
      _legacyLineBoxes.RemoveAllFromParent(*this);
    }
    DeleteLegacyLineBoxes();
  }
}
