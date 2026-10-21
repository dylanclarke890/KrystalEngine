#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockFormattingState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Block/BlockLayoutState.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineFormattingUtils.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItemsBuilder.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Invalidation/InlineInvalidation.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContent.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContentBuilder.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationInlineContentPainter.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationBoxTreeUpdater.hpp"
#include "Krystal.Booey/Layout/Integration/LayoutIntegrationUtils.hpp"
#include "Krystal.Booey/Layout/LayoutState.hpp"
#include "Krystal.Booey/Layout/LayoutTree/FormattingContextBoxIterator.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxGeometry.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBoxInlines.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/FloatingObjects.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/InlineWalker.hpp"
#include "Krystal.Booey/Rendering/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/RenderReplaced.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Shapes/ShapeOutsideInfo.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include <ranges>

namespace krys::boo::layout_integration
{
  namespace
  {
    enum class TypeOfChangeForInvalidation : uint8
    {
      NodeInsertion,
      NodeRemoval,
      NodeMutation
    };
    KRYS_NODISCARD bool CanUseForLineLayout(const RenderBlockFlow &) noexcept;

    KRYS_NODISCARD bool
      ShouldInvalidateLineLayoutAfterChangeFor(const RenderBlockFlow &rootBlockContainer,
                                               const RenderObject &renderer, const LineLayout &lineLayout,
                                               TypeOfChangeForInvalidation typeOfChange) noexcept
    {
      auto isSupportedRendererWithChange = [&](const RenderObject &renderer)
      {
        if (Is<RenderText>(renderer))
        {
          return true;
        }
        if (!renderer.IsInFlow())
        {
          return false;
        }
        if (Is<RenderLineBreak>(renderer))
        {
          return true;
        }
        if (auto *renderBox = DynamicDowncast<RenderBox>(renderer);
            renderBox && renderBox->HasRelativeDimensions())
        {
          return false;
        }
        if (Is<RenderReplaced>(renderer))
        {
          return typeOfChange == TypeOfChangeForInvalidation::NodeInsertion;
        }
        if (auto *inlineRenderer = DynamicDowncast<RenderInline>(renderer))
        {
          return typeOfChange == TypeOfChangeForInvalidation::NodeInsertion && !inlineRenderer->FirstChild();
        }
        return false;
      };
      if (!isSupportedRendererWithChange(renderer))
      {
        return true;
      }

      auto isSupportedParent = [&]
      {
        auto *parent = renderer.Parent();
        // Content append under existing inline box is not yet supported.
        return Is<RenderBlockFlow>(parent) || (Is<RenderInline>(parent) && !parent->EverHadLayout());
      };
      if (!isSupportedParent())
      {
        return true;
      }
      if (rootBlockContainer.ContainsFloats())
      {
        return true;
      }

      auto isBidiContent = [&]
      {
        if (lineLayout.ContentNeedsVisualReordering())
        {
          return true;
        }
        if (auto *textRenderer = DynamicDowncast<RenderText>(renderer))
        {
          auto hasStrongDirectionalityContent = textRenderer->HasStrongDirectionalityContent();
          if (!hasStrongDirectionalityContent)
          {
            hasStrongDirectionalityContent =
              layout::TextUtil::ContainsStrongDirectionalityText(textRenderer->Text());
            const_cast<RenderText *>(textRenderer)
              ->SetHasStrongDirectionalityContent(*hasStrongDirectionalityContent);
          }
          return *hasStrongDirectionalityContent;
        }
        if (auto *renderInline = DynamicDowncast<RenderInline>(renderer))
        {
          auto &style = renderInline->Style();
          return style.GetWritingMode().IsBidiRTL()
                 || (style.RtlOrdering() == Order::Logical && style.GetUnicodeBidi() != UnicodeBidi::Normal);
        }
        return false;
      };
      if (isBidiContent())
      {
        // FIXME: InlineItemsBuilder needs some work to support paragraph level bidi handling.
        return true;
      }
      auto hasFirstLetter = [&]
      {
        // FIXME: RenderTreeUpdater::updateTextRenderer produces odd values for offset/length when
        // first-letter is present webkit.org/b/263343
        if (rootBlockContainer.Style().HasPseudoStyle(PseudoElementType::FirstLetter))
        {
          return true;
        }
        if (rootBlockContainer.IsAnonymous())
        {
          return rootBlockContainer.ContainingBlock()
                 && rootBlockContainer.ContainingBlock()->Style().HasPseudoStyle(
                   PseudoElementType::FirstLetter);
        }
        return false;
      };
      if (hasFirstLetter())
      {
        return true;
      }

      if (auto *previousDamage = lineLayout.Damage();
          previousDamage
          && (previousDamage->Reasons()
                != EnumSet<layout::InlineDamage::Reason> {layout::InlineDamage::Reason::Append}
              || !previousDamage->LayoutStartPosition()))
      {
        // Only support subsequent append operations where we managed to invalidate the content for partial
        // layout.
        return true;
      }

      auto &rootBlockContainerStyle = rootBlockContainer.Style();
      auto shouldBalance = rootBlockContainerStyle.GetTextWrapMode() == TextWrapMode::Wrap
                           && rootBlockContainerStyle.GetTextWrapStyle() == TextWrapStyle::Balance;
      auto shouldPrettify = rootBlockContainerStyle.GetTextWrapMode() == TextWrapMode::Wrap
                            && rootBlockContainerStyle.GetTextWrapStyle() == TextWrapStyle::Pretty;
      auto hasAutospace = !rootBlockContainerStyle.GetTextAutospace().IsNoAutospace();
      if (rootBlockContainer.GetWritingMode().IsBidiRTL() || shouldBalance || shouldPrettify || hasAutospace)
      {
        return true;
      }

      auto rootHasNonSupportedRenderer = [&](bool shouldOnlyCheckForRelativeDimension = false)
      {
        for (auto *sibling = rootBlockContainer.FirstChild(); sibling; sibling = sibling->NextSibling())
        {
          if (auto *inlineBox = DynamicDowncast<RenderInline>(*sibling);
              inlineBox && !inlineBox->Style().GetTextAutospace().IsNoAutospace())
          {
            return true;
          }

          auto siblingHasRelativeDimensions = false;
          if (auto *renderBox = DynamicDowncast<RenderBox>(*sibling);
              renderBox && renderBox->HasRelativeDimensions())
          {
            siblingHasRelativeDimensions = true;
          }

          if (shouldOnlyCheckForRelativeDimension && !siblingHasRelativeDimensions)
          {
            continue;
          }

          if (siblingHasRelativeDimensions
              || (!Is<RenderText>(*sibling) && !Is<RenderLineBreak>(*sibling)
                  && !Is<RenderReplaced>(*sibling)))
          {
            return true;
          }
        }
        return !CanUseForLineLayout(rootBlockContainer);
      };
      switch (typeOfChange)
      {
        case TypeOfChangeForInvalidation::NodeRemoval:
          return (!renderer.PreviousSibling() && !renderer.NextSibling()) || rootHasNonSupportedRenderer();
        case TypeOfChangeForInvalidation::NodeInsertion:
          return rootHasNonSupportedRenderer(!renderer.NextSibling());
        case TypeOfChangeForInvalidation::NodeMutation: return rootHasNonSupportedRenderer();
      }
      krys_debug_assert(false);
      return true;
    }

