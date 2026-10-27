#include "Krystal.Booey/Rendering/LegacyInlineTextBox.hpp"

#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo
{
  namespace
  {
    using LegacyInlineTextBoxOverflowMap = HashMap<const LegacyInlineTextBox *, LayoutRect>;

    KRYS_NODISCARD LegacyInlineTextBoxOverflowMap &TextBoxesWithOverflow() noexcept
    {
      static NeverDestroyed<LegacyInlineTextBoxOverflowMap> textBoxesWithOverflow;
      return textBoxesWithOverflow.get();
    }
  }

  LegacyInlineTextBox::LegacyInlineTextBox(RenderSVGInlineText &renderer) noexcept : LegacyInlineBox(renderer)
  {
  }

  LegacyInlineTextBox::~LegacyInlineTextBox() noexcept
  {
    if (!KnownToHaveNoOverflow())
    {
      TextBoxesWithOverflow().erase(this);
    }
  }

  RenderSVGInlineText &LegacyInlineTextBox::Renderer() const noexcept
  {
    return Downcast<RenderSVGInlineText>(LegacyInlineBox::Renderer());
  }

  const RenderStyle &LegacyInlineTextBox::LineStyle() const noexcept
  {
    return IsFirstLine() ? Renderer().FirstLineStyle() : Renderer().Style();
  }

  bool LegacyInlineTextBox::HasTextContent() const noexcept
  {
    return _len;
  }

  void LegacyInlineTextBox::MarkDirty(bool dirty) noexcept
  {
    if (dirty)
    {
      _len = 0;
      _start = 0;
    }
    LegacyInlineBox::MarkDirty(dirty);
  }

  void LegacyInlineTextBox::SetLogicalOverflowRect(const LayoutRect &rect) noexcept
  {
    krys_debug_assert(!KnownToHaveNoOverflow());
    TextBoxesWithOverflow().emplace(this, rect);
  }

  LayoutUnit LegacyInlineTextBox::SelectionTop() const noexcept
  {
    return Root().SelectionTop();
  }

  LayoutUnit LegacyInlineTextBox::SelectionBottom() const noexcept
  {
    return Root().SelectionBottom();
  }

  LayoutUnit LegacyInlineTextBox::SelectionHeight() const noexcept
  {
    return Root().SelectionHeight();
  }

  RenderObject::HighlightState LegacyInlineTextBox::SelectionState() const noexcept
  {
    return Renderer().View().Selection().HighlightStateForTextBox(Renderer(), SelectableRange());
  }

  const FontCascade &LegacyInlineTextBox::LineFont() const noexcept
  {
    return LineStyle().GetFontCascade();
  }

  LayoutRect SnappedSelectionRect(const LayoutRect &selectionRect, float logicalRight,
                                  WritingMode writingMode) noexcept
  {
    auto snappedSelectionRect = EnclosingIntRect(selectionRect);
    LayoutUnit logicalWidth {snappedSelectionRect.Width()};
    if (static_cast<float>(snappedSelectionRect.X()) > logicalRight)
    {
      logicalWidth = 0_lu;
    }
    else if (static_cast<float>(snappedSelectionRect.MaxX()) > logicalRight)
    {
      logicalWidth = LayoutUnit {logicalRight - static_cast<float>(snappedSelectionRect.X())};
    }

    if (writingMode.IsHorizontal())
    {
      return {LayoutUnit {snappedSelectionRect.X()}, selectionRect.Y(), logicalWidth, selectionRect.Height()};
    }
    return {selectionRect.Y(), LayoutUnit {snappedSelectionRect.X()}, selectionRect.Height(), logicalWidth};
  }

  LayoutRect LegacyInlineTextBox::LocalSelectionRect(uint32 startPos, uint32 endPos) const noexcept
  {
    auto [clampedStart, clampedEnd] = SelectableRange().Clamp(startPos, endPos);

    if (clampedStart >= clampedEnd
        && !(startPos == endPos && startPos >= Start() && startPos <= (Start() + Len())))
    {
      return {};
    }

    TextRun textRun = CreateTextRun();
    auto writingMode = Renderer().GetWritingMode();
    auto width = LayoutUnit {LogicalWidth()};

    LayoutRect selectionRect {LayoutUnit(), SelectionTop(), width, SelectionHeight()};
    // Avoid measuring the text when the entire line box is selected as an optimization.
    if (clampedStart || clampedEnd != textRun.Length())
    {
      LineFont().AdjustSelectionRectForText(Renderer().CanUseSimplifiedTextMeasuring().value_or(false),
                                            textRun, selectionRect, clampedStart, clampedEnd);
    }

    if (!writingMode.IsLogicalLeftLineLeft())
    {
      selectionRect.SetX(width - selectionRect.X());
    }
    selectionRect.Move(LayoutUnit(LogicalLeft()), LayoutUnit());
    // FIXME: The computation of the snapped selection rect differs from the computation of this rect
    // in paintMarkedTextBackground(). See <https://bugs.webkit.org/show_bug.cgi?id=138913>.
    return SnappedSelectionRect(selectionRect, LogicalRight(), writingMode);
  }

  void LegacyInlineTextBox::DeleteLine() noexcept
  {
    Renderer().RemoveTextBox(*this);
    delete this;
  }

  bool LegacyInlineTextBox::IsLineBreak() const noexcept
  {
    return Renderer().Style().PreserveNewline() && Len() == 1 && Renderer().Text()[Start()] == u8'\n';
  }

  TextBoxSelectableRange LegacyInlineTextBox::SelectableRange() const noexcept
  {
    return {_start, _len, 0u, IsLineBreak()};
  }

  std::pair<uint32, uint32> LegacyInlineTextBox::SelectionStartEnd() const noexcept
  {
    return Renderer().View().Selection().RangeForTextBox(Renderer(), SelectableRange());
  }

  bool LegacyInlineTextBox::HasMarkers() const noexcept
  {
    // FIXME: Document markers (MarkedText::collectForDocumentMarkers), once there are document markers.
    return false;
  }

  int32 LegacyInlineTextBox::CaretMinOffset() const noexcept
  {
    return static_cast<int32>(_start);
  }

  int32 LegacyInlineTextBox::CaretMaxOffset() const noexcept
  {
    return static_cast<int32>(_start + _len);
  }

  float LegacyInlineTextBox::TextPos() const noexcept
  {
    // When computing the width of a text run, RenderBlock::computeInlineDirectionPositionsForLine() doesn't
    // include the actual offset from the containing block edge in its measurement. textPos() should be
    // consistent so the text are rendered in the same width.
    if (!LogicalLeft())
    {
      return 0;
    }
    return LogicalLeft() - Root().LogicalLeft();
  }

  TextRun LegacyInlineTextBox::CreateTextRun() const noexcept
  {
    const auto &style = LineStyle();
    TextRun textRun {Text(),
                     TextPos(),
                     0,
                     ExpansionBehavior::ForbidAll(),
                     Direction(),
                     style.RtlOrdering() == Order::Visual,
                     !Renderer().CanUseSimpleFontCodePath()};
    textRun.SetTabSize(!style.CollapseWhiteSpace(), style::ToPlatform<style::TabSize> {}(style.GetTabSize()));
    return textRun;
  }

  utf8_stringview LegacyInlineTextBox::Text() const noexcept
  {
    return utf8_stringview(Renderer().Text()).substr(_start, _len);
  }
}
