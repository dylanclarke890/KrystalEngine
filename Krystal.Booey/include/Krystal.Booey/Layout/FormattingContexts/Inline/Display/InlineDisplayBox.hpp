#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineRect.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <unicode/ubidi.h>

namespace krys::boo::inline_display
{
  /// @brief A box of laid out inline content on a line: a piece of text, a line break, an inline level box
  /// or a block in inline content, at its visual position (ignoring the block direction's flip).
  struct Box
  {
    /// @brief The text of a text or soft line break box: a byte range of its inline text box's content, and
    /// the content to render when it differs (a hyphen or a truncation).
    struct Text
    {
    public:
      enum class ShapingBoundary : uint8
      {
        Start,
        Inside,
        End,
        NotApplicable
      };

    private:
      friend struct Box;

      const utf8_string *_originalContent {nullptr};
      Maybe<utf8_string> _adjustedContentToRender;
      uint32 _start {0};
      uint32 _length {0};
      ShapingBoundary _shapingBoundary {ShapingBoundary::NotApplicable};
      uint32 _partiallyVisibleContentLength : 30 {};
      bool _hasPartiallyVisibleContentLength : 1 {false};
      bool _hasHyphen : 1 {false};

    public:
      Text() noexcept = default;
      /// @param originalContent The inline text box's content, which must outlive the box.
      Text(size_t start, size_t length, const utf8_string &originalContent,
           Maybe<utf8_string> adjustedContentToRender = null, bool hasHyphen = false,
           ShapingBoundary shapingBoundary = ShapingBoundary::NotApplicable) noexcept
          : _originalContent(&originalContent), _adjustedContentToRender(krys::move(adjustedContentToRender)),
            _start(static_cast<uint32>(start)), _length(static_cast<uint32>(length)),
            _shapingBoundary(shapingBoundary), _hasHyphen(hasHyphen)
      {
      }

      KRYS_NODISCARD size_t Start() const noexcept
      {
        return _start;
      }
      KRYS_NODISCARD size_t End() const noexcept
      {
        return Start() + Length();
      }
      KRYS_NODISCARD size_t Length() const noexcept
      {
        return _length;
      }

      KRYS_NODISCARD utf8_stringview OriginalContent() const noexcept
      {
        if (!_originalContent)
        {
          return {};
        }
        auto originalContent = utf8_stringview {*_originalContent};
        return originalContent.substr(std::min<size_t>(_start, originalContent.size()), _length);
      }
      KRYS_NODISCARD utf8_stringview RenderedContent() const noexcept
      {
        return _adjustedContentToRender ? utf8_stringview {*_adjustedContentToRender} : OriginalContent();
      }

      KRYS_NODISCARD bool IsAtShapingBoundaryStart() const noexcept
      {
        return _shapingBoundary == ShapingBoundary::Start;
      }
      KRYS_NODISCARD bool IsAtShapingBoundaryEnd() const noexcept
      {
        return _shapingBoundary == ShapingBoundary::End;
      }
      KRYS_NODISCARD bool IsInsideShapingBoundary() const noexcept
      {
        return _shapingBoundary == ShapingBoundary::Inside;
      }

      KRYS_NODISCARD bool HasHyphen() const noexcept
      {
        return _hasHyphen;
      }

      KRYS_NODISCARD Maybe<size_t> PartiallyVisibleContentLength() const noexcept
      {
        if (!_hasPartiallyVisibleContentLength)
        {
          return {};
        }
        return _partiallyVisibleContentLength;
      }
      void SetPartiallyVisibleContentLength(size_t truncatedLength) noexcept
      {
        _partiallyVisibleContentLength = static_cast<uint32>(truncatedLength);
        _hasPartiallyVisibleContentLength = true;
      }

      KRYS_NODISCARD friend bool operator==(const Text &a, const Text &b) noexcept
      {
        return a.Start() == b.Start() && a.Length() == b.Length()
               && a.RenderedContent() == b.RenderedContent();
      }
    };