    KRYS_NODISCARD const inline_display::Line &
      LastLineWithInflowContent(const inline_display::Lines &lines) noexcept
    {
      // Out-of-flow/float content only don't produce lines with inline content. They should not be taken into
      // account when computing content box height/baselines.
      for (auto index = lines.size(); index--;)
      {
        auto &line = lines[index];
        krys_debug_assert(line.BoxCount());
        if (line.HasContentfulInFlowBox())
        {
          return line;
        }
        // FIXME: This should be merged with margin collapsing.
        if (line.HasInflowBox() && index
            && lines[index - 1].LineBoxLogicalRect().MaxY() > line.LineBoxLogicalRect().Y())
        {
          krys_debug_assert(lines[index - 1].HasBlockLevelBox());
          return line;
        }
      }
      return lines.front();
    }

    KRYS_NODISCARD inline Maybe<layout::BlockLayoutState::LineClamp>
      LineClamp(const RenderBlockFlow &rootRenderer) noexcept
    {
      // The legacy line clamp (-webkit-line-clamp) was dropped.
      auto *layoutState = rootRenderer.View().FrameView().LayoutContext().LayoutState();
      if (!layoutState)
      {
        return null;
      }
      if (auto lineClamp = layoutState->GetLineClamp())
      {
        return layout::BlockLayoutState::LineClamp {lineClamp->MaximumLines, lineClamp->ShouldDiscardOverflow,
                                                    false};
      }
      return null;
    }

    KRYS_NODISCARD inline layout::BlockLayoutState::TextBoxTrim
      TextBoxTrim(const RenderBlockFlow &rootRenderer) noexcept
    {
      auto textBoxTrim = rootRenderer.View().FrameView().LayoutContext().GetTextBoxTrim();
      if (!textBoxTrim)
      {
        return {};
      }

      auto textBoxTrimForIFC = layout::BlockLayoutState::TextBoxTrim {};
      auto isLineInverted = rootRenderer.GetWritingMode().IsLineInverted();
      if (textBoxTrim->TrimFirstFormattedLine)
      {
        textBoxTrimForIFC.Add(isLineInverted ? layout::BlockLayoutState::TextBoxTrimSide::End
                                             : layout::BlockLayoutState::TextBoxTrimSide::Start);
      }

      if (textBoxTrim->LastFormattedLineRoot.get() == &rootRenderer)
      {
        textBoxTrimForIFC.Add(isLineInverted ? layout::BlockLayoutState::TextBoxTrimSide::Start
                                             : layout::BlockLayoutState::TextBoxTrimSide::End);
      }
      return textBoxTrimForIFC;
    }

    KRYS_NODISCARD float BaselineForEmptyContent(const RenderBlockFlow &rootRenderer) noexcept
    {
      auto *rootLayoutBox = rootRenderer.LayoutBox();
      if (!rootLayoutBox)
      {
        krys_debug_assert(false);
        return {};
      }

      auto &fontMetrics = rootRenderer.Style().MetricsOfPrimaryFont();
      auto ascent = layout::InlineFormattingUtils::SnapToInt(fontMetrics.Ascent(), *rootLayoutBox);
      auto descent = layout::InlineFormattingUtils::SnapToInt(fontMetrics.Descent(), *rootLayoutBox);
      auto baseline =
        ascent + (rootLayoutBox->FirstLineStyle().ComputedLineHeight() - (ascent + descent)) / 2;
      return layout::InlineFormattingUtils::SnapToInt(
        rootRenderer.BorderAndPaddingBefore().ToFloat() + baseline, *rootLayoutBox,
        layout::InlineFormattingUtils::SnapDirection::Floor);
    }

    // FIXME: SVG blocks other than foreign objects use legacy line layout unless the setting says otherwise,
    // once there's SVG.
    KRYS_NODISCARD bool CanUseForLineLayout(const RenderBlockFlow &) noexcept
    {
      return true;
    }

    KRYS_NODISCARD bool CanUseForPreferredWidthComputationImpl(const RenderBlockFlow &blockContainer) noexcept
    {
      for (auto walker = InlineWalker(blockContainer); !walker.AtEnd(); walker.Advance())
      {
        auto &renderer = *walker.Current();
        if (!renderer.IsInFlow())
        {
          return false;
        }

        auto isFullySupportedInFlowRenderer = renderer.IsRenderText() || Is<RenderLineBreak>(renderer)
                                              || Is<RenderInline>(renderer) || Is<RenderListMarker>(renderer);
        if (isFullySupportedInFlowRenderer)
        {
          continue;
        }

        auto &unsupportedRenderElement = Downcast<RenderElement>(renderer);
        if (!unsupportedRenderElement.GetWritingMode().IsHorizontal()
            || !unsupportedRenderElement.Style().LogicalWidth().IsFixed())
        {
          return false;
        }

        auto isNonSupportedFixedWidthContent = [&]
        {
          // FIXME: Implement this image special in line builder.
          auto allowImagesToBreak =
            !blockContainer.GetDocument().InQuirksMode() || !blockContainer.IsRenderTableCell();
          if (!allowImagesToBreak)
          {
            return true;
          }
          // FIXME: See RenderReplaced::ComputePreferredLogicalWidths where the minimum preferred logical
          // width is set to 0.
          auto isReplacedWithSpecialIntrinsicWidth = [&]
          {
            if (auto *renderReplaced = DynamicDowncast<RenderReplaced>(unsupportedRenderElement))
            {
              return renderReplaced->Style().LogicalMaxWidth().IsPercentOrCalculated();
            }
            return false;
          };
          return isReplacedWithSpecialIntrinsicWidth();
        };
        if (isNonSupportedFixedWidthContent())
        {
          return false;
        }
      }
      return true;
    }
  }

  LineLayout::LineLayout(RenderBlockFlow &flow) noexcept
      : _rootLayoutBox(ShareCheckedPtr(BoxTreeUpdater {flow}.Build().get())),
        _document(ShareCheckedPtr(&flow.GetDocument())),
        _layoutState(WeakPtr<layout::LayoutState>::WithRef(flow.View().GetLayoutState())),
        _blockFormattingState(GetLayoutState().EnsureBlockFormattingState(RootLayoutBox())),
        _inlineContentCache(GetLayoutState().GetInlineContentCache(RootLayoutBox())),
        _boxGeometryUpdater(flow.View().GetLayoutState(), RootLayoutBox())
  {
  }

  LineLayout::~LineLayout() noexcept
  {
    auto &rootRenderer = Flow();
    auto shouldPopulateBreakingPositionCache = [&]
    {
      auto mayHaveInvalidContent = IsDamaged() || !_inlineContent;
      if (_document->RenderTreeBeingDestroyed() || mayHaveInvalidContent)
      {
        return false;
      }
      return !_inlineContentCache.GetInlineItems().IsPopulatedFromCache();
    };
    if (shouldPopulateBreakingPositionCache())
    {
      layout::InlineItemsBuilder::PopulateBreakingPositionCache(
        _inlineContentCache.GetInlineItems().Content());
    }
    ClearInlineContent();
    GetLayoutState().DestroyInlineContentCache(RootLayoutBox());
    GetLayoutState().DestroyBlockFormattingState(RootLayoutBox());
    _boxGeometryUpdater.Clear();
    _lineDamage = nullptr;
    _rootLayoutBox = nullptr;

    BoxTreeUpdater {rootRenderer, *_document}.TearDown();
  }

  RenderBlockFlow *LineLayout::BlockContainer(const RenderObject &renderer) noexcept
  {
    // FIXME: Renderer replicas and scrollbar parts have their parent set but are not actually in the tree,
    // once there are any.
    for (auto *parent = renderer.Parent(); parent; parent = parent->Parent())
    {
      if (!parent->ChildrenInline())
      {
        return nullptr;
      }
      if (auto *renderBlockFlow = DynamicDowncast<RenderBlockFlow>(*parent))
      {
        return renderBlockFlow;
      }
    }

    return nullptr;
  }

