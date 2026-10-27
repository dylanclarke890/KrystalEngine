#include "Krystal.Booey/Rendering/Line/BreakingContext.hpp"

#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"

namespace krys::boo
{
  BreakingContext::BreakingContext(LineBreaker &lineBreaker, InlineBidiResolver &resolver,
                                   LineInfo &inLineInfo, LineWidth &lineWidth,
                                   RenderTextInfo &inRenderTextInfo, bool appliedStartWidth,
                                   RenderBlockFlow &block) noexcept
      : _lineBreaker(lineBreaker), _resolver(resolver), _current(resolver.Position()),
        _lineBreak(resolver.Position()), _block(block), _lastObject(_current.Renderer()),
        _blockStyle(block.Style()), _lineInfo(inLineInfo), _renderTextInfo(inRenderTextInfo),
        _width(lineWidth), _appliedStartWidth(appliedStartWidth),
        _allowImagesToBreak(!block.GetDocument().InQuirksMode() || !block.IsRenderTableCell()
                            || !_blockStyle.LogicalWidth().IsIntrinsicOrLegacyIntrinsicOrAuto()),
        _lineWhitespaceCollapsingState(resolver.GetWhitespaceCollapsingState())
  {
  }

  void BreakingContext::InitializeForCurrentObject() noexcept
  {
    _hadUncommittedWidthBeforeCurrent = !!_width.UncommittedWidth();
    auto &renderer = *_current.Renderer();

    _nextObject = NextInlineRendererSkippingEmpty(_block, &renderer);
    if (_nextObject && _nextObject->Parent() && !_nextObject->Parent()->IsDescendantOf(renderer.Parent()))
    {
      _includeEndWidth = true;
    }

    _currentTextWrap = renderer.IsBlockLevelReplacedOrAtomicInline() || Is<RenderText>(renderer)
                         ? renderer.Parent()->Style().GetTextWrapMode()
                         : renderer.Style().GetTextWrapMode();
    _currentWhitespaceCollapse = renderer.IsBlockLevelReplacedOrAtomicInline() || Is<RenderText>(renderer)
                                   ? renderer.Parent()->Style().GetWhiteSpaceCollapse()
                                   : renderer.Style().GetWhiteSpaceCollapse();

    _lastObjectTextWrap = _lastObject->IsBlockLevelReplacedOrAtomicInline() || Is<RenderText>(renderer)
                            ? _lastObject->Parent()->Style().GetTextWrapMode()
                            : _lastObject->Style().GetTextWrapMode();
    _lastObjectWhitespaceCollapse =
      _lastObject->IsBlockLevelReplacedOrAtomicInline() || Is<RenderText>(renderer)
        ? _lastObject->Parent()->Style().GetWhiteSpaceCollapse()
        : _lastObject->Style().GetWhiteSpaceCollapse();

    bool isSVGText = renderer.IsRenderSVGInlineText();
    _autoWrap = !isSVGText && _currentTextWrap != TextWrapMode::NoWrap;
    _autoWrapWasEverTrueOnLine = _autoWrapWasEverTrueOnLine || _autoWrap;

    _preservesNewline = !isSVGText && _currentWhitespaceCollapse != WhiteSpaceCollapse::Collapse;

    _collapseWhiteSpace = _currentWhitespaceCollapse == WhiteSpaceCollapse::Collapse
                          || _currentWhitespaceCollapse == WhiteSpaceCollapse::PreserveBreaks;
  }

  void BreakingContext::Increment() noexcept
  {
    // Clear out our character space bool, since inline <pre>s don't collapse whitespace
    // with adjacent inline normal/nowrap spans.
    if (!_collapseWhiteSpace)
    {
      _currentCharacterIsSpace = false;
    }

    if (_nextObject)
    {
      _current.MoveToStartOf(*_nextObject);
    }
    else
    {
      _current.Clear();
    }
    _atStart = false;
  }

