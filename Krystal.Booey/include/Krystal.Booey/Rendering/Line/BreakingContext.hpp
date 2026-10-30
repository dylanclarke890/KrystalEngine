#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Rendering/BreakablePositions.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"
#include "Krystal.Booey/Rendering/Line/LineBreaker.hpp"
#include "Krystal.Booey/Rendering/Line/LineInfo.hpp"
#include "Krystal.Booey/Rendering/Line/LineInlineHeaders.hpp"
#include "Krystal.Booey/Rendering/Line/LineWidth.hpp"
#include "Krystal.Booey/Rendering/Line/TrailingObjects.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include <unicode/uchar.h>

namespace krys::boo
{
  /// @brief We don't let our line box tree for a single line get any deeper than this.
  constexpr uint32 MaxLineDepth = 200;

  /// @brief Finds where the legacy line layout breaks a line.
  class BreakingContext
  {
  private:
    LineBreaker &_lineBreaker;
    InlineBidiResolver &_resolver;

    LegacyInlineIterator _current;
    LegacyInlineIterator _lineBreak;
    LegacyInlineIterator _startOfIgnoredSpaces;

    RenderBlockFlow &_block;
    RenderObject *_lastObject;
    RenderObject *_nextObject {nullptr};

    // Firefox and Opera will allow a table cell to grow to fit an image inside it under
    // very specific circumstances (in order to match common WinIE renderings).
    // Not supporting the quirk has caused us to mis-render some real sites. (See Bugzilla 10517.)
    const RenderStyle &_blockStyle;

    LineInfo &_lineInfo;

    RenderTextInfo &_renderTextInfo;

    LineWidth _width;

    TextWrapMode _currentTextWrap {TextWrapMode::Wrap};
    TextWrapMode _lastObjectTextWrap {TextWrapMode::Wrap};

    WhiteSpaceCollapse _currentWhitespaceCollapse {WhiteSpaceCollapse::Collapse};
    WhiteSpaceCollapse _lastObjectWhitespaceCollapse {WhiteSpaceCollapse::Collapse};

    bool _preservesNewline {false};
    bool _atStart {true};

    // This variable is used only if whitespace isn't set to WhiteSpace::Pre, and it tells us whether
    // or not we are currently ignoring whitespace.
    bool _ignoringSpaces {false};

    // This variable tracks whether the very last character we saw was a space. We use
    // this to detect when we encounter a second space so we know we have to terminate
    // a run.
    bool _currentCharacterIsSpace {false};
    bool _currentCharacterIsWS {false};
    bool _hasFormerOpportunity {false};
    bool _appliedStartWidth;
    bool _includeEndWidth {true};
    bool _autoWrap {false};
    bool _autoWrapWasEverTrueOnLine {false};
    bool _collapseWhiteSpace {false};
    bool _allowImagesToBreak;
    bool _atEnd {false};
    bool _hadUncommittedWidthBeforeCurrent {false};

    bool _hangsAtEnd {false};

    LineWhitespaceCollapsingState &_lineWhitespaceCollapsingState;

    TrailingObjects _trailingObjects;

  public:
    BreakingContext(LineBreaker &lineBreaker, InlineBidiResolver &resolver, LineInfo &inLineInfo,
                    LineWidth &lineWidth, RenderTextInfo &inRenderTextInfo, bool appliedStartWidth,
                    RenderBlockFlow &block) noexcept;

    KRYS_NODISCARD RenderObject *CurrentObject() const noexcept
    {
      return _current.Renderer();
    }
    KRYS_NODISCARD LegacyInlineIterator LineBreak() const noexcept
    {
      return _lineBreak;
    }
    KRYS_NODISCARD LineWidth &GetLineWidth() noexcept
    {
      return _width;
    }
    KRYS_NODISCARD bool AtEnd() const noexcept
    {
      return _atEnd;
    }

    KRYS_NODISCARD bool FitsOnLineOrHangsAtEnd() const noexcept
    {
      return _width.FitsOnLine() || _hangsAtEnd;
    }

    void InitializeForCurrentObject() noexcept;

    void Increment() noexcept;

    void HandleEmptyInline() noexcept;
    bool HandleText() noexcept;
    void TrailingSpacesHang(LegacyInlineIterator &, RenderObject &, bool canBreakMidWord,
                            bool previousCharacterIsSpace) noexcept;
    KRYS_NODISCARD bool CanBreakAtThisPosition() const noexcept
    {
      return false;
    }
    void CommitAndUpdateLineBreakIfNeeded() noexcept;
    KRYS_NODISCARD LegacyInlineIterator HandleEndOfLine() noexcept;

    void ClearLineBreakIfFitsOnLine(bool ignoringTrailingSpace = false) noexcept
    {
      if (_width.FitsOnLine(ignoringTrailingSpace)
          || (_lastObjectTextWrap == TextWrapMode::NoWrap
              && _lastObjectWhitespaceCollapse == WhiteSpaceCollapse::Collapse)
          || _hangsAtEnd)
      {
        _lineBreak.Clear();
      }
      _hangsAtEnd = false;
    }

    void CommitLineBreakClear() noexcept
    {
      _width.Commit();
      _lineBreak.Clear();
      _hangsAtEnd = false;
    }

    void CommitLineBreakAtCurrentWidth(RenderObject &object, uint32 offset = 0,
                                       Maybe<uint32> nextBreak = null) noexcept
    {
      _width.Commit();
      _lineBreak.MoveTo(object, offset, nextBreak);
      _hangsAtEnd = false;
    }
  };

  KRYS_NODISCARD inline LayoutUnit BorderPaddingMarginStart(const RenderInline &child) noexcept
  {
    return child.MarginStart() + child.PaddingStart() + child.BorderStart();
  }

  KRYS_NODISCARD inline LayoutUnit BorderPaddingMarginEnd(const RenderInline &child) noexcept
  {
    return child.MarginEnd() + child.PaddingEnd() + child.BorderEnd();
  }

  KRYS_NODISCARD LayoutUnit InlineLogicalWidth(const RenderObject &renderer, bool checkStartEdge = true,
                                               bool checkEndEdge = true) noexcept;
}