    enum class Type : uint8
    {
      Text,
      WordSeparator,
      Ellipsis,
      SoftLineBreak,
      LineBreakBox,
      AtomicInlineBox,
      NonRootInlineBox,
      RootInlineBox,
      GenericInlineLevelBox,
      BlockLevelBox,
    };

    struct Expansion
    {
      ExpansionBehavior Behavior {ExpansionBehavior::DefaultBehavior()};
      float HorizontalExpansion {0};
    };

    enum class PositionWithinInlineLevelBox : bool
    {
      First,
      Last
    };

  private:
    const layout::Box *_layoutBox;
    FloatRect _unflippedVisualRect;
    FloatRect _inkOverflow;
    uint32 _lineIndex {0};
    float _horizontalExpansion {0};

    ExpansionBehavior _expansionBehavior {ExpansionBehavior::DefaultBehavior()};
    UBiDiLevel _bidiLevel {UBIDI_DEFAULT_LTR};
    Type _type : 4 {Type::GenericInlineLevelBox};
    bool _hasContent : 1 {false};
    bool _isFirstForLayoutBox : 1 {false};
    bool _isLastForLayoutBox : 1 {false};
    bool _isFullyTruncated : 1 {false};
    bool _isFirstFormattedLine : 1 {false};
    Text _text;

  public:
    Box(size_t lineIndex, Type type, const layout::Box &layoutBox, UBiDiLevel bidiLevel,
        const FloatRect &physicalRect, const FloatRect &inkOverflow, bool isFirstFormattedLine,
        Expansion expansion, Maybe<Text> text = null, bool hasContent = true, bool isFullyTruncated = false,
        EnumSet<PositionWithinInlineLevelBox> positionWithinInlineLevelBox = {}) noexcept
        : _layoutBox(&layoutBox), _unflippedVisualRect(physicalRect), _inkOverflow(inkOverflow),
          _lineIndex(static_cast<uint32>(lineIndex)), _horizontalExpansion(expansion.HorizontalExpansion),
          _expansionBehavior(expansion.Behavior), _bidiLevel(bidiLevel), _type(type), _hasContent(hasContent),
          _isFirstForLayoutBox(positionWithinInlineLevelBox.Contains(PositionWithinInlineLevelBox::First)),
          _isLastForLayoutBox(positionWithinInlineLevelBox.Contains(PositionWithinInlineLevelBox::Last)),
          _isFullyTruncated(isFullyTruncated), _isFirstFormattedLine(isFirstFormattedLine),
          _text(text ? krys::move(*text) : Text {})
    {
    }

    KRYS_NODISCARD bool IsText() const noexcept
    {
      return _type == Type::Text || IsWordSeparator();
    }
    KRYS_NODISCARD bool IsWordSeparator() const noexcept
    {
      return _type == Type::WordSeparator;
    }
    KRYS_NODISCARD bool IsEllipsis() const noexcept
    {
      return _type == Type::Ellipsis;
    }
    KRYS_NODISCARD bool IsSoftLineBreak() const noexcept
    {
      return _type == Type::SoftLineBreak;
    }
    KRYS_NODISCARD bool IsTextOrSoftLineBreak() const noexcept
    {
      return IsText() || IsSoftLineBreak();
    }
    KRYS_NODISCARD bool IsLineBreakBox() const noexcept
    {
      return _type == Type::LineBreakBox;
    }
    KRYS_NODISCARD bool IsLineBreak() const noexcept
    {
      return IsSoftLineBreak() || IsLineBreakBox();
    }
    KRYS_NODISCARD bool IsAtomicInlineBox() const noexcept
    {
      return _type == Type::AtomicInlineBox;
    }
    KRYS_NODISCARD bool IsInlineBox() const noexcept
    {
      return IsNonRootInlineBox() || IsRootInlineBox();
    }
    KRYS_NODISCARD bool IsNonRootInlineBox() const noexcept
    {
      return _type == Type::NonRootInlineBox;
    }
    KRYS_NODISCARD bool IsRootInlineBox() const noexcept
    {
      return _type == Type::RootInlineBox;
    }
    KRYS_NODISCARD bool IsGenericInlineLevelBox() const noexcept
    {
      return _type == Type::GenericInlineLevelBox;
    }
    KRYS_NODISCARD bool IsInlineLevelBox() const noexcept
    {
      return IsAtomicInlineBox() || IsLineBreakBox() || IsInlineBox() || IsGenericInlineLevelBox();
    }
    KRYS_NODISCARD bool IsBlockLevelBox() const noexcept
    {
      return _type == Type::BlockLevelBox;
    }
    KRYS_NODISCARD bool IsNonRootInlineLevelBox() const noexcept
    {
      return IsInlineLevelBox() && !IsRootInlineBox();
    }

