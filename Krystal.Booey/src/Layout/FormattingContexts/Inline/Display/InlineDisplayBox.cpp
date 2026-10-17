#include "Krystal.Booey/Layout/FormattingContexts/Inline/Display/InlineDisplayBox.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::inline_display
{
  bool Box::IsHorizontal() const noexcept
  {
    return GetWritingMode().IsHorizontal();
  }

  bool Box::IsVisible() const noexcept
  {
    return !IsFullyTruncated() && Style().UsedVisibility() == Visibility::Visible;
  }

  bool Box::IsVisibleIgnoringUsedVisibility() const noexcept
  {
    return !IsFullyTruncated() && Style().GetVisibility() == Visibility::Visible;
  }

  boo::WritingMode Box::GetWritingMode() const noexcept
  {
    return Style().GetWritingMode();
  }

  bool operator==(const Box &a, const Box &b) noexcept
  {
    return &a.LayoutBox() == &b.LayoutBox() && a.Style() == b.Style() && a.LineIndex() == b.LineIndex()
           && a.BidiLevel() == b.BidiLevel()
           && a.VisualRectIgnoringBlockDirection() == b.VisualRectIgnoringBlockDirection()
           && a.InkOverflow() == b.InkOverflow() && a.IsFullyTruncated() == b.IsFullyTruncated()
           && (!a.IsTextOrSoftLineBreak() || (a.GetText() == b.GetText()));
  }
}