  bool LineLayout::Contains(const RenderElement &renderer) const noexcept
  {
    if (!renderer.LayoutBox())
    {
      return false;
    }
    if (!renderer.LayoutBox()->IsInFormattingContextEstablishedBy(RootLayoutBox()))
    {
      return false;
    }
    return GetLayoutState().HasBoxGeometry(*renderer.LayoutBox());
  }

  LineLayout *LineLayout::Containing(RenderObject &renderer) noexcept
  {
    if (!renderer.IsInline())
    {
      // IFC may contain block level boxes (floats and out-of-flow boxes).
      // FIXME: SVG blocks and frame sets, once there are any.
      auto adjustedContainingBlock = [&]() -> RenderBlockFlow *
      {
        if (renderer.IsOutOfFlowPositioned())
        {
          // Here we are looking for the containing block as if the out-of-flow box was inflow (for static
          // position purpose).
          auto *containingBlock = renderer.Parent();
          if (Is<RenderInline>(containingBlock))
          {
            return DynamicDowncast<RenderBlockFlow>(containingBlock->ContainingBlock());
          }
          return DynamicDowncast<RenderBlockFlow>(containingBlock);
        }
        if (renderer.IsFloating())
        {
          // Note that ContainingBlock() on boxes in top layer (i.e. dialog) may return incorrect result
          // during style change even with not-yet-updated style.
          return DynamicDowncast<RenderBlockFlow>(RenderObject::ContainingBlockForPositionType(
            Downcast<RenderBox>(renderer).Style().GetPosition(), renderer));
        }
        if (auto *parentInlineBox = DynamicDowncast<RenderInline>(renderer.Parent()))
        {
          return DynamicDowncast<RenderBlockFlow>(parentInlineBox->ContainingBlock());
        }
        return nullptr;
      };
      if (auto *blockContainer = adjustedContainingBlock())
      {
        return blockContainer->InlineLayout();
      }
      return nullptr;
    }

    if (auto *container = BlockContainer(renderer))
    {
      return container->InlineLayout();
    }

    return nullptr;
  }

  const LineLayout *LineLayout::Containing(const RenderObject &renderer) noexcept
  {
    return Containing(const_cast<RenderObject &>(renderer));
  }

  bool LineLayout::CanUseFor(const RenderBlockFlow &flow) noexcept
  {
    return CanUseForLineLayout(flow);
  }

  bool LineLayout::CanUseForPreferredWidthComputation(const RenderBlockFlow &flow) noexcept
  {
    return CanUseForPreferredWidthComputationImpl(flow);
  }

  bool LineLayout::ShouldInvalidateLineLayoutAfterContentChange(const RenderBlockFlow &parent,
                                                                const RenderObject &rendererWithNewContent,
                                                                const LineLayout &lineLayout) noexcept
  {
    return ShouldInvalidateLineLayoutAfterChangeFor(parent, rendererWithNewContent, lineLayout,
                                                    TypeOfChangeForInvalidation::NodeMutation);
  }

  bool LineLayout::ShouldInvalidateLineLayoutAfterTreeMutation(const RenderBlockFlow &parent,
                                                               const RenderObject &renderer,
                                                               const LineLayout &lineLayout,
                                                               bool isRemoval) noexcept
  {
    return ShouldInvalidateLineLayoutAfterChangeFor(parent, renderer, lineLayout,
                                                    isRemoval ? TypeOfChangeForInvalidation::NodeRemoval
                                                              : TypeOfChangeForInvalidation::NodeInsertion);
  }

  void LineLayout::UpdateFormattingContexGeometries(LayoutUnit availableLogicalWidth) noexcept
  {
    _boxGeometryUpdater.SetFormattingContextRootGeometry(availableLogicalWidth);
    _inlineContentConstraints = _boxGeometryUpdater.FormattingContextConstraints(availableLogicalWidth);
    _boxGeometryUpdater.SetFormattingContextContentGeometry(
      _inlineContentConstraints->Horizontal().LogicalWidth, null);
  }

  void LineLayout::UpdateStyle(const RenderObject &renderer) noexcept
  {
    BoxTreeUpdater::UpdateStyle(renderer);
  }

  bool LineLayout::RootStyleWillChange(const RenderBlockFlow &root, const RenderStyle &newStyle) noexcept
  {
    if (!root.LayoutBox() || !root.LayoutBox()->IsElementBox())
    {
      krys_debug_assert(false);
      return false;
    }
    if (!_inlineContent)
    {
      return false;
    }

    return layout::InlineInvalidation {EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(),
                                       _inlineContent->DisplayContent()}
      .RootStyleWillChange(*root.LayoutBox(), newStyle);
  }

  bool LineLayout::StyleWillChange(const RenderElement &renderer, const RenderStyle &newStyle,
                                   style::Difference diff) noexcept
  {
    if (!renderer.LayoutBox())
    {
      krys_debug_assert(false);
      return false;
    }
    if (!_inlineContent)
    {
      return false;
    }

    return layout::InlineInvalidation {EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(),
                                       _inlineContent->DisplayContent()}
      .StyleWillChange(*renderer.LayoutBox(), newStyle, diff);
  }

  bool LineLayout::BoxContentWillChange(const RenderBox &renderer) noexcept
  {
    if (!_inlineContent || !renderer.LayoutBox())
    {
      return false;
    }

    return layout::InlineInvalidation {EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(),
                                       _inlineContent->DisplayContent()}
      .InlineLevelBoxContentWillChange(*renderer.LayoutBox());
  }

  bool LineLayout::InsertedIntoTree(const RenderElement &parent, RenderObject &child) noexcept
  {
    if (Flow().Style().IsSkippedRootOrSkippedContent())
    {
      return false;
    }

    if (!_inlineContent)
    {
      // This should only be called on partial layout.
      krys_debug_assert(false);
      return false;
    }

    auto &childLayoutBox = BoxTreeUpdater {Flow()}.Insert(parent, child, child.PreviousSibling());
    if (auto *childInlineTextBox = DynamicDowncast<layout::InlineTextBox>(childLayoutBox))
    {
      auto invalidation = layout::InlineInvalidation {
        EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(), _inlineContent->DisplayContent()};
      return invalidation.TextInserted(*childInlineTextBox);
    }

    if (childLayoutBox.IsLineBreakBox() || childLayoutBox.IsReplacedBox() || childLayoutBox.IsInlineBox())
    {
      auto invalidation = layout::InlineInvalidation {
        EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(), _inlineContent->DisplayContent()};
      return invalidation.InlineLevelBoxInserted(childLayoutBox);
    }

    krys_debug_assert(false);
    return false;
  }

  bool LineLayout::RemovedFromTree(const RenderElement &parent, RenderObject &child) noexcept
  {
    if (Flow().Style().IsSkippedRootOrSkippedContent())
    {
      return false;
    }

    if (!child.EverHadLayout())
    {
      EnsureLineDamage().AddDetachedBox(BoxTreeUpdater {Flow()}.Remove(parent, child));
      return false;
    }

    if (!_inlineContent)
    {
      // This should only be called on partial layout.
      krys_debug_assert(false);
      return false;
    }

    auto &childLayoutBox = *child.LayoutBox();
    auto *childInlineTextBox = DynamicDowncast<layout::InlineTextBox>(childLayoutBox);
    auto invalidation = layout::InlineInvalidation {
      EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(), _inlineContent->DisplayContent()};
    auto boxIsInvalidated = childInlineTextBox ? invalidation.TextWillBeRemoved(*childInlineTextBox)
                            : childLayoutBox.IsLineBreakBox()
                              ? invalidation.InlineLevelBoxWillBeRemoved(childLayoutBox)
                              : false;
    if (boxIsInvalidated)
    {
      _lineDamage->AddDetachedBox(BoxTreeUpdater {Flow()}.Remove(parent, child));
    }
    return boxIsInvalidated;
  }

