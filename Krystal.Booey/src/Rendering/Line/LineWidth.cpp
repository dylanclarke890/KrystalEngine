#include "Krystal.Booey/Rendering/Line/LineWidth.hpp"

#include "Krystal.Booey/Core/MathExtras.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include <algorithm>

namespace krys::boo
{
  LineWidth::LineWidth(RenderBlockFlow &block) noexcept : _block(ShareCheckedRef(block))
  {
    UpdateAvailableWidth();
  }

  bool LineWidth::FitsOnLine(bool ignoringTrailingSpace) const noexcept
  {
    return ignoringTrailingSpace ? FitsOnLineExcludingTrailingCollapsedWhitespace()
                                 : FitsOnLineIncludingExtraWidth(0);
  }

  bool LineWidth::FitsOnLineIncludingExtraWidth(float extra) const noexcept
  {
    auto adjustedCurrentWidth = CurrentWidth() + extra;
    return adjustedCurrentWidth < _availableWidth
           || AreEssentiallyEqual(adjustedCurrentWidth, _availableWidth);
  }

  bool LineWidth::FitsOnLineExcludingTrailingWhitespace(float extra) const noexcept
  {
    auto adjustedCurrentWidth = CurrentWidth() - _trailingWhitespaceWidth + extra;
    return adjustedCurrentWidth < _availableWidth
           || AreEssentiallyEqual(adjustedCurrentWidth, _availableWidth);
  }

  bool LineWidth::FitsOnLineExcludingTrailingCollapsedWhitespace() const noexcept
  {
    auto adjustedCurrentWidth = CurrentWidth() - _trailingCollapsedWhitespaceWidth;
    return adjustedCurrentWidth < _availableWidth
           || AreEssentiallyEqual(adjustedCurrentWidth, _availableWidth);
  }

  void LineWidth::UpdateAvailableWidth() noexcept
  {
    LayoutUnit height = _block->LogicalHeight();
    auto lineHeight = std::max(0_lu, _block->LineHeight());
    _left = _block->LogicalLeftOffsetForLine(height, lineHeight).ToFloat();
    _right = _block->LogicalRightOffsetForLine(height, lineHeight).ToFloat();

    ComputeAvailableWidthFromLeftAndRight();
  }

  void LineWidth::Commit() noexcept
  {
    _committedWidth += _uncommittedWidth;
    _uncommittedWidth = 0;
    if (_hasUncommittedReplaced)
    {
      _hasCommittedReplaced = true;
      _hasUncommittedReplaced = false;
    }
    _hasCommitted = true;
  }

  void LineWidth::UpdateLineDimension(LayoutUnit newLineTop, LayoutUnit newLineWidth, float newLineLeft,
                                      float newLineRight) noexcept
  {
    if (newLineWidth.ToFloat() <= _availableWidth)
    {
      return;
    }

    _block->SetLogicalHeight(newLineTop);
    _availableWidth = newLineWidth.ToFloat();
    _left = newLineLeft;
    _right = newLineRight;
  }

  void LineWidth::SetTrailingWhitespaceWidth(float collapsedWhitespace, float borderPaddingMargin) noexcept
  {
    _trailingCollapsedWhitespaceWidth = collapsedWhitespace;
    _trailingWhitespaceWidth = collapsedWhitespace + borderPaddingMargin;
  }

  void LineWidth::ComputeAvailableWidthFromLeftAndRight() noexcept
  {
    _availableWidth = std::max<float>(0, _right - _left);
  }
}
