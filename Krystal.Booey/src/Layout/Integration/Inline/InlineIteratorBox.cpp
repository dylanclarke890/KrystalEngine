#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorInlineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorLineBox.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/LayoutIntegrationLineLayout.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Values/Text/TabSize.hpp"

namespace krys::boo::inline_iterator
{
  const RenderText &BoxModernPath::GetRenderText() const noexcept
  {
    return Downcast<RenderText>(Renderer());
  }

  TextRun BoxModernPath::GetTextRun(TextRunMode mode) const noexcept
  {
    auto &style = GetBox().Style();
    auto expansion = GetBox().GetExpansion();
    auto logicalLeft = [&]
    {
      if (style.GetWritingMode().IsBidiLTR())
      {
        return VisualRectIgnoringBlockDirection().X()
               - (GetLine().LineBoxLeft() + GetLine().ContentLogicalLeft());
      }
      return GetLine().LineBoxRight()
             - (VisualRectIgnoringBlockDirection().MaxX() + GetLine().ContentLogicalLeft());
    };
    auto characterScanForCodePath = IsText() && !GetRenderText().CanUseSimpleFontCodePath();
    auto textRun =
      TextRun {mode == TextRunMode::Editing ? OriginalText() : GetBox().GetText().RenderedContent(),
               logicalLeft(),
               expansion.HorizontalExpansion,
               expansion.Behavior,
               Direction(),
               style.RtlOrdering() == Order::Visual,
               characterScanForCodePath};
    textRun.SetTabSize(!style.CollapseWhiteSpace(), style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
    return textRun;
  }

  TextBoxSelectableRange BoxModernPath::SelectableRange() const noexcept
  {
    auto &box = GetBox();
    auto &textContent = box.GetText();
    auto extraTrailingLength = [&]() -> uint32
    {
      if (textContent.HasHyphen())
      {
        return static_cast<uint32>(box.Style().HyphenString().View().size());
      }
      if (Downcast<layout::InlineTextBox>(box.LayoutBox()).IsCombined())
      {
        krys_debug_assert(textContent.RenderedContent().size() >= Length());
        return static_cast<uint32>(textContent.RenderedContent().size() - Length());
      }
      return 0;
    };
    auto partiallyVisibleContentLength = textContent.PartiallyVisibleContentLength();
    return {Start(),
            Length(),
            extraTrailingLength(),
            box.IsLineBreak(),
            partiallyVisibleContentLength ? Maybe<uint32>(static_cast<uint32>(*partiallyVisibleContentLength))
                                          : null,
            FormattingContextRoot().GetWritingMode().BidiDirection() != Direction()};
  }

  bool BoxIterator::operator==(const BoxIterator &other) const noexcept
  {
    if (AtEnd() && other.AtEnd())
    {
      return true;
    }

    return _box._path == other._box._path;
  }

  BoxIterator &BoxIterator::TraverseLineRightwardOnLine() noexcept
  {
    _box._path.TraverseNextBoxOnLine();
    return *this;
  }

  BoxIterator &BoxIterator::TraverseLineRightwardOnLineSkippingChildren() noexcept
  {
    _box._path.TraverseNextBoxOnLineSkippingChildren();
    return *this;
  }

  WritingMode Box::GetWritingMode() const noexcept
  {
    return Style().GetWritingMode();
  }

  LeafBoxIterator Box::NextLineRightwardOnLine() const noexcept
  {
    return LeafBoxIterator(*this).TraverseLineRightwardOnLine();
  }

  LeafBoxIterator Box::NextLineLeftwardOnLine() const noexcept
  {
    return LeafBoxIterator(*this).TraverseLineLeftwardOnLine();
  }

  LeafBoxIterator Box::NextLineRightwardOnLineIgnoringLineBreak() const noexcept
  {
    return LeafBoxIterator(*this).TraverseLineRightwardOnLineIgnoringLineBreak();
  }

  LeafBoxIterator Box::NextLineLeftwardOnLineIgnoringLineBreak() const noexcept
  {
    return LeafBoxIterator(*this).TraverseLineLeftwardOnLineIgnoringLineBreak();
  }

  LeafBoxIterator Box::NextLogicalRightwardOnLine() const noexcept
  {
    return GetWritingMode().IsLogicalLeftLineLeft() ? NextLineRightwardOnLine() : NextLineLeftwardOnLine();
  }

  LeafBoxIterator Box::NextLogicalLeftwardOnLine() const noexcept
  {
    return GetWritingMode().IsLogicalLeftLineLeft() ? NextLineLeftwardOnLine() : NextLineRightwardOnLine();
  }

  LeafBoxIterator Box::NextLogicalRightwardOnLineIgnoringLineBreak() const noexcept
  {
    return GetWritingMode().IsLogicalLeftLineLeft() ? NextLineRightwardOnLineIgnoringLineBreak()
                                                    : NextLineLeftwardOnLineIgnoringLineBreak();
  }

  LeafBoxIterator Box::NextLogicalLeftwardOnLineIgnoringLineBreak() const noexcept
  {
    return GetWritingMode().IsLogicalLeftLineLeft() ? NextLineLeftwardOnLineIgnoringLineBreak()
                                                    : NextLineRightwardOnLineIgnoringLineBreak();
  }

  InlineBoxIterator Box::ParentInlineBox() const noexcept
  {
    return InlineBoxIterator {_path.ParentInlineBox()};
  }

  LineBoxIterator Box::GetLineBox() const noexcept
  {
    return LineBoxIterator {LineBoxIteratorModernPath {_path.GetInlineContent(), _path.GetBox().LineIndex()}};
  }

  FloatRect Box::VisualRect() const noexcept
  {
    auto rect = VisualRectIgnoringBlockDirection();
    FormattingContextRoot().FlipForWritingMode(rect);
    return rect;
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLineRightwardOnLine() noexcept
  {
    _box._path.TraverseNextLeafOnLine();
    return *this;
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLineLeftwardOnLine() noexcept
  {
    _box._path.TraversePreviousLeafOnLine();
    return *this;
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLineRightwardOnLineIgnoringLineBreak() noexcept
  {
    do
    {
      TraverseLineRightwardOnLine();
    } while (!AtEnd() && _box.IsLineBreak());
    return *this;
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLineLeftwardOnLineIgnoringLineBreak() noexcept
  {
    do
    {
      TraverseLineLeftwardOnLine();
    } while (!AtEnd() && _box.IsLineBreak());
    return *this;
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLogicalRightwardOnLine() noexcept
  {
    return _box.GetWritingMode().IsLogicalLeftLineLeft() ? TraverseLineRightwardOnLine()
                                                         : TraverseLineLeftwardOnLine();
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLogicalLeftwardOnLine() noexcept
  {
    return _box.GetWritingMode().IsLogicalLeftLineLeft() ? TraverseLineLeftwardOnLine()
                                                         : TraverseLineRightwardOnLine();
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLogicalRightwardOnLineIgnoringLineBreak() noexcept
  {
    return _box.GetWritingMode().IsLogicalLeftLineLeft() ? TraverseLineRightwardOnLineIgnoringLineBreak()
                                                         : TraverseLineLeftwardOnLineIgnoringLineBreak();
  }

  LeafBoxIterator &LeafBoxIterator::TraverseLogicalLeftwardOnLineIgnoringLineBreak() noexcept
  {
    return _box.GetWritingMode().IsLogicalLeftLineLeft() ? TraverseLineLeftwardOnLineIgnoringLineBreak()
                                                         : TraverseLineRightwardOnLineIgnoringLineBreak();
  }

  LeafBoxIterator BoxFor(const RenderLineBreak &renderer) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(renderer))
    {
      return lineLayout->BoxFor(renderer);
    }
    return {};
  }

  LeafBoxIterator BoxFor(const RenderBox &renderer) noexcept
  {
    if (auto *lineLayout = layout_integration::LineLayout::Containing(renderer))
    {
      return lineLayout->BoxFor(renderer);
    }
    return {};
  }

  LeafBoxIterator BoxFor(const layout_integration::InlineContent &content, size_t boxIndex) noexcept
  {
    return LeafBoxIterator {BoxModernPath {content, boxIndex}};
  }
}