  LayoutUnit InlineLogicalWidth(const RenderObject &renderer, bool checkStartEdge, bool checkEndEdge) noexcept
  {
    auto previousInFlowSibling = [](const RenderObject &renderer)
    {
      auto *previousSibling = renderer.PreviousSibling();
      for (; previousSibling && previousSibling->IsOutOfFlowPositioned();
           previousSibling = previousSibling->PreviousSibling())
      {
      }
      return previousSibling;
    };

    auto shouldAddBorderPaddingMargin = [](const RenderObject &renderer)
    {
      // When deciding whether we're at the edge of an inline, adjacent collapsed whitespace is the same as no
      // sibling at all.
      auto *textRenderer = DynamicDowncast<RenderText>(renderer);
      if (textRenderer && textRenderer->Text().empty())
      {
        return true;
      }
      if (Is<RenderLineBreak>(renderer)
          && renderer.Parent()->Style().GetBoxDecorationBreak() == BoxDecorationBreak::Clone)
      {
        return true;
      }
      return false;
    };

    uint32 lineDepth = 1;
    auto extraWidth = LayoutUnit {};
    auto *parent = DynamicDowncast<RenderInline>(renderer.Parent());
    auto *child = &renderer;
    while (parent && lineDepth++ < MaxLineDepth)
    {
      if (!IsEmptyInline(*parent))
      {
        auto *previousSibling = previousInFlowSibling(*child);
        checkStartEdge =
          checkStartEdge && (!previousSibling || shouldAddBorderPaddingMargin(*previousSibling));
        if (checkStartEdge)
        {
          extraWidth += BorderPaddingMarginStart(*parent);
        }
        auto *nextSibling = child->NextSibling();
        checkEndEdge = checkEndEdge && (!nextSibling || shouldAddBorderPaddingMargin(*nextSibling));
        if (checkEndEdge)
        {
          extraWidth += BorderPaddingMarginEnd(*parent);
        }
        if (!checkStartEdge && !checkEndEdge)
        {
          return extraWidth;
        }
      }
      child = parent;
      krys_debug_assert(child->Parent());
      parent = DynamicDowncast<RenderInline>(child->Parent());
    }
    return extraWidth;
  }

  namespace
  {
    // This is currently just used for list markers and inline flows that have line boxes. Neither should
    // have an effect on whitespace at the start of the line.
    KRYS_NODISCARD bool ShouldSkipWhitespaceAfterStartObject(
      RenderBlockFlow &block, RenderObject *renderer,
      LineWhitespaceCollapsingState &lineWhitespaceCollapsingState) noexcept
    {
      if (auto *nextText = DynamicDowncast<RenderText>(NextInlineRendererSkippingEmpty(block, renderer));
          nextText && !nextText->Text().empty())
      {
        char32 nextChar = text::CodePointAt(nextText->Text(), 0);
        if (nextText->Style().IsCollapsibleWhiteSpace(static_cast<char16_t>(nextChar)))
        {
          lineWhitespaceCollapsingState.StartIgnoringSpaces(LegacyInlineIterator(nullptr, renderer, 0));
          return true;
        }
      }

      return false;
    }

    void NextCharacter(char32 &currentCharacter, char32 &lastCharacter,
                       char32 &secondToLastCharacter) noexcept
    {
      secondToLastCharacter = lastCharacter;
      lastCharacter = currentCharacter;
    }

    // Adding a pair of whitespace collapsing transitions before a character will split it out into a new line
    // box.
    void EnsureCharacterGetsLineBox(LineWhitespaceCollapsingState &lineWhitespaceCollapsingState,
                                    LegacyInlineIterator &textParagraphSeparator) noexcept
    {
      lineWhitespaceCollapsingState.StartIgnoringSpaces(LegacyInlineIterator(
        nullptr, textParagraphSeparator.Renderer(), textParagraphSeparator.Offset() - 1));
      lineWhitespaceCollapsingState.StopIgnoringSpaces(
        LegacyInlineIterator(nullptr, textParagraphSeparator.Renderer(), textParagraphSeparator.Offset()));
    }

