/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Line/LineWidth.hpp"

#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Core/Maths/Extras.hpp"
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