  bool LineLayout::UpdateTextContent(const RenderText &textRenderer, Maybe<size_t> offset,
                                     size_t oldLength) noexcept
  {
    if (Flow().Style().IsSkippedRootOrSkippedContent())
    {
      return false;
    }

    if (!_inlineContent)
    {
      // This is supposed to be only called on partial layout, but
      // RenderText::setText may be (force) called after min/max size computation and before layout.
      // We may need to invalidate anyway to clean up inline item list.
      return false;
    }

    BoxTreeUpdater::UpdateContent(textRenderer);

    auto invalidation = layout::InlineInvalidation {
      EnsureLineDamage(), _inlineContentCache.GetInlineItems().Content(), _inlineContent->DisplayContent()};
    auto &inlineTextBox = *textRenderer.LayoutBox();
    if (!offset)
    {
      // Text content is entirely replaced.
      return invalidation.TextInserted(inlineTextBox, 0);
    }

    if (*offset == oldLength)
    {
      // This is essentially just an append.
      return invalidation.TextInserted(inlineTextBox);
    }

    auto newLength = inlineTextBox.Content().size();
    return newLength >= oldLength ? invalidation.TextInserted(inlineTextBox, offset)
                                  : invalidation.TextWillBeRemoved(inlineTextBox, offset);
  }

  void LineLayout::UpdateOverflow() noexcept
  {
    if (!_inlineContent)
    {
      return;
    }
    InlineContentBuilder {Flow()}.UpdateLineOverflow(*_inlineContent);
  }

  bool LineLayout::IsContentConsideredStale() const noexcept
  {
    auto *rootRenderer = _rootLayoutBox->RendererForIntegration();
    if (!rootRenderer)
    {
      return true;
    }
    if (rootRenderer->NormalChildNeedsLayout())
    {
      // FIXME: Let's bail out on needsLayout() -see webkit.org/b/304002
      return true;
    }
    if (rootRenderer->Style().IsSkippedRootOrSkippedContent())
    {
      return true;
    }
    if (_lineDamage && _lineDamage->HasDetachedContent())
    {
      return true;
    }
    return false;
  }