    KRYS_NODISCARD TrailingObjects::CollapseFirstSpace
      CheckWhitespaceCollapsingTransitions(LineWhitespaceCollapsingState &lineWhitespaceCollapsingState,
                                           const LegacyInlineIterator &lBreak) noexcept
    {
      // Check to see if our last transition is a start point beyond the line break. If so,
      // shave it off the list, and shave off a trailing space if the previous end point doesn't
      // preserve whitespace.
      if (lBreak.Renderer() && lineWhitespaceCollapsingState.NumTransitions()
          && !(lineWhitespaceCollapsingState.NumTransitions() % 2))
      {
        const auto &transitions = lineWhitespaceCollapsingState.Transitions();
        auto endpoint = transitions[lineWhitespaceCollapsingState.NumTransitions() - 2];
        auto startpoint = transitions[lineWhitespaceCollapsingState.NumTransitions() - 1];
        LegacyInlineIterator currpoint = endpoint;
        while (!currpoint.AtEnd() && currpoint != startpoint && currpoint != lBreak)
        {
          currpoint.Increment();
        }
        if (currpoint == lBreak)
        {
          // We hit the line break before the start point. Shave off the start point.
          lineWhitespaceCollapsingState.DecrementNumTransitions();
          if (auto *renderText = DynamicDowncast<RenderText>(*endpoint.Renderer());
              renderText && renderText->Style().CollapseWhiteSpace())
          {
            lineWhitespaceCollapsingState.DecrementTransitionAt(lineWhitespaceCollapsingState.NumTransitions()
                                                                - 1);
            return TrailingObjects::CollapseFirstSpace::No;
          }
        }
      }
      return TrailingObjects::CollapseFirstSpace::Yes;
    }
  }

  void BreakingContext::HandleEmptyInline() noexcept
  {
    RenderInline &flowBox = Downcast<RenderInline>(*_current.Renderer());

    // This should only end up being called on empty inlines
    krys_debug_assert(IsEmptyInline(flowBox));

    // Now that some inline flows have line boxes, if we are already ignoring spaces, we need
    // to make sure that we stop to include this object and then start ignoring spaces again.
    // If this object is at the start of the line, we need to behave like list markers and
    // start ignoring spaces.
    if (RequiresLineBoxForContent(flowBox, _lineInfo))
    {
      // An empty inline that only has line-height, vertical-align or font-metrics will only get a
      // line box to affect the height of the line if the rest of the line is not empty.
      if (_ignoringSpaces)
      {
        _trailingObjects.Clear();
        _lineWhitespaceCollapsingState.EnsureLineBoxInsideIgnoredSpaces(*_current.Renderer());
      }
      else if (_blockStyle.CollapseWhiteSpace() && _resolver.Position().Renderer() == _current.Renderer()
               && ShouldSkipWhitespaceAfterStartObject(_block, _current.Renderer(),
                                                       _lineWhitespaceCollapsingState))
      {
        // Like with list markers, we start ignoring spaces to make sure that any
        // additional spaces we see will be discarded.
        _currentCharacterIsSpace = true;
        _currentCharacterIsWS = true;
        _ignoringSpaces = true;
      }
      else
      {
        _trailingObjects.AppendBoxIfNeeded(flowBox);
      }
    }

    float inlineWidth = (InlineLogicalWidth(*_current.Renderer()) + BorderPaddingMarginStart(flowBox)
                         + BorderPaddingMarginEnd(flowBox))
                          .ToFloat();
    _width.AddUncommittedWidth(inlineWidth);
    if (_hangsAtEnd && inlineWidth)
    {
      _hangsAtEnd = false;
    }
  }

