#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FloatPoint.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/LayoutRect.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class LegacyInlineFlowBox;
  class LegacyRootInlineBox;
  class RenderBoxModelObject;
  class RenderStyle;

  namespace style
  {
    struct VerticalAlign;
  }

  /// @brief A rectangle that occurs on a line. It corresponds to some RenderObject (i.e., it represents a
  /// portion of that RenderObject).
  class LegacyInlineBox
  {
    KRYS_NON_COPY_MOVABLE(LegacyInlineBox);

  private:
    LegacyInlineBox *_nextOnLine {nullptr};     // The next element on the same line as us.
    LegacyInlineBox *_previousOnLine {nullptr}; // The previous element on the same line as us.

    LegacyInlineFlowBox *_parent {nullptr}; // The box that contains us.

    WeakPtr<RenderObject> _renderer;

    float _logicalWidth {0};
    FloatPoint _topLeft;

    // Some of these bits are actually for subclasses and moved here to compact the structures.
    // for this class
    bool _firstLine : 1 {false};
    bool _constructed : 1 {false};
    uint32 _bidiEmbeddingLevel : 6 {0}; // The maximium bidi level is 62:
                                        // http://unicode.org/reports/tr9/#Explicit_Levels_and_Directions
    bool _dirty : 1 {false};
    bool _extracted : 1 {false};
    bool _hasVirtualLogicalHeight : 1 {false};
    bool _isHorizontal : 1 {true};
    // for RootInlineBox
    bool _endsWithBreak : 1 {false}; // Whether the line ends with a <br>.
    // shared between RootInlineBox and LegacyInlineTextBox
    bool _knownToHaveNoOverflow : 1 {true};
    mutable bool _determinedIfNextOnLineExists : 1 {false};
    mutable bool _nextOnLineExists : 1 {false};

  public:
    virtual ~LegacyInlineBox() noexcept = default;

    virtual void DeleteLine() noexcept = 0;

    KRYS_NODISCARD virtual bool IsLineBreak() const noexcept
    {
      return Renderer().IsRenderLineBreak();
    }

    virtual void AdjustPosition(float dx, float dy) noexcept;
    void AdjustLogicalPosition(float deltaLogicalLeft, float deltaLogicalTop) noexcept
    {
      if (IsHorizontal())
      {
        AdjustPosition(deltaLogicalLeft, deltaLogicalTop);
      }
      else
      {
        AdjustPosition(deltaLogicalTop, deltaLogicalLeft);
      }
    }
    void AdjustLineDirectionPosition(float delta) noexcept
    {
      if (IsHorizontal())
      {
        AdjustPosition(delta, 0);
      }
      else
      {
        AdjustPosition(0, delta);
      }
    }
    void AdjustBlockDirectionPosition(float delta) noexcept
    {
      if (IsHorizontal())
      {
        AdjustPosition(0, delta);
      }
      else
      {
        AdjustPosition(delta, 0);
      }
    }

    KRYS_NODISCARD virtual bool IsInlineFlowBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsInlineTextBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsRootInlineBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSVGInlineTextBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSVGInlineFlowBox() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsSVGRootInlineBox() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool HasVirtualLogicalHeight() const noexcept
    {
      return _hasVirtualLogicalHeight;
    }
    void SetHasVirtualLogicalHeight() noexcept
    {
      _hasVirtualLogicalHeight = true;
    }
    KRYS_NODISCARD virtual float VirtualLogicalHeight() const noexcept
    {
      krys_debug_assert(false);
      return 0;
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept
    {
      return _isHorizontal;
    }
    void SetIsHorizontal(bool isHorizontal) noexcept
    {
      _isHorizontal = isHorizontal;
    }

    KRYS_NODISCARD bool IsConstructed() const noexcept
    {
      return _constructed;
    }
    virtual void SetConstructed() noexcept
    {
      _constructed = true;
    }

    void SetExtracted(bool extracted = true) noexcept
    {
      _extracted = extracted;
    }

    void SetIsFirstLine(bool firstLine) noexcept
    {
      _firstLine = firstLine;
    }
    KRYS_NODISCARD bool IsFirstLine() const noexcept
    {
      return _firstLine;
    }

    void RemoveFromParent() noexcept;

    KRYS_NODISCARD LegacyInlineBox *NextOnLine() const noexcept
    {
      return _nextOnLine;
    }
    KRYS_NODISCARD LegacyInlineBox *PreviousOnLine() const noexcept
    {
      return _previousOnLine;
    }
    void SetNextOnLine(LegacyInlineBox *next) noexcept
    {
      krys_debug_assert(_parent || !next);
      _nextOnLine = next;
    }
    void SetPreviousOnLine(LegacyInlineBox *previous) noexcept
    {
      krys_debug_assert(_parent || !previous);
      _previousOnLine = previous;
    }
    KRYS_NODISCARD bool NextOnLineExists() const noexcept;
    KRYS_NODISCARD bool PreviousOnLineExists() const noexcept;

    KRYS_NODISCARD virtual bool IsLeaf() const noexcept
    {
      return true;
    }

    KRYS_NODISCARD LegacyInlineBox *NextLeafOnLine() const noexcept;
    KRYS_NODISCARD LegacyInlineBox *PreviousLeafOnLine() const noexcept;

    // FIXME: Hide this once all callers are using tighter types.
    KRYS_NODISCARD RenderObject &Renderer() const noexcept
    {
      return *_renderer;
    }

    KRYS_NODISCARD LegacyInlineFlowBox *Parent() const noexcept
    {
      return _parent;
    }
    void SetParent(LegacyInlineFlowBox *parent) noexcept
    {
      _parent = parent;
    }

    KRYS_NODISCARD const LegacyRootInlineBox &Root() const noexcept;
    KRYS_NODISCARD LegacyRootInlineBox &Root() noexcept;

    /// @brief The left side of the box in the containing block's coordinate system.
    void SetX(float x) noexcept
    {
      _topLeft.SetX(x);
    }
    KRYS_NODISCARD float X() const noexcept
    {
      return _topLeft.X();
    }
    KRYS_NODISCARD float Left() const noexcept
    {
      return _topLeft.X();
    }

    /// @brief The top side of the box in the containing block's coordinate system.
    void SetY(float y) noexcept
    {
      _topLeft.SetY(y);
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _topLeft.Y();
    }
    KRYS_NODISCARD float Top() const noexcept
    {
      return _topLeft.Y();
    }

    KRYS_NODISCARD const FloatPoint &TopLeft() const noexcept
    {
      return _topLeft;
    }

    KRYS_NODISCARD float Width() const noexcept
    {
      return IsHorizontal() ? LogicalWidth() : LogicalHeight();
    }
    KRYS_NODISCARD float Height() const noexcept
    {
      return IsHorizontal() ? LogicalHeight() : LogicalWidth();
    }
    KRYS_NODISCARD FloatSize Size() const noexcept
    {
      return FloatSize(Width(), Height());
    }
    KRYS_NODISCARD float Right() const noexcept
    {
      return Left() + Width();
    }
    KRYS_NODISCARD float Bottom() const noexcept
    {
      return Top() + Height();
    }

    /// @brief The left edge of the line box in a horizontal line and the top edge in a vertical line.
    KRYS_NODISCARD float LogicalLeft() const noexcept
    {
      return IsHorizontal() ? _topLeft.X() : _topLeft.Y();
    }
    KRYS_NODISCARD float LogicalRight() const noexcept
    {
      return LogicalLeft() + LogicalWidth();
    }
    void SetLogicalLeft(float left) noexcept
    {
      if (IsHorizontal())
      {
        SetX(left);
      }
      else
      {
        SetY(left);
      }
    }

    /// @brief The top edge of the line box in a horizontal line and the left edge in a vertical line.
    KRYS_NODISCARD float LogicalTop() const noexcept
    {
      return IsHorizontal() ? _topLeft.Y() : _topLeft.X();
    }
    KRYS_NODISCARD float LogicalBottom() const noexcept
    {
      return LogicalTop() + LogicalHeight();
    }
    void SetLogicalTop(float top) noexcept
    {
      if (IsHorizontal())
      {
        SetY(top);
      }
      else
      {
        SetX(top);
      }
    }

    /// @brief Our extent in the line's overall inline direction, i.e., width for horizontal text and height
    /// for vertical text.
    void SetLogicalWidth(float w) noexcept
    {
      _logicalWidth = w;
    }
    KRYS_NODISCARD float LogicalWidth() const noexcept
    {
      return _logicalWidth;
    }

    /// @brief Our extent in the block flow direction, i.e., height for horizontal text and width for vertical
    /// text.
    KRYS_NODISCARD float LogicalHeight() const noexcept;

    KRYS_NODISCARD FloatRect LogicalFrameRect() const noexcept
    {
      return IsHorizontal() ? FloatRect(_topLeft.X(), _topLeft.Y(), _logicalWidth, LogicalHeight())
                            : FloatRect(_topLeft.Y(), _topLeft.X(), _logicalWidth, LogicalHeight());
    }
    KRYS_NODISCARD FloatRect FrameRect() const noexcept
    {
      return FloatRect(TopLeft(), Size());
    }

    KRYS_NODISCARD virtual int32 CaretMinOffset() const noexcept;
    KRYS_NODISCARD virtual int32 CaretMaxOffset() const noexcept;

    KRYS_NODISCARD uint8 BidiLevel() const noexcept
    {
      return static_cast<uint8>(_bidiEmbeddingLevel);
    }
    void SetBidiLevel(uint8 level) noexcept
    {
      _bidiEmbeddingLevel = level;
    }
    KRYS_NODISCARD TextDirection Direction() const noexcept
    {
      return BidiLevel() % 2 ? TextDirection::RTL : TextDirection::LTR;
    }
    KRYS_NODISCARD bool IsLeftToRightDirection() const noexcept
    {
      return Direction() == TextDirection::LTR;
    }
    KRYS_NODISCARD int32 CaretLeftmostOffset() const noexcept
    {
      return IsLeftToRightDirection() ? CaretMinOffset() : CaretMaxOffset();
    }
    KRYS_NODISCARD int32 CaretRightmostOffset() const noexcept
    {
      return IsLeftToRightDirection() ? CaretMaxOffset() : CaretMinOffset();
    }

    KRYS_NODISCARD bool IsDirty() const noexcept
    {
      return _dirty;
    }
    virtual void MarkDirty(bool dirty = true) noexcept
    {
      _dirty = dirty;
    }

    virtual void DirtyLineBoxes() noexcept;

    KRYS_NODISCARD virtual RenderObject::HighlightState SelectionState() const noexcept;

    KRYS_NODISCARD const RenderStyle &LineStyle() const noexcept;

    KRYS_NODISCARD const style::VerticalAlign &GetVerticalAlign() const noexcept;

    /// @brief Use with caution! The type is not checked!
    KRYS_NODISCARD RenderBoxModelObject *BoxModelObject() const noexcept;

    KRYS_NODISCARD FloatPoint LocationIncludingFlipping() const noexcept;
    void FlipForWritingMode(FloatRect &) const noexcept;
    KRYS_NODISCARD FloatPoint FlipForWritingMode(const FloatPoint &) const noexcept;
    void FlipForWritingMode(LayoutRect &) const noexcept;
    KRYS_NODISCARD LayoutPoint FlipForWritingMode(const LayoutPoint &) const noexcept;

    KRYS_NODISCARD bool KnownToHaveNoOverflow() const noexcept
    {
      return _knownToHaveNoOverflow;
    }
    void ClearKnownToHaveNoOverflow() noexcept;

  protected:
    explicit LegacyInlineBox(RenderObject &renderer) noexcept;

    // For RootInlineBox
    KRYS_NODISCARD bool EndsWithBreak() const noexcept
    {
      return _endsWithBreak;
    }
    void SetEndsWithBreak(bool endsWithBreak) noexcept
    {
      _endsWithBreak = endsWithBreak;
    }

    // For LegacyInlineFlowBox and LegacyInlineTextBox
    KRYS_NODISCARD bool Extracted() const noexcept
    {
      return _extracted;
    }
  };
}