    KRYS_NODISCARD UBiDiLevel BidiLevel() const noexcept
    {
      return _bidiLevel;
    }

    KRYS_NODISCARD bool IsHorizontal() const noexcept;

    KRYS_NODISCARD bool HasContent() const noexcept
    {
      return _hasContent;
    }
    KRYS_NODISCARD bool IsVisible() const noexcept;
    // Inline boxes around blocks are visible to hit testing but don't paint.
    KRYS_NODISCARD bool IsVisibleIgnoringUsedVisibility() const noexcept;
    KRYS_NODISCARD bool IsFullyTruncated() const noexcept
    {
      return _isFullyTruncated;
    }

    KRYS_NODISCARD const FloatRect &VisualRectIgnoringBlockDirection() const noexcept
    {
      return _unflippedVisualRect;
    }
    KRYS_NODISCARD static FloatRect
      VisibleRectIgnoringBlockDirection(const Box &box, const FloatRect &visibleLineRect) noexcept
    {
      auto visualRectIgnoringBlockDirection = box.VisualRectIgnoringBlockDirection();
      auto visibleBoxLeft = std::max(visualRectIgnoringBlockDirection.X(), visibleLineRect.X());
      visualRectIgnoringBlockDirection.SetX(visibleBoxLeft);
      auto visibleBoxRight = std::min(visualRectIgnoringBlockDirection.MaxX(), visibleLineRect.MaxX());
      visualRectIgnoringBlockDirection.ShiftMaxXEdgeTo(visibleBoxRight);
      return visualRectIgnoringBlockDirection;
    }
    KRYS_NODISCARD const FloatRect &InkOverflow() const noexcept
    {
      return _inkOverflow;
    }

    KRYS_NODISCARD float Top() const noexcept
    {
      return VisualRectIgnoringBlockDirection().Y();
    }
    KRYS_NODISCARD float Bottom() const noexcept
    {
      return VisualRectIgnoringBlockDirection().MaxY();
    }
    KRYS_NODISCARD float Left() const noexcept
    {
      return VisualRectIgnoringBlockDirection().X();
    }
    KRYS_NODISCARD float Right() const noexcept
    {
      return VisualRectIgnoringBlockDirection().MaxX();
    }
    KRYS_NODISCARD float Width() const noexcept
    {
      return VisualRectIgnoringBlockDirection().Width();
    }
    KRYS_NODISCARD float Height() const noexcept
    {
      return VisualRectIgnoringBlockDirection().Height();
    }

    void MoveVertically(float offset) noexcept
    {
      _unflippedVisualRect.Move({{}, offset});
      _inkOverflow.Move({{}, offset});
    }
    void MoveHorizontally(float offset) noexcept
    {
      _unflippedVisualRect.Move({offset, {}});
      _inkOverflow.Move({offset, {}});
    }
    void ExpandVertically(float delta) noexcept
    {
      _unflippedVisualRect.Expand({{}, delta});
      _inkOverflow.Expand({{}, delta});
    }
    void ExpandHorizontally(float delta) noexcept
    {
      _unflippedVisualRect.Expand({delta, {}});
      _inkOverflow.Expand({delta, {}});
    }
    void AdjustInkOverflow(const FloatRect &childBorderBox) noexcept
    {
      _inkOverflow.UniteEvenIfEmpty(childBorderBox);
    }