  bool BreakingContext::HandleText() noexcept
  {
    auto &renderer = Downcast<RenderText>(*_current.Renderer());
    auto *svgInlineTextRenderer = DynamicDowncast<RenderSVGInlineText>(renderer);
    const auto &text = renderer.Text();

    // If we have left a no-wrap inline and entered an autowrap inline while ignoring spaces
    // then we need to mark the start of the autowrap inline as a potential linebreak now.
    if (_autoWrap && _lastObjectTextWrap == TextWrapMode::NoWrap && _ignoringSpaces)
    {
      CommitLineBreakAtCurrentWidth(renderer);
    }

    const RenderStyle &style = LineStyle(renderer, _lineInfo);
    const FontCascade &font = style.GetFontCascade();
    bool canHangPunctuationAtStart =
      style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::First);
    bool canHangPunctuationAtEnd =
      style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::Last);
    bool canHangStopOrCommaAtLineEnd =
      style.GetHangingPunctuation().Contains(style::HangingPunctuationValue::AllowEnd);
    auto endPunctuationIndex = canHangPunctuationAtEnd && _collapseWhiteSpace
                                 ? static_cast<int64>(renderer.LastCharacterIndexStrippingSpaces())
                               : text.empty()
                                 ? int64 {-1}
                                 : static_cast<int64>(text::PreviousCodePointStart(text, text.length()));

    float wrapWidthOffset =
      _width.UncommittedWidth() + InlineLogicalWidth(renderer, !_appliedStartWidth, true).ToFloat();
    float wrapW = wrapWidthOffset;
    float charWidth = 0;
    // Auto-wrapping text should wrap in the middle of a word only if it could not wrap before the word,
    // which is only possible if the word is the first thing on the line.
    auto isWrappingAllowed = _currentTextWrap != TextWrapMode::NoWrap;
    bool breakWords =
      isWrappingAllowed && style.BreakWords() && !_width.CommittedWidth() && !_width.HasCommittedReplaced();
    bool midWordBreak = false;
    bool breakAnywhere = style.GetLineBreak() == LineBreak::Anywhere && _autoWrap;
    bool breakAll = (style.GetWordBreak() == WordBreak::BreakAll || breakAnywhere) && _autoWrap;
    bool keepAllWords = style.GetWordBreak() == WordBreak::KeepAll;
    auto iteratorMode = layout::TextUtil::LineBreakIteratorMode(_blockStyle.GetLineBreak());
    auto contentAnalysis = layout::TextUtil::GetContentAnalysis(_blockStyle.GetWordBreak());
    bool canUseLineBreakShortcut = iteratorMode == TextBreakLineBehavior::Default
                                   && contentAnalysis == TextBreakIterator::ContentAnalysis::Mechanical;

    if (svgInlineTextRenderer)
    {
      breakWords = false;
      breakAll = false;
    }

    if (_renderTextInfo.Text.get() != &renderer)
    {
      _renderTextInfo.Text = ShareCheckedPtr(&renderer);
      _renderTextInfo.Font = &font;
      _renderTextInfo.LineBreakIteratorFactory.ResetStringAndReleaseIterator(
        text, style.ComputedLocale().GetPlatform(), iteratorMode, contentAnalysis);
    }

    _hasFormerOpportunity = false;
    bool canBreakMidWord = breakWords || breakAll;
    char32 lastCharacter = _renderTextInfo.LineBreakIteratorFactory.GetPriorContext().LastCharacter();
    char32 secondToLastCharacter =
      _renderTextInfo.LineBreakIteratorFactory.GetPriorContext().SecondToLastCharacter();
    for (; _current.Offset() < text.length(); _current.IncrementByCodePointInTextNode())
    {
      krys_debug_assert(&renderer == _current.Renderer());
      bool previousCharacterIsSpace = _currentCharacterIsSpace;
      bool previousCharacterIsWS = _currentCharacterIsWS;
      char32 c = _current.Current();
      _currentCharacterIsSpace = c == U' ' || c == U'\t' || (!_preservesNewline && (c == U'\n'));

      // A single preserved leading white-space doesn't fulfill the 'betweenWords' condition, however it's
      // indeed a soft-breaking opportunty so we may want to avoid breaking in the middle of the word.
      if (_atStart && _currentCharacterIsSpace && !previousCharacterIsSpace)
      {
        _hasFormerOpportunity = !breakAnywhere;
        breakWords = false;
        canBreakMidWord = breakAll;
      }

      if (canHangPunctuationAtStart && !_width.CommittedWidth() && !wrapW
          && !InlineLogicalWidth(renderer, true, false))
      {
        _width.AddUncommittedWidth(-renderer.HangablePunctuationStartWidth(_current.Offset()));
        canHangPunctuationAtStart = false;
      }

      if (canHangPunctuationAtEnd && !_nextObject
          && static_cast<int64>(_current.Offset()) == endPunctuationIndex
          && !InlineLogicalWidth(renderer, false, true))
      {
        _width.AddUncommittedWidth(
          -renderer.HangablePunctuationEndWidth(static_cast<size_t>(endPunctuationIndex)));
        canHangPunctuationAtEnd = false;
      }

      if (!_collapseWhiteSpace || !_currentCharacterIsSpace)
      {
        _lineInfo.SetEmpty(false);
      }

      _currentCharacterIsWS = _currentCharacterIsSpace;

      if (canBreakMidWord && !midWordBreak
          && (!_currentCharacterIsSpace || _atStart
              || !(_currentWhitespaceCollapse == WhiteSpaceCollapse::Preserve
                   && _currentTextWrap == TextWrapMode::Wrap)))
      {
        // FIXME: This code is ultra wrong.
        // The spec says "word-break: break-all: Any typographic letter units are treated as ID(“ideographic
        // characters”) for the purpose of line-breaking." The spec describes how a "typographic letter unit"
        // is a cluster, not a code point: https://drafts.csswg.org/css-text-3/#typographic-character-unit
        wrapW += charWidth;
        charWidth = 0;
        midWordBreak = _width.CommittedWidth() + wrapW + charWidth > _width.AvailableWidth();
      }

      Maybe<uint32> nextBreakablePosition = _current.NextBreakablePosition();
      auto mayBreakHere = !(_currentWhitespaceCollapse == WhiteSpaceCollapse::Preserve
                            && _currentTextWrap == TextWrapMode::NoWrap);
      bool betweenWords =
        c == text::NewlineCharacter
        || (mayBreakHere && !_atStart
            && BreakablePositions::IsBreakable(_renderTextInfo.LineBreakIteratorFactory, _current.Offset(),
                                               nextBreakablePosition, false, canUseLineBreakShortcut,
                                               keepAllWords, breakAnywhere));
      _current.SetNextBreakablePosition(nextBreakablePosition);

      if (canHangStopOrCommaAtLineEnd && RenderText::IsHangableStopOrComma(c) && _width.FitsOnLine())
      {
        // We need to see if a measurement that excludes the stop would fit. If so, then we should hang
        // the stop/comma at the end. First measure including the comma.
        _hangsAtEnd = false;
        float widthIncludingComma = 0;
        _width.AddUncommittedWidth(widthIncludingComma);
        if (!_width.FitsOnLine())
        {
          // See if we fit without the comma involved. If we do, then this is a potential hang point.
          float widthWithoutStopOrComma = 0;
          _width.AddUncommittedWidth(widthWithoutStopOrComma - widthIncludingComma);
          if (_width.FitsOnLine())
          {
            _hangsAtEnd = true;
          }
        }
        else
        {
          _width.AddUncommittedWidth(-widthIncludingComma);
        }
      }

      if (betweenWords || midWordBreak)
      {
        bool stoppedIgnoringSpaces = false;
        if (_ignoringSpaces)
        {
          if (!_currentCharacterIsSpace)
          {
            // Stop ignoring spaces and begin at this new point.
            _ignoringSpaces = false;
            _lineWhitespaceCollapsingState.StopIgnoringSpaces(
              LegacyInlineIterator(nullptr, &renderer, _current.Offset()));
            stoppedIgnoringSpaces = true;
          }
          else
          {
            // Just keep ignoring these spaces.
            NextCharacter(c, lastCharacter, secondToLastCharacter);
            continue;
          }
        }

        float additionalTempWidth = 0;

        if (_collapseWhiteSpace && previousCharacterIsSpace && _currentCharacterIsSpace
            && additionalTempWidth)
        {
          _width.SetTrailingWhitespaceWidth(additionalTempWidth);
        }

        if (!_appliedStartWidth)
        {
          float inlineStartWidth = InlineLogicalWidth(renderer, true, false).ToFloat();
          _width.AddUncommittedWidth(inlineStartWidth);
          _appliedStartWidth = true;
          if (_hangsAtEnd && inlineStartWidth)
          {
            _hangsAtEnd = false;
          }
        }

        if (_autoWrap || breakWords)
        {
          // If we break only after white-space, consider the current character
          // as candidate width for this line.
          bool lineWasTooWide = false;
          auto mayBreakSpaces = _currentWhitespaceCollapse == WhiteSpaceCollapse::BreakSpaces
                                && _currentTextWrap == TextWrapMode::Wrap;
          if (FitsOnLineOrHangsAtEnd() && _currentCharacterIsWS && style.BreakOnlyAfterWhiteSpace()
              && (!midWordBreak || mayBreakSpaces))
          {
            // Check if line is too big even without the extra space
            // at the end of the line. If it is not, do nothing.
            // If the line needs the extra whitespace to be too long,
            // then move the line break to the space and skip all
            // additional whitespace.
            if (!_width.FitsOnLineIncludingExtraWidth(0))
            {
              lineWasTooWide = true;
              if (mayBreakSpaces)
              {
                TrailingSpacesHang(_lineBreak, renderer, canBreakMidWord, previousCharacterIsSpace);
              }
              else
              {
                _lineBreak.MoveTo(renderer, _current.Offset(), _current.NextBreakablePosition());
                _lineBreaker.SkipTrailingWhitespace(_lineBreak, _lineInfo);
              }
            }
          }
          if ((lineWasTooWide || !_width.FitsOnLine()) && !_hangsAtEnd)
          {
            if (_lineBreak.AtTextParagraphSeparator())
            {
              if (!stoppedIgnoringSpaces && _current.Offset() > 0)
              {
                EnsureCharacterGetsLineBox(_lineWhitespaceCollapsingState, _current);
              }
              _lineBreak.Increment();
            }
            // Didn't fit. Jump to the end unless there's still an opportunity to collapse whitespace.
            if (_ignoringSpaces || !_collapseWhiteSpace || !_currentCharacterIsSpace
                || !previousCharacterIsSpace)
            {
              _atEnd = true;
              return false;
            }
          }
          else
          {
            if (!betweenWords || (midWordBreak && !_autoWrap))
            {
              _width.AddUncommittedWidth(-additionalTempWidth);
            }
          }
        }

        if (c == text::NewlineCharacter && _preservesNewline)
        {
          if (!stoppedIgnoringSpaces && _current.Offset())
          {
            EnsureCharacterGetsLineBox(_lineWhitespaceCollapsingState, _current);
          }
          CommitLineBreakAtCurrentWidth(renderer, _current.Offset(), _current.NextBreakablePosition());
          _lineBreak.Increment();
          return true;
        }

        if (_autoWrap && betweenWords)
        {
          CommitLineBreakAtCurrentWidth(renderer, _current.Offset(), _current.NextBreakablePosition());
          wrapWidthOffset = 0;
          wrapW = wrapWidthOffset;
          // Auto-wrapping text should not wrap in the middle of a word once it has had an
          // opportunity to break after a word.
          _hasFormerOpportunity = !breakAnywhere;
          breakWords = false;
          canBreakMidWord = breakAll;
        }

        if (midWordBreak && !(U_GET_GC_MASK(static_cast<UChar32>(c)) & U_GC_M_MASK))
        {
          // Remember this as a breakable position in case
          // adding the end width forces a break.
          _lineBreak.MoveTo(renderer, _current.Offset(), _current.NextBreakablePosition());
          midWordBreak &= canBreakMidWord;
        }

        if (!_ignoringSpaces && style.CollapseWhiteSpace())
        {
          // If we encounter a newline, or if we encounter a second space,
          // we need to break up this run and enter a mode where we start collapsing spaces.
          if (_currentCharacterIsSpace && previousCharacterIsSpace)
          {
            _ignoringSpaces = true;

            // We just entered a mode where we are ignoring
            // spaces. Create a transition to terminate the run
            // before the second space.
            _lineWhitespaceCollapsingState.StartIgnoringSpaces(_startOfIgnoredSpaces);
            _trailingObjects.UpdateWhitespaceCollapsingTransitionsForTrailingBoxes(
              _lineWhitespaceCollapsingState, LegacyInlineIterator(),
              TrailingObjects::CollapseFirstSpace::No);
          }
        }
        // Measuring the width of complex text character-by-character, rather than measuring it all together,
        // could produce considerably different width values.
        if (!renderer.CanUseSimpleFontCodePath() && midWordBreak && _width.FitsOnLine())
        {
          midWordBreak = false;
          wrapW = wrapWidthOffset + additionalTempWidth;
        }
      }
      else
      {
        if (_ignoringSpaces)
        {
          // Stop ignoring spaces and begin at this new point.
          _ignoringSpaces = false;
          _lineWhitespaceCollapsingState.StopIgnoringSpaces(
            LegacyInlineIterator(nullptr, &renderer, _current.Offset()));
        }
        if (_hangsAtEnd && !RenderText::IsHangableStopOrComma(c))
        {
          _hangsAtEnd = false;
        }
      }

      if (svgInlineTextRenderer && _current.Offset())
      {
        // Force creation of new InlineBoxes for each absolute positioned character (those that start new text
        // chunks).
        if (svgInlineTextRenderer->CharacterStartsNewTextChunk(_current.Offset()))
        {
          EnsureCharacterGetsLineBox(_lineWhitespaceCollapsingState, _current);
        }
      }

      if (_currentCharacterIsSpace && !previousCharacterIsSpace)
      {
        _startOfIgnoredSpaces.SetRenderer(&renderer);
        _startOfIgnoredSpaces.SetOffset(_current.Offset());
        // Spaces after right-aligned text and before a line-break get collapsed away completely so that the
        // trailing space doesn't seem to push the text out from the right-hand edge.
        // FIXME: Do this regardless of the container's alignment - will require rebaselining a lot of test
        // results.
        if (_nextObject && _startOfIgnoredSpaces.Offset() && _nextObject->IsBR()
            && _blockStyle.GetTextAlign() == style::TextAlign::Right)
        {
          _startOfIgnoredSpaces.SetOffset(_startOfIgnoredSpaces.Offset() - 1);
          // If there's just a single trailing space start ignoring it now so it collapses away.
          if (_current.Offset() == text.length() - 1)
          {
            _lineWhitespaceCollapsingState.StartIgnoringSpaces(_startOfIgnoredSpaces);
          }
        }
      }

      if (!_currentCharacterIsWS && previousCharacterIsWS)
      {
        if (_autoWrap && style.BreakOnlyAfterWhiteSpace())
        {
          _lineBreak.MoveTo(renderer, _current.Offset(), _current.NextBreakablePosition());
        }
      }

      if (_collapseWhiteSpace && _currentCharacterIsSpace && !_ignoringSpaces)
      {
        _trailingObjects.SetTrailingWhitespace(renderer);
      }
      else if (!style.CollapseWhiteSpace() || !_currentCharacterIsSpace)
      {
        _trailingObjects.Clear();
      }

      _atStart = false;
      NextCharacter(c, lastCharacter, secondToLastCharacter);
    }

    _renderTextInfo.LineBreakIteratorFactory.GetPriorContext().Set({secondToLastCharacter, lastCharacter});

    // IMPORTANT: current.m_pos is > length here!
    float additionalTempWidth = 0;

    float inlineLogicalTempWidth =
      InlineLogicalWidth(renderer, !_appliedStartWidth, _includeEndWidth).ToFloat();
    _width.AddUncommittedWidth(additionalTempWidth + inlineLogicalTempWidth);
    if (_hangsAtEnd && inlineLogicalTempWidth)
    {
      _hangsAtEnd = false;
    }

    if (_collapseWhiteSpace && _currentCharacterIsSpace && additionalTempWidth)
    {
      _width.SetTrailingWhitespaceWidth(additionalTempWidth, inlineLogicalTempWidth);
    }

    _includeEndWidth = false;
    _appliedStartWidth = false;
    return false;
  }

  void BreakingContext::TrailingSpacesHang(LegacyInlineIterator &lineBreak, RenderObject &renderObject,
                                           bool canBreakMidWord, bool previousCharacterIsSpace) noexcept
  {
    krys_debug_assert(_currentWhitespaceCollapse == WhiteSpaceCollapse::BreakSpaces
                      && _currentTextWrap == TextWrapMode::Wrap);
    // Avoid breaking before the first white-space after a word if there is a
    // breaking opportunity before.
    if (_hasFormerOpportunity && !previousCharacterIsSpace)
    {
      return;
    }

    lineBreak.MoveTo(renderObject, _current.Offset(), _current.NextBreakablePosition());

    // Avoid breaking before the first white-space after a word, unless
    // overflow-wrap or word-break allow to.
    if (!previousCharacterIsSpace && !canBreakMidWord)
    {
      lineBreak.Increment();
    }
  }

  void BreakingContext::CommitAndUpdateLineBreakIfNeeded() noexcept
  {
    bool checkForBreak = CanBreakAtThisPosition();

    if (checkForBreak && !_width.FitsOnLine(_ignoringSpaces) && !_hangsAtEnd)
    {
      // if we have floats, try to get below them.
      if (_currentCharacterIsSpace && !_ignoringSpaces && _collapseWhiteSpace)
      {
        _trailingObjects.Clear();
      }

      if (_width.CommittedWidth())
      {
        _atEnd = true;
        return;
      }

      // |width| may have been adjusted because we got shoved down past a float (thus
      // giving us more room), so we need to retest, and only jump to
      // the end label if we still don't fit on the line. -dwh
      if (!_width.FitsOnLine(_ignoringSpaces))
      {
        _atEnd = true;
        return;
      }
    }

    if (!_current.Renderer()->IsFloatingOrOutOfFlowPositioned())
    {
      _lastObject = _current.Renderer();
      if (_lastObject->IsBlockLevelReplacedOrAtomicInline() && _autoWrap
          && (!_lastObject->IsRenderImage() || _allowImagesToBreak))
      {
        auto *renderListMarker = DynamicDowncast<RenderListMarker>(*_lastObject);
        if (!renderListMarker || renderListMarker->IsInside())
        {
          if (_nextObject)
          {
            CommitLineBreakAtCurrentWidth(*_nextObject);
          }
          else
          {
            CommitLineBreakClear();
          }
        }
      }
    }
  }

  LegacyInlineIterator BreakingContext::HandleEndOfLine() noexcept
  {
    if (_lineBreak == _resolver.Position())
    {
      if (!_lineBreak.Renderer() || !_lineBreak.Renderer()->IsBR())
      {
        // we just add as much as possible
        uint32 objectLength = 0;
        if (auto *renderText = DynamicDowncast<RenderText>(_lastObject))
        {
          objectLength = static_cast<uint32>(renderText->Length());
        }
        if (_blockStyle.GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve
            && _blockStyle.GetTextWrapMode() == TextWrapMode::NoWrap && !_current.Offset())
        {
          if (_lastObject)
          {
            CommitLineBreakAtCurrentWidth(*_lastObject, objectLength);
          }
          else
          {
            CommitLineBreakClear();
          }
        }
        else if (_lineBreak.Renderer())
        {
          // Don't ever break in the middle of a word if we can help it.
          // There's no room at all. We just have to be on this line,
          // even though we'll spill out.
          if (_current.Renderer())
          {
            CommitLineBreakAtCurrentWidth(*_current.Renderer(), _current.Offset());
          }
          else if (_lastObject)
          {
            CommitLineBreakAtCurrentWidth(*_lastObject, objectLength);
          }
        }
      }
      // make sure we consume at least one char/object.
      if (_lineBreak == _resolver.Position())
      {
        _lineBreak.Increment();
      }
    }
    else if (!_current.Offset() && !_width.CommittedWidth() && _width.UncommittedWidth()
             && !_hadUncommittedWidthBeforeCurrent)
    {
      // Do not push the current object to the next line, when this line has some content, but it is still
      // considered empty. Empty inline elements like <span></span> can produce such lines and now we just
      // ignore these break opportunities at the start of a line, if no width has been committed yet. Behave
      // as if it was actually empty and consume at least one object.
      auto overflowingBoxIsInlineLevelBox =
        _lineBreak.Renderer() && !_lineBreak.Renderer()->IsFloatingOrOutOfFlowPositioned();
      if (overflowingBoxIsInlineLevelBox)
      {
        _lineBreak.Increment();
      }
    }

    // Sanity check our whitespace collapsing transitions.
    auto collapsed = CheckWhitespaceCollapsingTransitions(_lineWhitespaceCollapsingState, _lineBreak);
    _trailingObjects.UpdateWhitespaceCollapsingTransitionsForTrailingBoxes(_lineWhitespaceCollapsingState,
                                                                           _lineBreak, collapsed);

    // We might have made lineBreak an iterator that points past the end
    // of the object. Do this adjustment to make it point to the start
    // of the next object instead to avoid confusing the rest of the
    // code.
    if (_lineBreak.Offset())
    {
      _lineBreak.SetOffset(_lineBreak.Offset() - 1);
      _lineBreak.Increment();
    }

    return _lineBreak;
  }
}
