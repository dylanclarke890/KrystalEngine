#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief The width the legacy line layout has used on a line and the width it has.
  class LineWidth
  {
  private:
    const CheckedRef<RenderBlockFlow> _block;
    float _uncommittedWidth {0};
    float _committedWidth {0};
    float _trailingWhitespaceWidth {0};
    float _trailingCollapsedWhitespaceWidth {0};
    float _left {0};
    float _right {0};
    float _availableWidth {0};
    bool _hasCommitted {false};
    bool _hasCommittedReplaced {false};
    bool _hasUncommittedReplaced {false};

  public:
    explicit LineWidth(RenderBlockFlow &) noexcept;

    KRYS_NODISCARD bool FitsOnLine(bool ignoringTrailingSpace = false) const noexcept;
    KRYS_NODISCARD bool FitsOnLineIncludingExtraWidth(float extra) const noexcept;
    KRYS_NODISCARD bool FitsOnLineExcludingTrailingWhitespace(float extra) const noexcept;

    KRYS_NODISCARD float CurrentWidth() const noexcept
    {
      return _committedWidth + _uncommittedWidth;
    }
    // FIXME: We should eventually replace these three functions by ones that work on a higher abstraction.
    KRYS_NODISCARD float UncommittedWidth() const noexcept
    {
      return _uncommittedWidth;
    }
    KRYS_NODISCARD float CommittedWidth() const noexcept
    {
      return _committedWidth;
    }
    KRYS_NODISCARD float AvailableWidth() const noexcept
    {
      return _availableWidth;
    }
    KRYS_NODISCARD float LogicalLeftOffset() const noexcept
    {
      return _left;
    }

    KRYS_NODISCARD bool HasCommitted() const noexcept
    {
      return _hasCommitted;
    }
    KRYS_NODISCARD bool HasCommittedReplaced() const noexcept
    {
      return _hasCommittedReplaced;
    }

    void UpdateAvailableWidth() noexcept;
    void AddUncommittedWidth(float delta) noexcept
    {
      _uncommittedWidth += delta;
    }
    void AddUncommittedReplacedWidth(float delta) noexcept
    {
      AddUncommittedWidth(delta);
      _hasUncommittedReplaced = true;
    }
    void Commit() noexcept;
    void SetTrailingWhitespaceWidth(float collapsedWhitespace, float borderPaddingMargin = 0) noexcept;

  private:
    void ComputeAvailableWidthFromLeftAndRight() noexcept;
    KRYS_NODISCARD bool FitsOnLineExcludingTrailingCollapsedWhitespace() const noexcept;
    void UpdateLineDimension(LayoutUnit newLineTop, LayoutUnit newLineWidth, float newLineLeft,
                             float newLineRight) noexcept;
  };
}