    void SetLeft(float physicalLeft) noexcept
    {
      auto offset = physicalLeft - Left();
      _unflippedVisualRect.SetX(physicalLeft);
      _inkOverflow.SetX(_inkOverflow.X() + offset);
    }
    void SetRight(float physicalRight) noexcept
    {
      auto offset = physicalRight - Right();
      _unflippedVisualRect.ShiftMaxXEdgeTo(physicalRight);
      _inkOverflow.ShiftMaxXEdgeTo(_inkOverflow.MaxX() + offset);
    }
    void SetTop(float physicalTop) noexcept
    {
      auto offset = physicalTop - Top();
      _unflippedVisualRect.SetY(physicalTop);
      _inkOverflow.SetY(_inkOverflow.Y() + offset);
    }
    void SetBottom(float physicalBottom) noexcept
    {
      auto offset = physicalBottom - Bottom();
      _unflippedVisualRect.ShiftMaxYEdgeTo(physicalBottom);
      _inkOverflow.ShiftMaxYEdgeTo(_inkOverflow.MaxY() + offset);
    }
    void SetRect(const FloatRect &rect, const FloatRect &inkOverflow) noexcept
    {
      _unflippedVisualRect = rect;
      _inkOverflow = inkOverflow;
    }

    void SetHasContent() noexcept
    {
      _hasContent = true;
    }
    void SetIsFullyTruncated() noexcept
    {
      _isFullyTruncated = true;
    }

    KRYS_NODISCARD Text &GetText() noexcept
    {
      krys_debug_assert(IsTextOrSoftLineBreak());
      return _text;
    }
    KRYS_NODISCARD const Text &GetText() const noexcept
    {
      krys_debug_assert(IsTextOrSoftLineBreak());
      return _text;
    }

    void SetExpansion(Expansion expansion) noexcept
    {
      _horizontalExpansion = expansion.HorizontalExpansion;
      _expansionBehavior = expansion.Behavior;
    }
    KRYS_NODISCARD Expansion GetExpansion() const noexcept
    {
      return {_expansionBehavior, _horizontalExpansion};
    }

    KRYS_NODISCARD const layout::Box &LayoutBox() const noexcept
    {
      return *_layoutBox;
    }
    KRYS_NODISCARD const RenderStyle &Style() const noexcept
    {
      return IsFirstFormattedLine() ? LayoutBox().FirstLineStyle() : LayoutBox().Style();
    }
    KRYS_NODISCARD boo::WritingMode GetWritingMode() const noexcept;

    void MoveToLine(uint32 lineIndex) noexcept
    {
      _lineIndex = lineIndex;
    }
    KRYS_NODISCARD size_t LineIndex() const noexcept
    {
      return _lineIndex;
    }
    KRYS_NODISCARD bool IsFirstFormattedLine() const noexcept
    {
      return _isFirstFormattedLine;
    }

    // These functions tell you whether this display box is the first/last for the associated inline level box
    // (Layout::Box) and not whether it's the first/last box on the line. (e.g. always true for atomic boxes,
    // but inline boxes spanning over multiple lines can produce individual first/last boxes).
    KRYS_NODISCARD bool IsFirstForLayoutBox() const noexcept
    {
      return _isFirstForLayoutBox;
    }
    KRYS_NODISCARD bool IsLastForLayoutBox() const noexcept
    {
      return _isLastForLayoutBox;
    }
    void SetIsFirstForLayoutBox() noexcept
    {
      _isFirstForLayoutBox = true;
    }
    void SetIsLastForLayoutBox() noexcept
    {
      _isLastForLayoutBox = true;
    }

    KRYS_NODISCARD friend bool operator==(const Box &a, const Box &b) noexcept;
  };
}
