#include "Krystal.Booey/Rendering/RenderTextControl.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  RenderTextControl::RenderTextControl(Type type, html::HTMLTextFormControlElement &element,
                                       RenderStyle &&style) noexcept
      : RenderBlockFlow(type, element, krys::move(style), {BlockFlowFlag::IsTextControl})
  {
    krys_debug_assert(IsRenderTextControl());
  }

  RenderTextControl::~RenderTextControl() noexcept = default;

  html::HTMLTextFormControlElement &RenderTextControl::TextFormControlElement() const noexcept
  {
    return Downcast<html::HTMLTextFormControlElement>(*GetElement());
  }

  RefPtr<html::TextControlInnerTextElement> RenderTextControl::InnerTextElement() const noexcept
  {
    return TextFormControlElement().InnerTextElement();
  }

  void RenderTextControl::StyleDidChange(style::Difference diff, const RenderStyle *oldStyle) noexcept
  {
    RenderBlockFlow::StyleDidChange(diff, oldStyle);
    auto innerText = InnerTextElement();
    if (!innerText)
    {
      return;
    }
    auto *innerTextRenderer = innerText->Renderer();
    if (innerTextRenderer && oldStyle)
    {
      // FIXME: The height property of the inner text block style may be mutated by
      // RenderTextControlSingleLine::layout. See if the original has changed before setting it and triggering
      // a layout.
      auto newInnerTextStyle = TextFormControlElement().CreateInnerTextStyle(Style());
      auto oldInnerTextStyle = TextFormControlElement().CreateInnerTextStyle(*oldStyle);
      if (newInnerTextStyle != oldInnerTextStyle)
      {
        innerTextRenderer->SetStyle(krys::move(newInnerTextStyle));
      }
      else if (diff == style::DifferenceResult::RepaintIfText || diff == style::DifferenceResult::Repaint)
      {
        // Repaint is expected to be propagated down to the shadow tree when non-inherited style property
        // changes (e.g. text-decoration-color) since that's where the value actually takes effect.
        innerTextRenderer->Repaint();
      }
    }
    TextFormControlElement().UpdatePlaceholderVisibility();
  }

  int32 RenderTextControl::ScrollbarThickness() const noexcept
  {
    // FIXME: We should get the size of the scrollbar from the RenderTheme instead.
    return ScrollbarTheme::Theme().ScrollbarThickness(
      style::ToPlatform<style::ScrollbarWidth> {}(Style().GetScrollbarWidth()),
      OverlayScrollbarSizeRelevancy::IgnoreOverlayScrollbarSize);
  }

  RenderBox::LogicalExtentComputedValues
    RenderTextControl::ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept
  {
    auto innerText = InnerTextElement();
    if (!innerText)
    {
      return RenderBox::ComputeLogicalHeight(LayoutUnit(), LayoutUnit());
    }

    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      auto logicalHeightExtent = RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
      if (Style().LogicalHeight().IsSpecified())
      {
        return logicalHeightExtent;
      }

      auto placeholderLogicalHeight = [&]() -> LayoutUnit
      {
        auto *placeholder = TextFormControlElement().PlaceholderElement();
        if (!placeholder)
        {
          return {};
        }
        auto *placeholderBox = placeholder->GetRenderBox();
        if (!placeholderBox)
        {
          return {};
        }
        return placeholderBox
          ->ComputeLogicalHeight(placeholderBox->LogicalHeight(), placeholderBox->LogicalTop())
          .Extent;
      };
      logicalHeightExtent.Extent = std::max(logicalHeightExtent.Extent, placeholderLogicalHeight());
      return logicalHeightExtent;
    }

    if (auto *innerTextBox = innerText->GetRenderBox())
    {
      LayoutUnit nonContentHeight =
        innerTextBox->BorderAndPaddingLogicalHeight() + innerTextBox->MarginLogicalHeight();
      logicalHeight = ComputeControlLogicalHeight(innerTextBox->LineHeight(), nonContentHeight);

      // We are able to have a horizontal scrollbar if the overflow style is scroll, or if its auto and
      // there's no word wrap.
      auto shouldIncludeScrollbarHeight = [&]
      {
        auto &style = Style();
        auto isHorizontalWritingMode = IsHorizontalWritingMode();
        return (isHorizontalWritingMode && style.OverflowX() == Overflow::Scroll)
               || (!isHorizontalWritingMode && style.OverflowY() == Overflow::Scroll);
      };
      if (shouldIncludeScrollbarHeight())
      {
        logicalHeight += ScrollbarThickness();
      }

      // FIXME: The logical height of the inner text box should have been added
      // before calling computeLogicalHeight to avoid this hack.
      CacheIntrinsicContentLogicalHeightForFlexItem(logicalHeight);

      logicalHeight += BorderAndPaddingLogicalHeight();
    }

    return RenderBox::ComputeLogicalHeight(logicalHeight, logicalTop);
  }

  void RenderTextControl::HitInnerTextElement(HitTestResult &result, const LayoutPoint &pointInContainer,
                                              const LayoutPoint &accumulatedOffset) noexcept
  {
    auto innerText = InnerTextElement();
    if (!innerText->Renderer())
    {
      return;
    }

    LayoutPoint adjustedLocation = accumulatedOffset + Location();
    LayoutPoint localPoint = pointInContainer
                             - ToLayoutSize(adjustedLocation + innerText->GetRenderBox()->Location())
                             + ToLayoutSize(GetScrollPosition());
    result.SetInnerNode(innerText.get());
    result.SetInnerNonSharedNode(innerText.get());
    result.SetLocalPoint(localPoint);
  }

  float RenderTextControl::GetAverageCharWidth() noexcept
  {
    float width;
    if (Style().GetFontCascade().FastAverageCharWidthIfAvailable(width))
    {
      return width;
    }

    const auto &font = Style().GetFontCascade();
    auto textRun = ConstructTextRun(u8"0", Style(), ExpansionBehavior::AllowRightOnly());
    return font.Width(textRun);
  }

  float RenderTextControl::ScaleEmToUnits(int32 x) const noexcept
  {
    // This matches the unitsPerEm value for MS Shell Dlg and Courier New from the "head" font table.
    float unitsPerEm = 2048.0f;
    return std::roundf(Style().GetFontCascade().Size() * x / unitsPerEm);
  }

  void RenderTextControl::ComputeIntrinsicLogicalWidths(LayoutUnit &minLogicalWidth,
                                                        LayoutUnit &maxLogicalWidth) const noexcept
  {
    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      return RenderBlockFlow::ComputeIntrinsicLogicalWidths(minLogicalWidth, maxLogicalWidth);
    }

    if (ShouldApplySizeOrInlineSizeContainment())
    {
      if (auto width = ExplicitIntrinsicInnerLogicalWidth())
      {
        minLogicalWidth = *width;
        maxLogicalWidth = *width;
      }
      return;
    }
    // Use average character width. Matches IE.
    maxLogicalWidth =
      PreferredContentLogicalWidth(const_cast<RenderTextControl *>(this)->GetAverageCharWidth());
    // FIXME: The theme's maximum width for the control (adjustedMaximumLogicalWidthForControl), with the
    // theme.

    auto &logicalWidth = Style().LogicalWidth();
    if (logicalWidth.IsCalculated())
    {
      minLogicalWidth =
        std::max(0_lu, style::Evaluate<LayoutUnit>(logicalWidth, 0_lu, Style().UsedZoomForLength()));
    }
    else if (!logicalWidth.IsPercent())
    {
      minLogicalWidth = maxLogicalWidth;
    }
  }

  void RenderTextControl::ComputePreferredLogicalWidths() noexcept
  {
    krys_debug_assert(NeedsPreferredLogicalWidthsUpdate());
    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      RenderBlockFlow::ComputePreferredLogicalWidths();
      return;
    }

    _minPreferredLogicalWidth = 0_lu;
    _maxPreferredLogicalWidth = 0_lu;

    if (auto fixedLogicalWidth = Style().LogicalWidth().TryFixed();
        fixedLogicalWidth && fixedLogicalWidth->IsPositiveOrZero())
    {
      _minPreferredLogicalWidth = _maxPreferredLogicalWidth = AdjustContentBoxLogicalWidthForBoxSizing(
        LayoutUnit {fixedLogicalWidth->ResolveZoom(Style().UsedZoomForLength())});
    }
    else
    {
      ComputeIntrinsicLogicalWidths(_minPreferredLogicalWidth, _maxPreferredLogicalWidth);
    }

    RenderBox::ComputePreferredLogicalWidths(Style().LogicalMinWidth(), Style().LogicalMaxWidth(),
                                             BorderAndPaddingLogicalWidth());

    ClearNeedsPreferredWidthsUpdate();
  }

  void RenderTextControl::LayoutExcludedChildren(RelayoutChildren relayoutChildren) noexcept
  {
    RenderBlockFlow::LayoutExcludedChildren(relayoutChildren);

    auto *placeholder = TextFormControlElement().PlaceholderElement();
    if (!placeholder)
    {
      return;
    }

    auto *placeholderRenderer = placeholder->Renderer();
    if (!placeholderRenderer)
    {
      return;
    }

    placeholderRenderer->SetIsExcludedFromNormalLayout(true);

    if (Style().GetFieldSizing() == FieldSizing::Content)
    {
      // In order to take placeholder height into account while computing the size of the input box, we need
      // to layout the placeholder too (which is how excluded content normally works).
      placeholderRenderer->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      placeholderRenderer->LayoutIfNeeded();
    }

    if (relayoutChildren == RelayoutChildren::Yes)
    {
      // The markParents arguments should be false because this function is
      // called from layout() of the parent and the placeholder layout doesn't
      // affect the parent layout.
      placeholderRenderer->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
    }
  }

  RenderTextControlInnerContainer::RenderTextControlInnerContainer(dom::Element &element,
                                                                   RenderStyle &&style) noexcept
      : RenderFlexibleBox(Type::TextControlInnerContainer, element, krys::move(style))
  {
  }

  RenderTextControlInnerContainer::~RenderTextControlInnerContainer() noexcept = default;
}