  void LineLayout::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset,
                         const RenderInline *layerRenderer) noexcept
  {
    if (!_inlineContent)
    {
      return;
    }

    if (IsContentConsideredStale())
    {
      krys_debug_assert(false);
      return;
    }

    auto shouldPaintForPhase = [&]
    {
      switch (paintInfo.Phase)
      {
        case PaintPhase::Accessibility:
        case PaintPhase::Foreground:
        case PaintPhase::EventRegion:
        case PaintPhase::TextClip:
        case PaintPhase::Mask:
        case PaintPhase::Selection:
        case PaintPhase::Outline:
        case PaintPhase::ChildOutlines:
        case PaintPhase::SelfOutline:   return true;
        case PaintPhase::ChildBlockBackground:
        case PaintPhase::ChildBlockBackgrounds:
        case PaintPhase::Float:
          // These phases are only needed in blocks-in-inline case.
          return _inlineContent->HasBlockLevelBoxes();
        default: return false;
      }
    };
    if (!shouldPaintForPhase())
    {
      return;
    }

    if (!_inlineContent->HasPaintedInlineLevelBoxes())
    {
      if (!_inlineContent->HasBlockLevelBoxes())
      {
        return;
      }
      if (!layerRenderer)
      {
        // Shortcut block-only painting.
        Flow().PaintBlockLevelContentInInline(paintInfo, paintOffset);
        return;
      }
    }

    InlineContentPainter {paintInfo, paintOffset, layerRenderer, *_inlineContent, Flow()}.Paint();
  }

  bool LineLayout::HitTest(const HitTestRequest &request, HitTestResult &result,
                           const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                           HitTestAction hitTestAction, const RenderInline *layerRenderer) noexcept
  {
    if (!_inlineContent)
    {
      return false;
    }

    // All real inline content is foreground.
    if (hitTestAction != HitTestForeground && !_inlineContent->HasBlockLevelBoxes())
    {
      return false;
    }
    if (hitTestAction == HitTestBlockBackground)
    {
      return false;
    }

    if (IsContentConsideredStale())
    {
      krys_debug_assert(false);
      return false;
    }

    auto hitTestBoundingBox = locationInContainer.BoundingBox();
    hitTestBoundingBox.MoveBy(-accumulatedOffset);
    auto boxRange = _inlineContent->BoxesForRect(hitTestBoundingBox);

    LayerPaintScope layerPaintScope(layerRenderer);

    auto flippedContentOffsetIfNeeded = [&](const RenderBox &childRenderer, const LayoutPoint &contentOffset)
    {
      if (Flow().GetWritingMode().IsBlockFlipped())
      {
        return Flow().FlipForWritingModeForChild(childRenderer, contentOffset);
      }
      return contentOffset;
    };

    for (auto it = boxRange.rbegin(); it != boxRange.rend(); ++it)
    {
      auto &box = *it;
      if (!layerPaintScope.TestIsIncludesAndUpdate(box))
      {
        continue;
      }

      bool visibleForHitTesting =
        request.UserTriggered() ? box.IsVisible() : box.IsVisibleIgnoringUsedVisibility();
      if (!visibleForHitTesting)
      {
        continue;
      }

      auto shouldHitTestForPhase = [&]
      {
        switch (hitTestAction)
        {
          case HitTestForeground:
            // Inline boxes around block-in-inline are hit tested in block background phase.
            return !_inlineContent->IsInlineBoxWrapperForBlockLevelBox(box);
          case HitTestChildBlockBackground:
            return box.IsBlockLevelBox() || _inlineContent->IsInlineBoxWrapperForBlockLevelBox(box);
          case HitTestChildBlockBackgrounds:
          case HitTestFloat:                 return box.IsBlockLevelBox();
          case HitTestBlockBackground:       break;
        }
        krys_debug_assert(false);
        return false;
      };

      if (!shouldHitTestForPhase())
      {
        continue;
      }

      auto &renderer = *box.LayoutBox().RendererForIntegration();

      if (box.IsBlockLevelBox())
      {
        auto &renderBox = Downcast<RenderBox>(renderer);
        auto childHitTest =
          hitTestAction == HitTestChildBlockBackgrounds ? HitTestChildBlockBackground : hitTestAction;
        LayoutPoint childPoint = flippedContentOffsetIfNeeded(renderBox, accumulatedOffset);
        if (!renderBox.HasSelfPaintingLayer()
            && renderBox.NodeAtPoint(request, result, locationInContainer, childPoint, childHitTest))
        {
          return true;
        }
        continue;
      }

      if (box.IsAtomicInlineBox())
      {
        if (renderer.HitTest(request, result, locationInContainer,
                             flippedContentOffsetIfNeeded(Downcast<RenderBox>(renderer), accumulatedOffset)))
        {
          return true;
        }
        continue;
      }

      auto &currentLine = _inlineContent->DisplayContent().LineList[box.LineIndex()];
      auto flippedRect = LayoutRect {inline_display::Box::VisibleRectIgnoringBlockDirection(
        box, currentLine.VisibleRectIgnoringBlockDirection())};
      Flow().FlipForWritingMode(flippedRect);
      auto boxRect = flippedRect;
      boxRect.MoveBy(accumulatedOffset);

      if (!locationInContainer.Intersects(boxRect))
      {
        continue;
      }

      auto &elementRenderer = *[&]() -> const RenderElement *
      {
        auto *renderElement = DynamicDowncast<RenderElement>(renderer);
        return renderElement ? renderElement : renderer.Parent();
      }();
      if (!elementRenderer.VisibleToHitTesting(&request))
      {
        continue;
      }

      renderer.UpdateHitTestResult(
        result, Flow().FlipForWritingMode(locationInContainer.Point() - ToLayoutSize(accumulatedOffset)));
      if (result.AddNodeToListBasedTestResult(renderer.NodeForHitTest(), request, locationInContainer,
                                              boxRect)
          == HitTestProgress::Stop)
      {
        return true;
      }
    }

    return false;
  }

  std::pair<LayoutUnit, LayoutUnit> LineLayout::ComputeIntrinsicWidthConstraints() noexcept
  {
    auto parentBlockLayoutState = layout::BlockLayoutState {_blockFormattingState.GetPlacedFloats(), {}};
    auto inlineFormattingContext =
      layout::InlineFormattingContext {RootLayoutBox(), GetLayoutState(), parentBlockLayoutState};
    // FIXME: This is where we need to switch between minimum and maximum box geometries.
    // Currently we only support content where min == max.
    if (_lineDamage)
    {
      _inlineContentCache.ResetMinimumMaximumContentSizes();
    }
    _boxGeometryUpdater.SetFormattingContextContentGeometry(null, layout::IntrinsicWidthMode::Minimum);
    return inlineFormattingContext.MinimumMaximumContentSize(_lineDamage.get());
  }

  Maybe<LayoutRect> LineLayout::Layout(RenderBlockFlow::MarginInfo &marginInfo,
                                       ForceFullLayout forcedFullLayout) noexcept
  {
    if (forcedFullLayout == ForceFullLayout::Yes && _lineDamage)
    {
      layout::InlineInvalidation::ResetInlineDamage(*_lineDamage);
    }

    PreparePlacedFloats();

    auto isPartialLayout = layout::InlineInvalidation::MayOnlyNeedPartialLayout(_lineDamage.get());
    if (!isPartialLayout)
    {
      // FIXME: Partial layout should not rely on previous inline display content.
      ClearInlineContent();
    }

    krys_debug_assert(_inlineContentConstraints.has_value());
    auto intrusiveInitialLetterBottom = [&]() -> Maybe<LayoutUnit>
    {
      if (auto lowestInitialLetterLogicalBottom = Flow().LowestInitialLetterLogicalBottom())
      {
        return *lowestInitialLetterLogicalBottom - _inlineContentConstraints->LogicalTop();
      }
      return null;
    };

    auto parentBlockLayoutState =
      layout::BlockLayoutState {_blockFormattingState.GetPlacedFloats(),
                                layout::IntegrationUtils::ToMarginState(marginInfo, {}),
                                LineClamp(Flow()),
                                TextBoxTrim(Flow()),
                                Flow().Style().GetTextBoxEdge(),
                                intrusiveInitialLetterBottom()};
    auto inlineFormattingContext =
      layout::InlineFormattingContext {RootLayoutBox(), GetLayoutState(), parentBlockLayoutState};
    // Temporary, integration only.
    inlineFormattingContext.GetLayoutState().SetNestedListMarkerOffsets(
      _boxGeometryUpdater.TakeNestedListMarkerOffsets());

    auto inlineContentConstraints = [&]() -> layout::ConstraintsForInlineContent
    {
      if (!isPartialLayout || !_inlineContent)
      {
        return *_inlineContentConstraints;
      }
      auto damagedLineIndex = _lineDamage->LayoutStartPosition()->LineIndex;
      if (!damagedLineIndex)
      {
        return *_inlineContentConstraints;
      }
      if (damagedLineIndex >= _inlineContent->DisplayContent().LineList.size())
      {
        krys_debug_assert(false);
        return *_inlineContentConstraints;
      }
      auto constraintsForInFlowContent = layout::ConstraintsForInFlowContent {
        _inlineContentConstraints->Horizontal(), _lineDamage->LayoutStartPosition()->PartialContentTop};
      return {constraintsForInFlowContent, _inlineContentConstraints->VisualLeft(),
              _inlineContentConstraints->FormattingRootBorderBoxSize()};
    };

    auto layoutResult = inlineFormattingContext.Layout(inlineContentConstraints(), _lineDamage.get());
    auto didDiscardContent = layoutResult.DidDiscardContent;
    auto repaintRect =
      LayoutRect {ConstructContent(inlineFormattingContext.GetLayoutState(), krys::move(layoutResult))};

    _lineDamage = nullptr;

    // FIXME: Adjust the lines for pagination (adjustContentForPagination), once there's pagination.

    UpdateRenderTreePositions(inlineFormattingContext.GetLayoutState(), didDiscardContent);

    marginInfo = layout::IntegrationUtils::ToMarginInfo(parentBlockLayoutState.GetMarginState());

    if (!_inlineContent->HasPaintedInlineLevelBoxes())
    {
      // Nothing to repaint. If layout affects painted block level content those issue their own repaints.
      return LayoutRect {};
    }

    return isPartialLayout ? Maybe<LayoutRect> {repaintRect} : null;
  }

  FloatRect LineLayout::ConstructContent(const layout::InlineLayoutState &inlineLayoutState,
                                         layout::InlineLayoutResult &&layoutResult) noexcept
  {
    auto damagedRect =
      InlineContentBuilder {Flow()}.Build(krys::move(layoutResult), EnsureInlineContent(), _lineDamage.get());

    _inlineContent->SetClearGapBeforeFirstLine(inlineLayoutState.ClearGapBeforeFirstLine());
    _inlineContent->SetClearGapAfterLastLine(inlineLayoutState.ClearGapAfterLastLine());
    _inlineContent->ShrinkToFit();

    _inlineContentCache.GetInlineItems().ShrinkToFit();
    _blockFormattingState.ShrinkToFit();

    // FIXME: These needs to be incorporated into the partial damage.
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    damagedRect.Expand(0.f, offsetAndGaps);
    return damagedRect;
  }

  void LineLayout::UpdateRenderTreePositions(const layout::InlineLayoutState &inlineLayoutState,
                                             bool didDiscardContent) noexcept
  {
    if (!_inlineContent && !didDiscardContent)
    {
      return;
    }

    auto &blockFlow = Flow();
    auto placedFloatsWritingMode =
      _blockFormattingState.GetPlacedFloats().BlockFormattingContextRoot().Style().GetWritingMode();

    if (_inlineContent)
    {
      for (auto &box : _inlineContent->DisplayContent().BoxList)
      {
        if (box.IsInlineBox() || box.IsText())
        {
          continue;
        }

        auto &layoutBox = box.LayoutBox();
        if (!layoutBox.IsAtomicInlineBox())
        {
          continue;
        }

        auto &renderer = Downcast<RenderBox>(*box.LayoutBox().RendererForIntegration());

        if (auto *layer = renderer.Layer())
        {
          layer->SetIsHiddenByOverflowTruncation(box.IsFullyTruncated());
        }

        renderer.SetLocation(LayoutPoint {box.VisualRectIgnoringBlockDirection().Location()});
      }
    }

    // FIXME: The floats' pagination offsets, once there's pagination.

    for (auto &layoutBox : layout::FormattingContextBoxes(RootLayoutBox()))
    {
      if (didDiscardContent)
      {
        layoutBox.RendererForIntegration()->ClearNeedsLayout();
      }

      if (!layoutBox.IsFloatingPositioned() && !layoutBox.IsOutOfFlowPositioned())
      {
        continue;
      }
      if (layoutBox.IsLineBreakBox())
      {
        continue;
      }
      auto &renderer = Downcast<RenderBox>(*layoutBox.RendererForIntegration());
      auto &logicalGeometry = GetLayoutState().GeometryForBox(layoutBox);

      if (layoutBox.IsFloatingPositioned())
      {
        // FIXME: Find out what to do with discarded (see line-clamp) floats in render tree.
        auto isInitialLetter = layoutBox.Style().GetPseudoElementType() == PseudoElementType::FirstLetter;
        auto &floatingObject = Flow().InsertFloatingBox(renderer);
        auto [marginBoxVisualRect, borderBoxVisualRect] =
          layout::IntegrationUtils::ToMarginAndBorderBoxVisualRect(
            logicalGeometry, _inlineContentConstraints->FormattingRootBorderBoxSize(),
            placedFloatsWritingMode);

        if (isInitialLetter)
        {
          auto firstLineTrim = LayoutUnit {inlineLayoutState.FirstLineStartTrimForInitialLetter()};
          marginBoxVisualRect.Move(0_lu, -firstLineTrim);
          borderBoxVisualRect.Move(0_lu, -firstLineTrim);
        }

        floatingObject.SetFrameRect(marginBoxVisualRect);
        floatingObject.SetMarginOffset({borderBoxVisualRect.X() - marginBoxVisualRect.X(),
                                        borderBoxVisualRect.Y() - marginBoxVisualRect.Y()});
        floatingObject.SetIsPlaced(true);

        auto oldRect = renderer.FrameRect();
        renderer.SetLocation(borderBoxVisualRect.Location());

        if (renderer.CheckForRepaintDuringLayout())
        {
          auto hasMoved = oldRect.Location() != renderer.Location();
          if (hasMoved)
          {
            renderer.RepaintDuringLayoutIfMoved(oldRect);
          }
          else
          {
            renderer.Repaint();
          }
        }
        continue;
      }

      if (layoutBox.IsOutOfFlowPositioned())
      {
        krys_debug_assert(renderer.Layer());
        auto &layer = *renderer.Layer();
        auto borderBoxLogicalTopLeft = layout::BoxGeometry::BorderBoxTopLeft(logicalGeometry);
        auto previousStaticPosition = LayoutPoint {layer.StaticInlinePosition(), layer.StaticBlockPosition()};
        auto delta = borderBoxLogicalTopLeft - previousStaticPosition;
        auto hasStaticInlinePositioning =
          layoutBox.Style().HasStaticInlinePosition(renderer.IsHorizontalWritingMode());

        if (layoutBox.Style().IsOriginalDisplayInlineType())
        {
          blockFlow.SetStaticInlinePositionForChild(renderer, borderBoxLogicalTopLeft.X());
          if (hasStaticInlinePositioning)
          {
            renderer.Move(delta.Width(), delta.Height());
          }
        }

        layer.SetStaticBlockPosition(borderBoxLogicalTopLeft.Y());
        layer.SetStaticInlinePosition(borderBoxLogicalTopLeft.X());
        continue;
      }
    }
  }

  void LineLayout::PreparePlacedFloats() noexcept
  {
    auto &placedFloats = _blockFormattingState.GetPlacedFloats();
    placedFloats.Clear();

    if (!Flow().ContainsFloats())
    {
      return;
    }

    auto placedFloatsWritingMode = placedFloats.BlockFormattingContextRoot().Style().GetWritingMode();
    auto placedFloatsIsLeftToRight = placedFloatsWritingMode.IsLogicalLeftInlineStart();
    auto isHorizontalWritingMode = placedFloatsWritingMode.IsHorizontal();
    for (auto &floatingObject : *Flow().GetFloatingObjectSet())
    {
      if (!floatingObject->Renderer())
      {
        continue;
      }

      auto &visualRect = floatingObject->FrameRect();

      auto usedPosition = RenderStyle::GetUsedFloat(*floatingObject->Renderer());
      auto logicalPosition = (usedPosition == UsedFloat::Left) == placedFloatsIsLeftToRight
                               ? layout::PlacedFloats::Item::Position::Start
                               : layout::PlacedFloats::Item::Position::End;

      auto boxGeometry = layout::BoxGeometry {};
      auto logicalRect = [&]
      {
        // FIXME: We are flooring here for legacy compatibility. See
        // FloatingObjects::IntervalForFloatingObject and RenderBlockFlow::ClearFloats.
        auto logicalTop = isHorizontalWritingMode ? LayoutUnit(visualRect.Y().Floor()) : visualRect.X();
        auto logicalLeft = isHorizontalWritingMode ? visualRect.X() : LayoutUnit(visualRect.Y().Floor());
        auto logicalHeight =
          (isHorizontalWritingMode ? LayoutUnit(visualRect.MaxY().Floor()) : visualRect.MaxX()) - logicalTop;
        auto logicalWidth =
          (isHorizontalWritingMode ? visualRect.MaxX() : LayoutUnit(visualRect.MaxY().Floor())) - logicalLeft;
        if (!placedFloatsIsLeftToRight)
        {
          auto rootBorderBoxWidth = _inlineContentConstraints->VisualLeft()
                                    + _inlineContentConstraints->Horizontal().LogicalWidth
                                    + _inlineContentConstraints->Horizontal().LogicalLeft;
          logicalLeft = rootBorderBoxWidth - (logicalLeft + logicalWidth);
        }
        return LayoutRect {logicalLeft, logicalTop, logicalWidth, logicalHeight};
      }();

      boxGeometry.SetTopLeft(logicalRect.Location());
      boxGeometry.SetContentBoxWidth(logicalRect.Width());
      boxGeometry.SetContentBoxHeight(logicalRect.Height());
      boxGeometry.SetBorder({});
      boxGeometry.SetPadding({});
      boxGeometry.SetHorizontalMargin({});
      boxGeometry.SetVerticalMargin({});

      auto *shapeOutsideInfo = floatingObject->Renderer()->GetShapeOutsideInfo();
      auto shape = shapeOutsideInfo ? ShareRefPtr(&shapeOutsideInfo->ComputedShape()) : nullptr;

      placedFloats.Add({logicalPosition, boxGeometry, logicalRect.Location(), krys::move(shape)});
    }
  }

  bool LineLayout::IsPaginated() const noexcept
  {
    return _inlineContent && _inlineContent->IsPaginated();
  }

  bool LineLayout::HasEllipsisInBlockDirectionOnLastFormattedLine() const noexcept
  {
    if (!_inlineContent)
    {
      return false;
    }

    for (auto &line : _inlineContent->DisplayContent().LineList | std::views::reverse)
    {
      if (!line.HasContentfulInFlowBox())
      {
        // Out-of-flow content could initiate a line with no inline content.
        continue;
      }
      auto lastFormattedLineEllipsis = line.GetEllipsis();
      return lastFormattedLineEllipsis
             && lastFormattedLineEllipsis->EllipsisType == inline_display::Line::Ellipsis::Type::Block;
    }
    return false;
  }

  Maybe<LayoutUnit> LineLayout::ClampedContentLogicalHeight() const noexcept
  {
    if (!_inlineContent)
    {
      return null;
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      // Out-of-flow only content (and/or with floats) may produce blank inline content.
      return null;
    }

    auto firstTruncatedLineIndex = [&]() -> Maybe<size_t>
    {
      for (size_t lineIndex = 0; lineIndex < lines.size(); ++lineIndex)
      {
        if (lines[lineIndex].IsFullyTruncatedInBlockDirection())
        {
          return lineIndex;
        }
      }
      return null;
    }();
    if (!firstTruncatedLineIndex)
    {
      return null;
    }
    if (!*firstTruncatedLineIndex)
    {
      // This content is fully truncated in the block direction.
      return LayoutUnit {};
    }

    auto contentHeight = lines[*firstTruncatedLineIndex - 1].LineBoxLogicalRect().MaxY()
                         - lines.front().LineBoxLogicalRect().Y();
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    return LayoutUnit {contentHeight + offsetAndGaps};
  }

  LayoutUnit LineLayout::ContentLogicalHeight() const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      // Out-of-flow only content (and/or with floats) may produce blank inline content.
      return {};
    }

    auto contentHeight =
      LastLineWithInflowContent(lines).LineBoxLogicalRect().MaxY() - lines.front().LineBoxLogicalRect().Y();
    auto offsetAndGaps = _inlineContent->FirstLinePaginationOffset() + _inlineContent->ClearBeforeAfterGaps();
    return LayoutUnit {contentHeight + offsetAndGaps};
  }

  bool LineLayout::IsSelfCollapsingContent() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return true;
    }

    auto &displayContent = _inlineContent->DisplayContent();
    for (auto &line : displayContent.LineList)
    {
      if (line.HasContentfulInlineLevelBox())
      {
        return false;
      }

      if (line.HasBlockLevelBox())
      {
        auto blockLevelBox = [&]() -> RenderBox *
        {
          for (auto index = line.FirstBoxIndex(); index < line.LastBoxIndex(); ++index)
          {
            if (displayContent.BoxList[index].IsBlockLevelBox())
            {
              return DynamicDowncast<RenderBox>(
                displayContent.BoxList[index].LayoutBox().RendererForIntegration());
            }
            krys_debug_assert(displayContent.BoxList[index].IsInlineBox());
          }
          return nullptr;
        };
        if (auto *renderBox = blockLevelBox(); renderBox && !renderBox->IsSelfCollapsingBlock())
        {
          return false;
        }
      }
    }
    return true;
  }

  bool LineLayout::HasContentfulInlineOrBlockLine() const noexcept
  {
    return _inlineContent && _inlineContent->HasContentfulInFlowBox();
  }

  bool LineLayout::HasContentfulInlineLine() const noexcept
  {
    return _inlineContent && _inlineContent->HasContentfulInlineLevelBox();
  }

  size_t LineLayout::LineCount() const noexcept
  {
    if (!_inlineContent)
    {
      return 0;
    }
    if (!_inlineContent->HasContentfulInFlowBox())
    {
      return 0;
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    if (lines.empty())
    {
      return 0;
    }
    // In some cases (trailing out-of-flow, non-contentful content after <br>) we produce last line with no
    // content but root inline box only.
    return lines.back().HasContentfulInFlowBox() ? lines.size() : lines.size() - 1;
  }

  bool LineLayout::HasInkOverflow() const noexcept
  {
    return _inlineContent && _inlineContent->HasInkOverflow();
  }

  Maybe<LayoutUnit> LineLayout::FirstLineBaseline() const noexcept
  {
    if (!_inlineContent)
    {
      // FIXME: We should really assert here but table code asks for baseline even before running layout.
      // and we get here by having a LineLayout object initiated by preferred with computation.
      return null;
    }

    auto baselineForLineOrBlock = [&](const inline_display::Line &line) -> Maybe<LayoutUnit>
    {
      if (!line.HasContentfulInFlowBox())
      {
        return null;
      }

      if (auto *blockLevelBox = _inlineContent->BlockLevelBoxForLine(line))
      {
        // For block-in-inline look for the baseline of the child box.
        auto &blockRenderer = Downcast<RenderBox>(*blockLevelBox->LayoutBox().RendererForIntegration());
        return blockRenderer.FirstLineBaseline();
      }
      return LayoutUnit {
        layout::InlineFormattingUtils::SnapToInt(BaselineForLine(line).ToFloat(), RootLayoutBox(),
                                                 layout::InlineFormattingUtils::SnapDirection::Floor)};
    };

    for (auto &line : _inlineContent->DisplayContent().LineList)
    {
      if (auto baseline = baselineForLineOrBlock(line))
      {
        return baseline;
      }
    }

    return Flow().HasLineIfEmpty() ? Maybe<LayoutUnit> {LayoutUnit {BaselineForEmptyContent(Flow())}} : null;
  }

  Maybe<LayoutUnit> LineLayout::LastLineBaseline() const noexcept
  {
    if (!_inlineContent)
    {
      // FIXME: We should really assert here but table code asks for baseline even before running layout.
      // and we get here by having a LineLayout object initiated by preferred with computation.
      return null;
    }

    auto baselineForLineOrBlock = [&](const inline_display::Line &line) -> Maybe<LayoutUnit>
    {
      if (!line.HasContentfulInFlowBox())
      {
        return null;
      }

      if (auto *blockLevelBox = _inlineContent->BlockLevelBoxForLine(line))
      {
        // For block-in-inline look for the baseline of the child box.
        auto &blockRenderer = Downcast<RenderBox>(*blockLevelBox->LayoutBox().RendererForIntegration());
        return blockRenderer.LastLineBaseline();
      }
      return LayoutUnit {
        layout::InlineFormattingUtils::SnapToInt(BaselineForLine(line).ToFloat(), RootLayoutBox(),
                                                 layout::InlineFormattingUtils::SnapDirection::Floor)};
    };

    for (auto &line : _inlineContent->DisplayContent().LineList | std::views::reverse)
    {
      if (auto baseline = baselineForLineOrBlock(line))
      {
        return baseline;
      }
    }

    return Flow().HasLineIfEmpty() ? Maybe<LayoutUnit> {LayoutUnit {BaselineForEmptyContent(Flow())}} : null;
  }

  LayoutUnit LineLayout::BaselineForLine(const inline_display::Line &line) const noexcept
  {
    auto rootWritingMode = RootLayoutBox().GetWritingMode();
    auto baseline = line.Baseline();
    if (rootWritingMode.IsLineInverted())
    {
      baseline = line.LineBoxLogicalRect().Height() - baseline;
    }

    switch (rootWritingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop: return LayoutUnit {line.LineBoxTop() + baseline};
      case FlowDirection::LeftToRight:
      case FlowDirection::RightToLeft: return LayoutUnit {line.LineBoxLeft() + baseline};
    }
    krys_debug_assert(false);
    return {};
  }

  void LineLayout::CollectOverflow() noexcept
  {
    if (!_inlineContent)
    {
      return;
    }

    Flow().AddLayoutOverflow(LayoutRect {_inlineContent->ScrollableOverflow()});
    if (!Flow().HasNonVisibleOverflow())
    {
      Flow().AddVisualOverflow(LayoutRect {_inlineContent->InkOverflow()});
    }
  }

  InlineContent &LineLayout::EnsureInlineContent() noexcept
  {
    if (!_inlineContent)
    {
      _inlineContent = CreateUnique<InlineContent>(Flow());
    }
    return *_inlineContent;
  }

  inline_iterator::TextBoxIterator LineLayout::TextBoxesFor(const RenderText &renderText) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderText.LayoutBox();
    auto firstIndex = _inlineContent->FirstBoxIndexForLayoutBox(layoutBox);
    if (!firstIndex)
    {
      return {};
    }

    return inline_iterator::TextBoxFor(*_inlineContent, *firstIndex);
  }

  inline_iterator::LeafBoxIterator LineLayout::BoxFor(const RenderElement &renderElement) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderElement.LayoutBox();
    auto firstIndex = _inlineContent->FirstBoxIndexForLayoutBox(layoutBox);
    if (!firstIndex)
    {
      return {};
    }

    return inline_iterator::BoxFor(*_inlineContent, *firstIndex);
  }

  inline_iterator::InlineBoxIterator
    LineLayout::FirstInlineBoxFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();
    auto *box = _inlineContent->FirstBoxForLayoutBox(layoutBox);
    if (!box)
    {
      return {};
    }

    return inline_iterator::InlineBoxFor(*_inlineContent, *box);
  }

  inline_iterator::InlineBoxIterator LineLayout::FirstRootInlineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    return inline_iterator::InlineBoxFor(*_inlineContent, _inlineContent->DisplayContent().BoxList[0]);
  }

  inline_iterator::LineBoxIterator LineLayout::FirstLineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    return inline_iterator::LineBoxIterator {inline_iterator::LineBoxIteratorModernPath {*_inlineContent, 0}};
  }

  inline_iterator::LineBoxIterator LineLayout::LastLineBox() const noexcept
  {
    if (!_inlineContent || !_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    auto &lines = _inlineContent->DisplayContent().LineList;
    return inline_iterator::LineBoxIterator {
      inline_iterator::LineBoxIteratorModernPath {*_inlineContent, lines.empty() ? 0 : lines.size() - 1}};
  }

  LayoutRect LineLayout::FirstInlineBoxRect(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();
    auto *firstBox = _inlineContent->FirstBoxForLayoutBox(layoutBox);
    if (!firstBox)
    {
      return {};
    }

    // FIXME: We should be able to flip the display boxes soon after the root block
    // is finished sizing in one go.
    auto firstBoxRect = LayoutRect {firstBox->VisualRectIgnoringBlockDirection()};
    switch (RootLayoutBox().GetWritingMode().BlockDirection())
    {
      case FlowDirection::TopToBottom:
      case FlowDirection::BottomToTop:
      case FlowDirection::LeftToRight: return firstBoxRect;
      case FlowDirection::RightToLeft:
        firstBoxRect.SetX(Flow().Width() - firstBoxRect.MaxX());
        return firstBoxRect;
    }
    krys_debug_assert(false);
    return firstBoxRect;
  }

  LayoutRect LineLayout::EnclosingBorderBoxRectFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    // FIXME: This preserves existing output.
    if (!_inlineContent->HasContentfulInFlowBox())
    {
      return {};
    }

    auto borderBoxLogicalRect = LayoutRect {
      layout::BoxGeometry::BorderBoxRect(GetLayoutState().GeometryForBox(*renderInline.LayoutBox()))};
    return Flow().GetWritingMode().IsHorizontal() ? borderBoxLogicalRect
                                                  : borderBoxLogicalRect.TransposedRect();
  }

  LayoutRect LineLayout::InkOverflowBoundingBoxRectFor(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();

    LayoutRect result;
    _inlineContent->TraverseNonRootInlineBoxes(layoutBox, [&](const inline_display::Box &inlineBox)
                                               { result.Unite(LayoutRect {inlineBox.InkOverflow()}); });

    return result;
  }

  List<FloatRect> LineLayout::CollectInlineBoxRects(const RenderInline &renderInline) const noexcept
  {
    if (!_inlineContent)
    {
      return {};
    }

    auto &layoutBox = *renderInline.LayoutBox();

    List<FloatRect> result;
    _inlineContent->TraverseNonRootInlineBoxes(
      layoutBox, [&](const inline_display::Box &inlineBox)
      { result.push_back(inlineBox.VisualRectIgnoringBlockDirection()); });

    return result;
  }

  void LineLayout::ShiftLinesBy(LayoutUnit blockShift) noexcept
  {
    if (!_inlineContent)
    {
      return;
    }
    bool isHorizontalWritingMode = Flow().GetWritingMode().IsHorizontal();

    for (auto &line : _inlineContent->DisplayContent().LineList)
    {
      line.MoveInBlockDirection(blockShift.ToFloat());
    }

    auto deltaX = isHorizontalWritingMode ? 0_lu : blockShift;
    auto deltaY = isHorizontalWritingMode ? blockShift : 0_lu;
    for (auto &box : _inlineContent->DisplayContent().BoxList)
    {
      if (isHorizontalWritingMode)
      {
        box.MoveVertically(blockShift.ToFloat());
      }
      else
      {
        box.MoveHorizontally(blockShift.ToFloat());
      }

      if (box.IsAtomicInlineBox())
      {
        auto &renderer = Downcast<RenderBox>(*box.LayoutBox().RendererForIntegration());
        renderer.Move(deltaX, deltaY);
      }
    }

    for (auto &layoutBox : layout::FormattingContextBoxes(RootLayoutBox()))
    {
      if (layoutBox.IsOutOfFlowPositioned()
          && layoutBox.Style().HasStaticBlockPosition(isHorizontalWritingMode))
      {
        if (auto *layerRenderer = DynamicDowncast<RenderLayerModelObject>(layoutBox.RendererForIntegration()))
        {
          if (auto *layer = layerRenderer->Layer())
          {
            layer->SetStaticBlockPosition(layer->StaticBlockPosition() + blockShift);
          }
        }
        if (auto *renderBox = DynamicDowncast<RenderBox>(layoutBox.RendererForIntegration()))
        {
          renderBox->Move(deltaX, deltaY);
        }
      }
    }
  }

  const RenderBlockFlow &LineLayout::Flow() const noexcept
  {
    return Downcast<RenderBlockFlow>(*_rootLayoutBox->RendererForIntegration());
  }

  RenderBlockFlow &LineLayout::Flow() noexcept
  {
    return Downcast<RenderBlockFlow>(*_rootLayoutBox->RendererForIntegration());
  }

  void LineLayout::ReleaseCaches(RenderView &view) noexcept
  {
    for (auto &renderer : DescendantsOfType<RenderBlockFlow>(view))
    {
      if (auto *lineLayout = renderer.InlineLayout())
      {
        lineLayout->ReleaseCachesAndResetDamage();
      }
    }
  }

  void LineLayout::ReleaseCachesAndResetDamage() noexcept
  {
    _inlineContentCache.GetInlineItems().Content().clear();
    if (_inlineContent)
    {
      _inlineContent->ReleaseCaches();
    }
    if (_lineDamage)
    {
      layout::InlineInvalidation::ResetInlineDamage(*_lineDamage);
    }
  }

  layout::InlineDamage &LineLayout::EnsureLineDamage() noexcept
  {
    if (!_lineDamage)
    {
      _lineDamage = CreateUnique<layout::InlineDamage>();
    }
    return *_lineDamage;
  }

  void LineLayout::ClearInlineContent() noexcept
  {
    _inlineContent = nullptr;
  }

  bool LineLayout::ContentNeedsVisualReordering() const noexcept
  {
    return _inlineContentCache.GetInlineItems().RequiresVisualReordering();
  }

  bool LineLayout::HasBlocks() const noexcept
  {
    return _inlineContent && _inlineContent->HasBlockLevelBoxes();
  }
}
