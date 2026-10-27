#pragma once

#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FontBaseline.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo::inline_display
{
  /// @brief A laid out line: its line box, overflow, baseline and content extent, and the range of its boxes
  /// in the display content.
  class Line
  {
  public:
    struct EnclosingTopAndBottom
    {
      // This values encloses the root inline box and any other inline level box's border box.
      float Top {0};
      float Bottom {0};
    };

    struct Ellipsis
    {
      enum class Type : uint8
      {
        Inline,
        Block
      };
      Type EllipsisType {Type::Inline};
      // This is visual rect ignoring block direction.
      FloatRect VisualRect;
      StringAtom Text {StringAtom::Null()};
    };

  private:
    // FIXME: Move these to a side structure.
    size_t _firstBoxIndex {0};
    size_t _boxCount {0};

    // This is line box geometry (see https://www.w3.org/TR/css-inline-3/#line-box).
    FloatRect _lineBoxRect;
    FloatRect _lineBoxLogicalRect;
    FloatRect _scrollableOverflow;
    // FIXME: Merge this with scrollable overflow (see InlineContentBuilder::updateLineOverflow).
    FloatRect _contentOverflow;
    // FIXME: This should be transitioned to spec aligned overflow value.
    FloatRect _inkOverflow;
    // Enclosing top and bottom includes all inline level boxes (border box) vertically.
    // While the line box usually enclose them as well, its vertical geometry is based on
    // the layout bounds of the inline level boxes which may be different when line-height is present.
    EnclosingTopAndBottom _enclosingLogicalTopAndBottom;
    float _alignmentBaseline {0.f};
    // Content is mostly in flush with the line box edge except for cases like text-align.
    float _contentLogicalLeft {0.f};
    float _contentLogicalLeftIgnoringInlineDirection {0.f};
    float _contentLogicalWidth {0.f};
    FontBaseline _baselineType {FontBaseline::Alphabetic};
    bool _isLeftToRightDirection : 1 {true};
    bool _isHorizontal : 1 {true};
    bool _isFirstAfterPageBreak : 1 {false};
    bool _isFullyTruncatedInBlockDirection : 1 {false};
    bool _hasContentAfterEllipsisBox : 1 {false};
    bool _hasInflowBox : 1 {false};
    bool _hasContentfulBox : 1 {false};
    bool _hasBlockLevelBox : 1 {false};
    Maybe<Ellipsis> _ellipsis {};

  public:
    Line(bool hasInflowBox, bool hasContentfulBox, bool hasBlockLevelBox, const FloatRect &lineBoxLogicalRect,
         const FloatRect &lineBoxRect, const FloatRect &contentOverflow,
         EnclosingTopAndBottom enclosingLogicalTopAndBottom, float alignmentBaseline,
         FontBaseline baselineType, float contentLogicalLeft, float contentLogicalLeftIgnoringInlineDirection,
         float contentLogicalWidth, bool isLeftToRightDirection, bool isHorizontal,
         bool isTruncatedInBlockDirection) noexcept
        : _lineBoxRect(lineBoxRect), _lineBoxLogicalRect(lineBoxLogicalRect),
          _contentOverflow(contentOverflow), _enclosingLogicalTopAndBottom(enclosingLogicalTopAndBottom),
          _alignmentBaseline(alignmentBaseline), _contentLogicalLeft(contentLogicalLeft),
          _contentLogicalLeftIgnoringInlineDirection(contentLogicalLeftIgnoringInlineDirection),
          _contentLogicalWidth(contentLogicalWidth), _baselineType(baselineType),
          _isLeftToRightDirection(isLeftToRightDirection), _isHorizontal(isHorizontal),
          _isFullyTruncatedInBlockDirection(isTruncatedInBlockDirection), _hasInflowBox(hasInflowBox),
          _hasContentfulBox(hasContentfulBox), _hasBlockLevelBox(hasBlockLevelBox)
    {
    }

    KRYS_NODISCARD float Left() const noexcept
    {
      return _lineBoxRect.X();
    }
    KRYS_NODISCARD float Right() const noexcept
    {
      return _lineBoxRect.MaxX();
    }
    KRYS_NODISCARD float Top() const noexcept
    {
      return _lineBoxRect.Y();
    }
    KRYS_NODISCARD float Bottom() const noexcept
    {
      return _lineBoxRect.MaxY();
    }
    KRYS_NODISCARD FloatPoint TopLeft() const noexcept
    {
      return _lineBoxRect.Location();
    }

    KRYS_NODISCARD float LineBoxTop() const noexcept
    {
      return _lineBoxRect.Y();
    }
    KRYS_NODISCARD float LineBoxBottom() const noexcept
    {
      return _lineBoxRect.MaxY();
    }
    KRYS_NODISCARD float LineBoxLeft() const noexcept
    {
      return _lineBoxRect.X();
    }
    KRYS_NODISCARD float LineBoxRight() const noexcept
    {
      return _lineBoxRect.MaxX();
    }
    KRYS_NODISCARD float LineBoxHeight() const noexcept
    {
      return _lineBoxRect.Height();
    }
    KRYS_NODISCARD float LineBoxWidth() const noexcept
    {
      return _lineBoxRect.Width();
    }

    KRYS_NODISCARD const FloatRect &LineBoxRect() const noexcept
    {
      return _lineBoxRect;
    }
    KRYS_NODISCARD const FloatRect &LineBoxLogicalRect() const noexcept
    {
      return _lineBoxLogicalRect;
    }
    KRYS_NODISCARD const FloatRect &ScrollableOverflow() const noexcept
    {
      return _scrollableOverflow;
    }
    KRYS_NODISCARD const FloatRect &ContentOverflow() const noexcept
    {
      return _contentOverflow;
    }
    KRYS_NODISCARD const FloatRect &InkOverflow() const noexcept
    {
      return _inkOverflow;
    }

    KRYS_NODISCARD FloatRect VisibleRectIgnoringBlockDirection() const noexcept
    {
      if (_isFullyTruncatedInBlockDirection)
      {
        return {};
      }
      if (!HasEllipsis() || HasContentAfterEllipsisBox())
      {
        return _inkOverflow;
      }
      if (_isLeftToRightDirection)
      {
        auto visibleLineBoxRight = std::min(_lineBoxRect.MaxX(), _ellipsis->VisualRect.MaxX());
        return {_lineBoxRect.Location(), FloatPoint {visibleLineBoxRight, _lineBoxRect.MaxY()}};
      }
      auto visibleLineBoxLeft = std::max(_lineBoxRect.X(), _ellipsis->VisualRect.X());
      return {FloatPoint {visibleLineBoxLeft, _lineBoxRect.Y()},
              FloatPoint {_lineBoxRect.MaxX(), _lineBoxRect.MaxY()}};
    }

    KRYS_NODISCARD float EnclosingContentLogicalTop() const noexcept
    {
      return _enclosingLogicalTopAndBottom.Top;
    }
    KRYS_NODISCARD float EnclosingContentLogicalBottom() const noexcept
    {
      return _enclosingLogicalTopAndBottom.Bottom;
    }

    KRYS_NODISCARD float Baseline() const noexcept
    {
      return _alignmentBaseline;
    }
    KRYS_NODISCARD FontBaseline BaselineType() const noexcept
    {
      return _baselineType;
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _isHorizontal;
    }
    KRYS_NODISCARD bool IsLeftToRightInlineDirection() const noexcept
    {
      return _isLeftToRightDirection;
    }

    KRYS_NODISCARD float ContentLogicalLeft() const noexcept
    {
      return _contentLogicalLeft;
    }
    // This is "visual" left in inline direction (it is still considered logical as there's no flip for
    // writing mode).
    KRYS_NODISCARD float ContentLogicalLeftIgnoringInlineDirection() const noexcept
    {
      return _contentLogicalLeftIgnoringInlineDirection;
    }
    KRYS_NODISCARD float ContentLogicalWidth() const noexcept
    {
      return _contentLogicalWidth;
    }

    KRYS_NODISCARD size_t FirstBoxIndex() const noexcept
    {
      return _firstBoxIndex;
    }
    KRYS_NODISCARD size_t LastBoxIndex() const noexcept
    {
      return FirstBoxIndex() + BoxCount() - 1;
    }
    KRYS_NODISCARD size_t BoxCount() const noexcept
    {
      return _boxCount;
    }

    KRYS_NODISCARD bool IsFirstAfterPageBreak() const noexcept
    {
      return _isFirstAfterPageBreak;
    }

    void MoveInBlockDirection(float offset) noexcept
    {
      if (!offset)
      {
        return;
      }
      auto physicalOffset = IsHorizontal() ? FloatSize {{}, offset} : FloatSize {offset, {}};
      _lineBoxRect.Move(physicalOffset);
      _scrollableOverflow.Move(physicalOffset);
      _contentOverflow.Move(physicalOffset);
      _inkOverflow.Move(physicalOffset);
      if (_ellipsis)
      {
        _ellipsis->VisualRect.Move(physicalOffset);
      }
      _lineBoxLogicalRect.Move({{}, offset});
      _enclosingLogicalTopAndBottom.Top += offset;
      _enclosingLogicalTopAndBottom.Bottom += offset;
    }

    void ShrinkInBlockDirection(float delta) noexcept
    {
      if (!delta)
      {
        return;
      }
      auto physicalDelta = IsHorizontal() ? FloatSize {{}, delta} : FloatSize {delta, {}};
      _lineBoxRect.Contract(physicalDelta);
      _scrollableOverflow.Contract(physicalDelta);
      _contentOverflow.Contract(physicalDelta);
      _inkOverflow.Contract(physicalDelta);
      if (_ellipsis)
      {
        _ellipsis->VisualRect.Contract(physicalDelta);
      }
      _lineBoxLogicalRect.Contract({{}, delta});
      _enclosingLogicalTopAndBottom.Bottom -= delta;
    }

    void SetEllipsis(const Ellipsis &ellipsis) noexcept
    {
      _ellipsis = ellipsis;
    }
    KRYS_NODISCARD Maybe<Ellipsis> GetEllipsis() const noexcept
    {
      return _ellipsis;
    }
    KRYS_NODISCARD bool HasEllipsis() const noexcept
    {
      return _ellipsis.has_value();
    }
    KRYS_NODISCARD bool IsFullyTruncatedInBlockDirection() const noexcept
    {
      return _isFullyTruncatedInBlockDirection;
    }
    KRYS_NODISCARD bool HasContentAfterEllipsisBox() const noexcept
    {
      return _hasContentAfterEllipsisBox;
    }
    void SetHasContentAfterEllipsisBox() noexcept
    {
      _hasContentAfterEllipsisBox = true;
    }

    KRYS_NODISCARD bool HasInflowBox() const noexcept
    {
      return _hasInflowBox;
    }
    KRYS_NODISCARD bool HasContentfulInFlowBox() const noexcept
    {
      return _hasContentfulBox && _hasInflowBox;
    }
    KRYS_NODISCARD bool HasInlineLevelBox() const noexcept
    {
      return HasInflowBox() && !HasBlockLevelBox();
    }
    KRYS_NODISCARD bool HasContentfulInlineLevelBox() const noexcept
    {
      return HasContentfulInFlowBox() && HasInlineLevelBox();
    }
    KRYS_NODISCARD bool HasBlockLevelBox() const noexcept
    {
      return _hasBlockLevelBox;
    }

    void SetFirstBoxIndex(size_t firstBoxIndex) noexcept
    {
      _firstBoxIndex = firstBoxIndex;
    }
    void SetBoxCount(size_t boxCount) noexcept
    {
      _boxCount = boxCount;
    }
    void SetIsFirstAfterPageBreak() noexcept
    {
      _isFirstAfterPageBreak = true;
    }
    void SetInkOverflow(const FloatRect &inkOverflowRect) noexcept
    {
      _inkOverflow = inkOverflowRect;
    }
    void SetScrollableOverflow(const FloatRect &scrollableOverflow) noexcept
    {
      _scrollableOverflow = scrollableOverflow;
    }

    void SetLineBoxRectForSVGText(const FloatRect &rect) noexcept
    {
      _lineBoxRect = rect;
      _scrollableOverflow = rect;
      _contentOverflow = rect;
      _inkOverflow = rect;
      _lineBoxLogicalRect = _isHorizontal ? rect : rect.TransposedRect();
      _enclosingLogicalTopAndBottom.Top = _lineBoxLogicalRect.Y();
      _enclosingLogicalTopAndBottom.Bottom = _lineBoxLogicalRect.MaxY();
    }
  };
}
