#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorTextBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::inline_iterator
{
  float LineBoxIteratorModernPath::ContentLogicalTopAdjustedForPrecedingLineBox() const noexcept
  {
    if (FormattingContextRoot().GetWritingMode().IsLineInverted() || !_lineIndex)
    {
      return ContentLogicalTop();
    }
    return LineBoxIteratorModernPath {*_inlineContent, _lineIndex - 1}.ContentLogicalBottom();
  }

  float LineBoxIteratorModernPath::ContentLogicalBottomAdjustedForFollowingLineBox() const noexcept
  {
    if (!FormattingContextRoot().GetWritingMode().IsLineInverted() || _lineIndex == Lines().size() - 1)
    {
      return ContentLogicalBottom();
    }
    return LineBoxIteratorModernPath {*_inlineContent, _lineIndex + 1}.ContentLogicalTop();
  }

  float LineBoxIteratorModernPath::ContentLogicalLeft() const noexcept
  {
    auto writingMode = FormattingContextRoot().GetWritingMode();
    if (writingMode.IsLogicalLeftLineLeft())
    {
      return GetLine().LineBoxLeft() + GetLine().ContentLogicalLeftIgnoringInlineDirection();
    }
    krys_debug_assert(writingMode.IsVertical()); // Currently only sideways-lr gets this far.
    return GetLine().Bottom()
           - (GetLine().ContentLogicalLeftIgnoringInlineDirection() + GetLine().ContentLogicalWidth());
  }

  BoxModernPath LineBoxIteratorModernPath::FirstLeafBox() const noexcept
  {
    if (!GetLine().BoxCount())
    {
      return BoxModernPath {*_inlineContent};
    }
    auto runIterator = BoxModernPath {*_inlineContent, GetLine().FirstBoxIndex()};
    if (runIterator.GetBox().IsInlineBox())
    {
      runIterator.TraverseNextLeafOnLine();
    }
    return runIterator;
  }

  BoxModernPath LineBoxIteratorModernPath::LastLeafBox() const noexcept
  {
    auto boxCount = GetLine().BoxCount();
    if (!boxCount)
    {
      return BoxModernPath {*_inlineContent};
    }
    auto runIterator = BoxModernPath {*_inlineContent, GetLine().FirstBoxIndex() + boxCount - 1};
    if (runIterator.GetBox().IsInlineBox())
    {
      runIterator.TraversePreviousLeafOnLine();
    }
    return runIterator;
  }

  const RenderStyle &LineBox::Style() const noexcept
  {
    return IsFirst() ? FormattingContextRoot().FirstLineStyle() : FormattingContextRoot().Style();
  }

  FloatRect LineBox::EllipsisVisualRect(AdjustedForSelection adjustedForSelection) const noexcept
  {
    krys_debug_assert(HasEllipsis());

    auto visualRect = SwitchOn(_pathVariant, [](const auto &path)
                               { return path.EllipsisVisualRectIgnoringBlockDirection(); });

    // FIXME: Add pixel snapping here.
    if (adjustedForSelection == AdjustedForSelection::No)
    {
      FormattingContextRoot().FlipForWritingMode(visualRect);
      return visualRect;
    }
    auto selectionTop = FormattingContextRoot().AdjustEnclosingTopForPrecedingBlock(
      LayoutUnit {ContentLogicalTopAdjustedForPrecedingLineBox()});
    auto selectionBottom = ContentLogicalBottomAdjustedForFollowingLineBox();

    visualRect.SetY(static_cast<float>(selectionTop));
    visualRect.SetHeight(selectionBottom - static_cast<float>(selectionTop));
    FormattingContextRoot().FlipForWritingMode(visualRect);
    return visualRect;
  }

  RenderObjectHighlightState LineBox::EllipsisSelectionState() const noexcept
  {
    auto lastLeafBox = LineRightmostLeafBox();
    if (!lastLeafBox)
    {
      return RenderObjectHighlightState::None;
    }

    auto *text = DynamicDowncast<TextBox>(*lastLeafBox);
    if (!text || text->SelectionState() == RenderObjectHighlightState::None)
    {
      return RenderObjectHighlightState::None;
    }

    auto selectionRange = text->SelectableRange();
    if (!selectionRange.Truncation)
    {
      return RenderObjectHighlightState::None;
    }

    auto [selectionStart, selectionEnd] =
      FormattingContextRoot().View().Selection().RangeForTextBox(text->Renderer(), selectionRange);
    return selectionStart <= *selectionRange.Truncation && selectionEnd >= *selectionRange.Truncation
             ? RenderObjectHighlightState::Inside
             : RenderObjectHighlightState::None;
  }

  bool LineBox::IsFirst() const noexcept
  {
    return !Previous();
  }

  LineBoxIterator LineBox::Next() const noexcept
  {
    return LineBoxIterator(*this).TraverseNext();
  }

  LineBoxIterator LineBox::Previous() const noexcept
  {
    return LineBoxIterator(*this).TraversePrevious();
  }

  LeafBoxIterator LineBox::LineLeftmostLeafBox() const noexcept
  {
    return SwitchOn(_pathVariant, [](const auto &path) { return LeafBoxIterator {path.FirstLeafBox()}; });
  }

  LeafBoxIterator LineBox::LineRightmostLeafBox() const noexcept
  {
    return SwitchOn(_pathVariant, [](const auto &path) { return LeafBoxIterator {path.LastLeafBox()}; });
  }

  LeafBoxIterator LineBox::LogicalLeftmostLeafBox() const noexcept
  {
    return FormattingContextRoot().GetWritingMode().IsLogicalLeftLineLeft() ? LineLeftmostLeafBox()
                                                                            : LineRightmostLeafBox();
  }

  LeafBoxIterator LineBox::LogicalRightmostLeafBox() const noexcept
  {
    return FormattingContextRoot().GetWritingMode().IsLogicalLeftLineLeft() ? LineRightmostLeafBox()
                                                                            : LineLeftmostLeafBox();
  }

  LeafBoxIterator LineBox::BlockLevelBox() const noexcept
  {
    if (!HasBlockContent())
    {
      return {};
    }
    auto blockBox = LineRightmostLeafBox();
    if (!blockBox || !blockBox->IsBlockLevelBox())
    {
      krys_debug_assert(false);
      return {};
    }
    return blockBox;
  }

  LineBoxIterator &LineBoxIterator::TraverseNext() noexcept
  {
    SwitchOn(_lineBox._pathVariant, [](auto &path) { path.TraverseNext(); });
    return *this;
  }

  LineBoxIterator &LineBoxIterator::TraversePrevious() noexcept
  {
    SwitchOn(_lineBox._pathVariant, [](auto &path) { path.TraversePrevious(); });
    return *this;
  }

  LineBoxIterator FirstLineBoxFor(const RenderBlockFlow &flow) noexcept
  {
    if (auto *lineLayout = flow.InlineLayout())
    {
      return lineLayout->FirstLineBox();
    }
    return LineBoxIterator {LineBoxIteratorLegacyPath {flow.LegacyRootBox()}};
  }

  LineBoxIterator LastLineBoxFor(const RenderBlockFlow &flow) noexcept
  {
    if (auto *lineLayout = flow.InlineLayout())
    {
      return lineLayout->LastLineBox();
    }
    return LineBoxIterator {LineBoxIteratorLegacyPath {flow.LegacyRootBox()}};
  }

  LineBoxIterator LineBoxFor(const layout_integration::InlineContent &inlineContent,
                             size_t lineIndex) noexcept
  {
    return LineBoxIterator {LineBoxIteratorModernPath {inlineContent, lineIndex}};
  }

  float PreviousLineBoxContentBottomOrBorderAndPadding(const LineBox &lineBox) noexcept
  {
    return lineBox.IsFirst() ? lineBox.FormattingContextRoot().BorderAndPaddingBefore().ToFloat()
                             : lineBox.ContentLogicalTopAdjustedForPrecedingLineBox();
  }

  float ContentStartInBlockDirection(const LineBox &lineBox) noexcept
  {
    if (!lineBox.FormattingContextRoot().GetWritingMode().IsBlockFlipped())
    {
      return std::max(lineBox.ContentLogicalTop(), PreviousLineBoxContentBottomOrBorderAndPadding(lineBox));
    }
    return std::min(lineBox.ContentLogicalBottom(),
                    lineBox.ContentLogicalBottomAdjustedForFollowingLineBox());
  }

  LeafBoxIterator ClosestBoxForHorizontalPosition(const LineBox &lineBox, float horizontalPosition,
                                                  bool editableOnly) noexcept
  {
    auto isEditable = [&](const LeafBoxIterator &box)
    {
      return box && box->Renderer().GetNode() && box->Renderer().GetNode()->HasEditableStyle();
    };

    auto firstBox = lineBox.LogicalLeftmostLeafBox();
    auto lastBox = lineBox.LogicalRightmostLeafBox();

    if (firstBox != lastBox)
    {
      if (firstBox->IsLineBreak())
      {
        firstBox = firstBox->NextLogicalRightwardOnLineIgnoringLineBreak();
      }
      else if (lastBox->IsLineBreak())
      {
        lastBox = lastBox->NextLogicalLeftwardOnLineIgnoringLineBreak();
      }
    }

    if (firstBox == lastBox && (!editableOnly || isEditable(firstBox)))
    {
      return firstBox;
    }

    if (firstBox && horizontalPosition <= firstBox->LogicalLeft()
        && !firstBox->Renderer().IsRenderListMarker() && (!editableOnly || isEditable(firstBox)))
    {
      return firstBox;
    }

    if (lastBox && horizontalPosition >= lastBox->LogicalRight() && !lastBox->Renderer().IsRenderListMarker()
        && (!editableOnly || isEditable(lastBox)))
    {
      return lastBox;
    }

    auto closestBox = lastBox;
    for (auto box = firstBox; box; box = box.TraverseLogicalRightwardOnLineIgnoringLineBreak())
    {
      if (!box->Renderer().IsRenderListMarker() && (!editableOnly || isEditable(box)))
      {
        if (horizontalPosition < box->LogicalRight())
        {
          return box;
        }
        closestBox = box;
      }
    }

    return closestBox;
  }
}
