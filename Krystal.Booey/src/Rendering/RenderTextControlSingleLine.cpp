#include "Krystal.Booey/Rendering/RenderTextControlSingleLine.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Graphics/FontCascade.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Shadow/TextControlInnerElements.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Rendering/HitTestLocation.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/LayoutScope.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerScrollableArea.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    void ResetOverriddenHeight(RenderBox *box, const RenderObject *ancestor) noexcept
    {
      krys_debug_assert(box != ancestor);
      if (!box || box->Style().LogicalHeight().IsAuto())
      {
        return; // Null box or its height was not overridden.
      }
      box->MutableStyle().SetLogicalHeight(css::keywords::Auto {});
      for (RenderObject *renderer = box; renderer != ancestor; renderer = renderer->Parent())
      {
        krys_debug_assert(renderer);
        renderer->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
    }
  }

  RenderTextControlSingleLine::RenderTextControlSingleLine(Type type, html::HTMLInputElement &element,
                                                           RenderStyle &&style) noexcept
      : RenderTextControl(type, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTextControlSingleLine());
  }

  RenderTextControlSingleLine::~RenderTextControlSingleLine() noexcept = default;

  html::HTMLElement *RenderTextControlSingleLine::ContainerElement() const noexcept
  {
    return InputElement().ContainerElement();
  }

  html::HTMLElement *RenderTextControlSingleLine::InnerBlockElement() const noexcept
  {
    return InputElement().InnerBlockElement();
  }

  html::HTMLElement *RenderTextControlSingleLine::InnerSpinButtonElement() const noexcept
  {
    return InputElement().InnerSpinButtonElement();
  }

  void RenderTextControlSingleLine::Layout() noexcept
  {
    // FIXME: We should remove the height-related hacks in layout() and
    // styleDidChange(). We need them because we want to:
    // - Center the inner elements vertically if the input height is taller than
    //   the intrinsic height of the inner elements.
    // - Shrink the heights of the inner elements if the input height is smaller
    //   than the intrinsic heights of the inner elements.
    // - Make the height of the container element equal to the intrinsic height of
    //   the inner elements when the field has a strong password button.
    //
    // We don't honor padding and borders for textfields without decorations
    // and type=search if the text height is taller than the contentBoxHeight()
    // because of compability.

    auto innerText = InnerTextElement();
    RenderTextControlInnerBlock *innerTextRenderer = innerText ? innerText->Renderer() : nullptr;
    RenderBox *innerBlockRenderer = InnerBlockElement() ? InnerBlockElement()->GetRenderBox() : nullptr;
    html::HTMLElement *container = ContainerElement();
    RenderBox *containerRenderer = container ? container->GetRenderBox() : nullptr;

    // To ensure consistency between layouts, we need to reset any conditionally overridden height.
    ResetOverriddenHeight(innerTextRenderer, this);
    ResetOverriddenHeight(innerBlockRenderer, this);
    ResetOverriddenHeight(containerRenderer, this);

    // Save the old size of the inner text (if we have one) as we will need to layout the placeholder
    // and update selection if it changes. One way the size may change is if text decorations are
    // toggled. For example, hiding and showing the caps lock indicator will cause a size change.
    LayoutSize oldInnerTextSize;
    if (innerTextRenderer)
    {
      oldInnerTextSize = innerTextRenderer->Size();
    }

    {
      auto scope = LayoutScope {*this};
      RenderBlockFlow::LayoutBlock(RelayoutChildren::No);
    }

    // Set the text block height
    LayoutUnit inputContentBoxLogicalHeight = LogicalHeight() - BorderAndPaddingLogicalHeight();
    LayoutUnit logicalHeightLimit = LogicalHeight();
    LayoutUnit innerTextLogicalHeight;
    if (innerTextRenderer)
    {
      innerTextLogicalHeight = innerTextRenderer->LogicalHeight();
    }

    auto shrinkInnerTextRendererIfNeeded = [&]
    {
      if (!innerTextRenderer || innerTextLogicalHeight <= logicalHeightLimit)
      {
        return;
      }
      // Let the simple (non-container based) inner text overflow (and clip) to be able to center it.
      if (!containerRenderer)
      {
        return;
      }

      if (inputContentBoxLogicalHeight != innerTextLogicalHeight)
      {
        SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }

      innerTextRenderer->MutableStyle().SetLogicalHeight(
        style::PreferredSize::Fixed {inputContentBoxLogicalHeight.ToFloat()});
      innerTextRenderer->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      if (innerBlockRenderer)
      {
        innerBlockRenderer->MutableStyle().SetLogicalHeight(
          style::PreferredSize::Fixed {inputContentBoxLogicalHeight.ToFloat()});
        innerBlockRenderer->SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      innerTextLogicalHeight = inputContentBoxLogicalHeight;
    };
    shrinkInnerTextRendererIfNeeded();

    // The container might be taller because of decoration elements.
    if (containerRenderer)
    {
      containerRenderer->LayoutIfNeeded();
      LayoutUnit containerLogicalHeight = containerRenderer->LogicalHeight();

      // FIXME: The strong password AutoFill button's height, with autofill.
      auto usedZoomForLength = containerRenderer->Style().UsedZoomForLength().Value;
      if (containerLogicalHeight > logicalHeightLimit)
      {
        containerRenderer->MutableStyle().SetLogicalHeight(
          style::PreferredSize::Fixed {logicalHeightLimit.ToFloat() / usedZoomForLength});
        SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      else if (containerRenderer->LogicalHeight() < ContentBoxLogicalHeight())
      {
        containerRenderer->MutableStyle().SetLogicalHeight(
          style::PreferredSize::Fixed {ContentBoxLogicalHeight().ToFloat() / usedZoomForLength});
        SetNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      else
      {
        containerRenderer->MutableStyle().SetLogicalHeight(
          style::PreferredSize::Fixed {containerLogicalHeight.ToFloat() / usedZoomForLength});
      }
    }

    // If we need another layout pass, we have changed one of children's height so we need to relayout them.
    if (NeedsLayout())
    {
      auto scope = LayoutScope {*this};
      RenderBlockFlow::LayoutBlock(RelayoutChildren::Yes);
    }

    // Center the child block in the block progression direction (vertical centering for horizontal text
    // fields).
    auto centerRendererIfNeeded = [&](RenderBox &renderer)
    {
      auto height = [&]
      {
        if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(renderer))
        {
          if (auto lineBox = inline_iterator::FirstLineBoxFor(*blockFlow))
          {
            return LayoutUnit {std::max(lineBox->LogicalHeight(), lineBox->ContentLogicalHeight())};
          }
        }
        return renderer.LogicalHeight();
      }();
      auto contentBoxHeight = ContentBoxLogicalHeight();
      if (contentBoxHeight == height)
      {
        return;
      }
      renderer.SetLogicalTop(renderer.LogicalTop() + (contentBoxHeight / 2 - height / 2));
    };
    if (!container && innerTextRenderer)
    {
      centerRendererIfNeeded(*innerTextRenderer);
    }
    else if (container && containerRenderer)
    {
      centerRendererIfNeeded(*containerRenderer);
    }

    bool innerTextSizeChanged = innerTextRenderer && innerTextRenderer->Size() != oldInnerTextSize;

    html::HTMLElement *placeholderElement = InputElement().PlaceholderElement();
    if (RenderBox *placeholderBox = placeholderElement ? placeholderElement->GetRenderBox() : nullptr)
    {
      auto innerTextWidth = LayoutUnit {};
      auto usedZoomForLength = placeholderBox->Style().UsedZoomForLength().Value;
      if (innerTextRenderer)
      {
        innerTextWidth = innerTextRenderer->LogicalWidth();
      }
      placeholderBox->MutableStyle().SetWidth(style::PreferredSize::Fixed {
        (innerTextWidth - placeholderBox->HorizontalBorderAndPaddingExtent()).ToFloat() / usedZoomForLength});
      bool neededLayout = placeholderBox->NeedsLayout();
      bool placeholderBoxHadLayout = placeholderBox->EverHadLayout();
      if (innerTextSizeChanged)
      {
        // The caps lock indicator was hidden. Layout the placeholder. Its layout does not affect its parent.
        placeholderBox->SetChildNeedsLayout(RenderObjectMarkingBehavior::MarkOnlyThis);
      }
      placeholderBox->LayoutIfNeeded();
      auto placeholderTopLeft = containerRenderer ? containerRenderer->Location() : LayoutPoint {};
      auto *innerBlockBox = InnerBlockElement() ? InnerBlockElement()->GetRenderBox() : nullptr;
      if (innerBlockBox)
      {
        placeholderTopLeft += ToLayoutSize(innerBlockBox->Location());
      }
      if (innerTextRenderer)
      {
        placeholderTopLeft += ToLayoutSize(innerTextRenderer->Location());
      }
      placeholderBox->SetLogicalLeft(placeholderTopLeft.X());
      // Here the container box indicates the renderer that the placeholder content is aligned with (no parent
      // and/or containing block relationship).
      RenderBox *containerBox = innerTextRenderer ? static_cast<RenderBox *>(innerTextRenderer)
                                : innerBlockBox   ? innerBlockBox
                                                  : containerRenderer;
      if (containerBox)
      {
        auto placeholderHeight = [&]
        {
          if (auto *blockFlow = DynamicDowncast<RenderBlockFlow>(*placeholderBox))
          {
            if (auto placeholderLineBox = inline_iterator::FirstLineBoxFor(*blockFlow))
            {
              return LayoutUnit {
                std::max(placeholderLineBox->LogicalHeight(), placeholderLineBox->ContentLogicalHeight())};
            }
          }
          return placeholderBox->LogicalHeight();
        };
        // Center vertical align the placeholder content.
        auto logicalTop =
          placeholderTopLeft.Y() + (containerBox->LogicalHeight() / 2 - placeholderHeight() / 2);
        placeholderBox->SetLogicalTop(logicalTop);
      }
      if (!placeholderBoxHadLayout && placeholderBox->CheckForRepaintDuringLayout())
      {
        // This assumes a shadow tree without floats. If floats are added, the
        // logic should be shared with RenderBlock::layoutBlockChild.
        placeholderBox->Repaint();
      }
      // The placeholder gets layout last, after the parent text control and its other children,
      // so in order to get the correct overflow from the placeholder we need to recompute it now.
      if (neededLayout)
      {
        ComputeOverflow(ClientLogicalBottom());
      }
    }

    if (innerTextSizeChanged && GetDocument().Selection().IsFocusedAndActive()
        && GetDocument().FocusedElement() == &InputElement())
    {
      // The caps lock indicator was hidden or shown. If it is now visible then it may be occluding
      // the current selection (say, the caret was after the last character in the text field).
      // Schedule an update and reveal of the current selection.
      // FIXME: Revealing the selection after the update (RevealSelectionAfterUpdate::Forced).
      GetDocument().Selection().SetNeedsSelectionUpdate();
    }
  }

  bool RenderTextControlSingleLine::NodeAtPoint(const HitTestRequest &request, HitTestResult &result,
                                                const HitTestLocation &locationInContainer,
                                                const LayoutPoint &accumulatedOffset,
                                                HitTestAction hitTestAction) noexcept
  {
    if (!RenderTextControl::NodeAtPoint(request, result, locationInContainer, accumulatedOffset,
                                        hitTestAction))
    {
      return false;
    }

    // Say that we hit the inner text element if
    //  - we hit a node inside the inner text element,
    //  - we hit the <input> element (e.g. we're over the border or padding), or
    //  - we hit regions not in any decoration buttons.
    html::HTMLElement *container = ContainerElement();
    auto innerText = InnerTextElement();
    if ((innerText && dom::TreeQueries::IsDescendant(*result.InnerNode(), *innerText))
        || result.InnerNode() == &InputElement() || (container && container == result.InnerNode()))
    {
      LayoutPoint pointInParent = locationInContainer.Point();
      if (container && InnerBlockElement())
      {
        if (InnerBlockElement()->GetRenderBox())
        {
          pointInParent -= ToLayoutSize(InnerBlockElement()->GetRenderBox()->Location());
        }
        if (container->GetRenderBox())
        {
          pointInParent -= ToLayoutSize(container->GetRenderBox()->Location());
        }
      }
      HitInnerTextElement(result, pointInParent, accumulatedOffset);
    }
    return true;
  }

  void RenderTextControlSingleLine::StyleDidChange(style::Difference diff,
                                                   const RenderStyle *oldStyle) noexcept
  {
    RenderTextControl::StyleDidChange(diff, oldStyle);

    // We may have set the width and the height in the old style in layout().
    // Reset them now to avoid getting a spurious layout hint.
    html::HTMLElement *innerBlock = InnerBlockElement();
    if (auto *innerBlockRenderer = innerBlock ? innerBlock->Renderer() : nullptr)
    {
      innerBlockRenderer->MutableStyle().SetHeight(css::keywords::Auto {});
      innerBlockRenderer->MutableStyle().SetWidth(css::keywords::Auto {});
    }
    html::HTMLElement *container = ContainerElement();
    if (auto *containerRenderer = container ? container->Renderer() : nullptr)
    {
      containerRenderer->MutableStyle().SetHeight(css::keywords::Auto {});
      containerRenderer->MutableStyle().SetWidth(css::keywords::Auto {});
    }
    if (diff == style::DifferenceResult::Layout)
    {
      if (auto *innerTextRenderer = InnerTextElement()->Renderer())
      {
        innerTextRenderer->SetNeedsLayout(RenderObjectMarkingBehavior::MarkContainingBlockChain);
      }
      if (auto *placeholder = InputElement().PlaceholderElement())
      {
        if (placeholder->Renderer())
        {
          placeholder->Renderer()->SetNeedsLayout(RenderObjectMarkingBehavior::MarkContainingBlockChain);
        }
      }
    }
    SetHasNonVisibleOverflow(false);
  }

  bool RenderTextControlSingleLine::HasControlClip() const noexcept
  {
    auto shouldClipInnerContent = [&]
    {
      // Apply control clip for text fields with decorations.
      RenderBox *innerRenderBox = nullptr;
      if (auto *containerElement = InputElement().ContainerElement())
      {
        innerRenderBox = containerElement->GetRenderBox();
      }

      if (!innerRenderBox && InputElement().PlaceholderElement())
      {
        innerRenderBox = InputElement().PlaceholderElement()->GetRenderBox();
      }

      if (!innerRenderBox && InputElement().InnerTextElement())
      {
        innerRenderBox = InputElement().InnerTextElement()->GetRenderBox();
      }

      return !!innerRenderBox;
    };
    return shouldClipInnerContent();
  }

  LayoutRect RenderTextControlSingleLine::ControlClipRect(const LayoutPoint &additionalOffset) const noexcept
  {
    krys_debug_assert(HasControlClip());
    auto clipRect = PaddingBoxRect();
    if (auto *containerElementRenderer = ContainerElement() ? ContainerElement()->GetRenderBox() : nullptr)
    {
      clipRect = UnionRect(clipRect, containerElementRenderer->FrameRect());
    }
    clipRect.MoveBy(additionalOffset);
    return clipRect;
  }

  bool RenderTextControlSingleLine::InnerTextElementHasNonVisibleOverflow() const noexcept
  {
    if (auto innerTextElement = InnerTextElement(); innerTextElement && innerTextElement->Renderer())
    {
      return innerTextElement->Renderer()->HasNonVisibleOverflow();
    }
    return false;
  }

  float RenderTextControlSingleLine::GetAverageCharWidth() noexcept
  {
    // Since Lucida Grande is the default font, we want this to match the width
    // of MS Shell Dlg, the default font for textareas in Firefox, Safari Win and
    // IE for some encodings (in IE, the default font is encoding specific).
    // 901 is the avgCharWidth value in the OS/2 table for MS Shell Dlg.
    if (Style().GetFontCascade().FirstFamily() == StringAtom(u8"Lucida Grande"))
    {
      return ScaleEmToUnits(901);
    }

    return RenderTextControl::GetAverageCharWidth();
  }

  LayoutUnit RenderTextControlSingleLine::PreferredContentLogicalWidth(float charWidth) const noexcept
  {
    int32 factor = 0;
    bool includesDecoration = InputElement().SizeShouldIncludeDecoration(factor);
    if (factor <= 0)
    {
      factor = 20;
    }

    LayoutUnit result = LayoutUnit::FromFloatCeil(charWidth * factor);

    float maxCharWidth = 0.f;

    const auto &family = Style().GetFontCascade().FirstFamily();
    // Since Lucida Grande is the default font, we want this to match the width
    // of MS Shell Dlg, the default font for textareas in Firefox, Safari Win and
    // IE for some encodings (in IE, the default font is encoding specific).
    // 4027 is the (xMax - xMin) value in the "head" font table for MS Shell Dlg.
    if (family == StringAtom(u8"Lucida Grande"))
    {
      maxCharWidth = ScaleEmToUnits(4'027);
    }
    else if (Style().GetFontCascade().HasValidAverageCharWidth())
    {
      maxCharWidth = std::roundf(Style().GetFontCascade().PrimaryFont()->MaxCharWidth());
    }

    // For text inputs, IE adds some extra width.
    if (maxCharWidth > 0.f)
    {
      result += maxCharWidth - charWidth;
    }

    if (includesDecoration)
    {
      result += InputElement().DecorationWidth(result.ToFloat());
    }

    if (auto innerText = InnerTextElement())
    {
      if (auto *innerRenderer = innerText->Renderer())
      {
        result += innerRenderer->EndPaddingWidthForCaret();
      }
    }

    return result;
  }

  LayoutUnit
    RenderTextControlSingleLine::ComputeControlLogicalHeight(LayoutUnit lineHeight,
                                                             LayoutUnit nonContentHeight) const noexcept
  {
    return lineHeight + nonContentHeight;
  }

  void RenderTextControlSingleLine::Autoscroll(const IntPoint &position) noexcept
  {
    auto *renderer = InnerTextElement()->Renderer();
    if (!renderer)
    {
      return;
    }
    if (auto *layer = renderer->Layer())
    {
      layer->Autoscroll(position);
    }
  }

  int32 RenderTextControlSingleLine::ScrollWidth() const noexcept
  {
    if (auto innerText = InnerTextElement())
    {
      if (auto *innerTextRenderer = innerText->Renderer())
      {
        // Adjust scrollWidth to inculde input element horizontal paddings and decoration width.
        auto adjustment = ClientWidth() - innerTextRenderer->ClientWidth();
        return innerTextRenderer->ScrollWidth() + adjustment.ToInt();
      }
    }
    return RenderBlockFlow::ScrollWidth();
  }

  int32 RenderTextControlSingleLine::ScrollHeight() const noexcept
  {
    if (auto innerText = InnerTextElement())
    {
      if (auto *innerTextRenderer = innerText->Renderer())
      {
        // Adjust scrollHeight to inculde input element vertical paddings and decoration height.
        auto adjustment = ClientHeight() - innerTextRenderer->ClientHeight();
        return innerTextRenderer->ScrollHeight() + adjustment.ToInt();
      }
    }
    return RenderBlockFlow::ScrollHeight();
  }

  int32 RenderTextControlSingleLine::ScrollLeft() const noexcept
  {
    if (auto innerTextElement = InnerTextElement(); innerTextElement && innerTextElement->Renderer())
    {
      return innerTextElement->Renderer()->ScrollLeft();
    }
    return RenderBlockFlow::ScrollLeft();
  }

  int32 RenderTextControlSingleLine::ScrollTop() const noexcept
  {
    if (auto innerTextElement = InnerTextElement(); innerTextElement && innerTextElement->Renderer())
    {
      return innerTextElement->Renderer()->ScrollTop();
    }
    return RenderBlockFlow::ScrollTop();
  }

  void RenderTextControlSingleLine::SetScrollLeft(int32 newLeft, const ScrollPositionChangeOptions &) noexcept
  {
    if (auto innerTextElement = InnerTextElement())
    {
      innerTextElement->SetScrollLeft(newLeft);
    }
  }

  void RenderTextControlSingleLine::SetScrollTop(int32 newTop, const ScrollPositionChangeOptions &) noexcept
  {
    if (auto innerTextElement = InnerTextElement())
    {
      innerTextElement->SetScrollTop(newTop);
    }
  }

  void RenderTextControlSingleLine::SetScrollPosition(const ScrollPosition &position,
                                                      const ScrollPositionChangeOptions &options) noexcept
  {
    if (auto innerTextElement = InnerTextElement(); innerTextElement && innerTextElement->Renderer())
    {
      innerTextElement->Renderer()->SetScrollPosition(position, options);
    }
  }

  bool RenderTextControlSingleLine::Scroll(ScrollDirection direction, ScrollGranularity granularity,
                                           uint32 stepCount, dom::Element **stopElement) noexcept
  {
    auto *renderer = InnerTextElement()->Renderer();
    if (!renderer)
    {
      return false;
    }
    auto *scrollableArea = renderer->Layer() ? renderer->Layer()->GetScrollableArea() : nullptr;
    if (scrollableArea && scrollableArea->Scroll(direction, granularity, stepCount))
    {
      return true;
    }
    return RenderBlockFlow::Scroll(direction, granularity, stepCount, stopElement);
  }

  bool RenderTextControlSingleLine::LogicalScroll(ScrollLogicalDirection direction,
                                                  ScrollGranularity granularity, uint32 stepCount,
                                                  dom::Element **stopElement) noexcept
  {
    auto *layer = InnerTextElement()->Renderer()->Layer();
    auto *scrollableArea = layer ? layer->GetScrollableArea() : nullptr;
    if (scrollableArea
        && scrollableArea->Scroll(
          LogicalToPhysical(direction, GetWritingMode().IsHorizontal(), GetWritingMode().IsBlockFlipped()),
          granularity, stepCount))
    {
      return true;
    }
    return RenderBlockFlow::LogicalScroll(direction, granularity, stepCount, stopElement);
  }

  html::HTMLInputElement &RenderTextControlSingleLine::InputElement() const noexcept
  {
    return Downcast<html::HTMLInputElement>(RenderTextControl::TextFormControlElement());
  }

  RenderTextControlInnerBlock *RenderTextControlSingleLine::InnerTextRenderer() const noexcept
  {
    auto innerText = InnerTextElement();
    return innerText ? innerText->Renderer() : nullptr;
  }

  RenderTextControlInnerBlock::RenderTextControlInnerBlock(dom::Element &element,
                                                           RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TextControlInnerBlock, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTextControlInnerBlock());
  }

  RenderTextControlInnerBlock::~RenderTextControlInnerBlock() noexcept = default;

  bool RenderTextControlInnerBlock::CanBeProgramaticallyScrolled() const noexcept
  {
    // FIXME: Not with a strong password AutoFill button (hasAutofillStrongPasswordButton), with autofill.
    return true;
  }
}
