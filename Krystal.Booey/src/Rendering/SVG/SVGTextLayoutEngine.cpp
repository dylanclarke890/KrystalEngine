#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutEngine.hpp"
#include "Krystal.Booey/Graphics/PathTraversalState.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGTextPath.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutEngineBaseline.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutEngineSpacing.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGGeometryElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/SVG/SVGTextContentElement.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo
{
  SVGTextLayoutEngine::SVGTextLayoutEngine(List<SVGTextLayoutAttributes *> &layoutAttributes) noexcept
      : _layoutAttributes(layoutAttributes)
  {
    krys_debug_assert(!_layoutAttributes.empty());
  }

  void SVGTextLayoutEngine::UpdateCharacterPositionIfNeeded(float &x, float &y) noexcept
  {
    if (_inPathLayout)
    {
      return;
    }

    // Replace characters x/y position, with the current text position plus any
    // relative adjustments, if it doesn't specify an absolute position itself.
    if (SVGTextLayoutAttributes::IsEmptyValue(x))
    {
      x = _x + _dx;
    }

    if (SVGTextLayoutAttributes::IsEmptyValue(y))
    {
      y = _y + _dy;
    }

    _dx = 0;
    _dy = 0;
  }

  void SVGTextLayoutEngine::UpdateCurrentTextPosition(float x, float y, float glyphAdvance) noexcept
  {
    // Update current text position after processing the character.
    if (_isVerticalText)
    {
      _x = x;
      _y = y + glyphAdvance;
    }
    else
    {
      _x = x + glyphAdvance;
      _y = y;
    }
  }

  void SVGTextLayoutEngine::UpdateRelativePositionAdjustmentsIfNeeded(float dx, float dy) noexcept
  {
    // Update relative positioning information.
    if (SVGTextLayoutAttributes::IsEmptyValue(dx) && SVGTextLayoutAttributes::IsEmptyValue(dy))
    {
      return;
    }

    if (SVGTextLayoutAttributes::IsEmptyValue(dx))
    {
      dx = 0;
    }
    if (SVGTextLayoutAttributes::IsEmptyValue(dy))
    {
      dy = 0;
    }

    if (_inPathLayout)
    {
      if (_isVerticalText)
      {
        _dx += dx;
        _dy = dy;
      }
      else
      {
        _dx = dx;
        _dy += dy;
      }

      return;
    }

    _dx = dx;
    _dy = dy;
  }

  void SVGTextLayoutEngine::RecordTextFragment(inline_iterator::SVGTextBoxIterator textBox,
                                               const List<SVGTextMetrics> &textMetricsValues) noexcept
  {
    krys_debug_assert(!_currentTextFragment.Length);
    krys_debug_assert(_visualMetricsListOffset > 0);

    // Figure out length of fragment.
    _currentTextFragment.Length = _visualCharacterOffset - _currentTextFragment.CharacterOffset;

    // Figure out fragment metrics.
    auto &lastCharacterMetrics = textMetricsValues.at(_visualMetricsListOffset - 1);
    _currentTextFragment.Width = lastCharacterMetrics.Width();
    _currentTextFragment.Height = lastCharacterMetrics.Height();

    if (_visualMetricsListOffset - _currentTextFragment.MetricsListOffset > 1)
    {
      // SVGTextLayoutAttributesBuilder assures that the length of the range is equal to the sum of the
      // individual lengths of the glyphs.
      float length = 0;
      if (_isVerticalText)
      {
        for (uint32 i = _currentTextFragment.MetricsListOffset; i < _visualMetricsListOffset; ++i)
        {
          length += textMetricsValues.at(i).Height();
        }
        _currentTextFragment.Height = length;
      }
      else
      {
        for (uint32 i = _currentTextFragment.MetricsListOffset; i < _visualMetricsListOffset; ++i)
        {
          length += textMetricsValues.at(i).Width();
        }
        _currentTextFragment.Width = length;
      }
    }

    _fragmentMap[inline_iterator::MakeKey(*textBox)].push_back(_currentTextFragment);
    _currentTextFragment = SVGTextFragment();
  }

  bool SVGTextLayoutEngine::ParentDefinesTextLength(RenderObject *parent) const noexcept
  {
    RenderObject *currentParent = parent;
    while (currentParent)
    {
      if (RefPtr textContentElement =
            ShareRefPtr(svg::SVGTextContentElement::ElementFromRenderer(currentParent)))
      {
        svg::SVGLengthContext lengthContext(textContentElement.get());
        if (textContentElement->LengthAdjust() == svg::SVGLengthAdjustSpacing
            && textContentElement->SpecifiedTextLength().Value(lengthContext) > 0)
        {
          return true;
        }
      }

      if (currentParent->IsRenderSVGText())
      {
        return false;
      }

      currentParent = currentParent->Parent();
    }

    krys_debug_assert(false);
    return false;
  }

  void SVGTextLayoutEngine::BeginTextPathLayout(const RenderSVGTextPath &textPath,
                                                SVGTextLayoutEngine &lineLayout) noexcept
  {
    _inPathLayout = true;

    _textPath = textPath.LayoutPath();
    if (_textPath.IsEmpty())
    {
      return;
    }

    const auto &startOffset = textPath.StartOffset();
    _textPathLength = _textPath.Length();

    if (textPath.StartOffset().LengthType() == svg::SVGLengthType::Percentage)
    {
      _textPathStartOffset = startOffset.ValueAsPercentage() * _textPathLength;
    }
    else
    {
      _textPathStartOffset = startOffset.ValueInSpecifiedUnits();
      if (RefPtr targetElement = ShareRefPtr(textPath.TargetElement()))
      {
        // FIXME: A value of zero is valid. Need to differentiate this case from being unspecified.
        if (float pathLength = targetElement->PathLength())
        {
          _textPathStartOffset *= _textPathLength / pathLength;
        }
      }
    }

    lineLayout._chunkLayoutBuilder.BuildTextChunks(
      lineLayout._lineLayoutBoxes, lineLayout._lineLayoutChunkStarts, lineLayout._fragmentMap);

    // Handle text-anchor as additional start offset for text paths.
    _textPathStartOffset += lineLayout._chunkLayoutBuilder.TotalAnchorShift();
    _textPathCurrentOffset = _textPathStartOffset;

    // Eventually handle textLength adjustments.
    RefPtr textContentElement = ShareRefPtr(svg::SVGTextContentElement::ElementFromRenderer(&textPath));
    if (!textContentElement)
    {
      return;
    }

    svg::SVGLengthContext lengthContext(textContentElement.get());
    float desiredTextLength = textContentElement->SpecifiedTextLength().Value(lengthContext);
    if (!desiredTextLength)
    {
      return;
    }

    float totalLength = lineLayout._chunkLayoutBuilder.TotalLength();
    uint32 totalCharacters = lineLayout._chunkLayoutBuilder.TotalCharacters();

    if (textContentElement->LengthAdjust() == svg::SVGLengthAdjustSpacing)
    {
      if (totalCharacters > 1)
      {
        _textPathSpacing = (desiredTextLength - totalLength) / (totalCharacters - 1);
      }
    }
    else
    {
      _textPathScaling = desiredTextLength / totalLength;
    }
  }

  void SVGTextLayoutEngine::EndTextPathLayout() noexcept
  {
    _inPathLayout = false;
    _textPath = Path();
    _textPathLength = 0;
    _textPathStartOffset = 0;
    _textPathCurrentOffset = 0;
    _textPathSpacing = 0;
    _textPathScaling = 1;
  }

  void SVGTextLayoutEngine::LayoutInlineTextBox(inline_iterator::SVGTextBoxIterator textBox) noexcept
  {
    auto &text = textBox->Renderer();
    krys_debug_assert(text.Parent());
    krys_debug_assert(text.Parent()->GetElement());
    krys_debug_assert(Is<svg::SVGElement>(*text.Parent()->GetElement()));

    const RenderStyle &style = text.Style();

    _isVerticalText = style.GetWritingMode().IsVertical();
    LayoutTextOnLineOrPath(textBox, text, style);

    if (_inPathLayout)
    {
      _pathLayoutBoxes.push_back(textBox);
      return;
    }

    _lineLayoutBoxes.push_back(textBox);
  }

  void SVGTextLayoutEngine::FinalizeTransformMatrices(
    List<inline_iterator::SVGTextBoxIterator> &textBoxes) noexcept
  {
    if (textBoxes.empty())
    {
      return;
    }

    for (auto &textBox : textBoxes)
    {
      auto textBoxTransformation = _chunkLayoutBuilder.TransformationForTextBox(textBox);
      if (textBoxTransformation.IsIdentity())
      {
        continue;
      }

      auto it = _fragmentMap.find(inline_iterator::MakeKey(*textBox));
      if (it != _fragmentMap.end())
      {
        for (auto &fragment : it->second)
        {
          krys_debug_assert(fragment.LengthAdjustTransform.IsIdentity());
          fragment.LengthAdjustTransform = textBoxTransformation;
        }
      }
    }

    textBoxes.clear();
  }

  SVGTextFragmentMap SVGTextLayoutEngine::FinishLayout() noexcept
  {
    // After all text fragments are stored in their correpsonding SVGInlineTextBoxes, we can layout individual
    // text chunks. Chunk layouting is only performed for line layout boxes, not for path layout, where it has
    // already been done.
    _chunkLayoutBuilder.LayoutTextChunks(_lineLayoutBoxes, _lineLayoutChunkStarts, _fragmentMap);

    // Finalize transform matrices, after the chunk layout corrections have been applied, and all fragment x/y
    // positions are finalized.
    if (!_lineLayoutBoxes.empty())
    {
      FinalizeTransformMatrices(_lineLayoutBoxes);
    }

    if (!_pathLayoutBoxes.empty())
    {
      FinalizeTransformMatrices(_pathLayoutBoxes);
    }

    return krys::move(_fragmentMap);
  }

  bool SVGTextLayoutEngine::CurrentLogicalCharacterAttributes(
    SVGTextLayoutAttributes *&logicalAttributes) noexcept
  {
    if (_layoutAttributesPosition == _layoutAttributes.size())
    {
      return false;
    }

    logicalAttributes = _layoutAttributes[_layoutAttributesPosition];
    krys_debug_assert(logicalAttributes);

    if (_logicalCharacterOffset != logicalAttributes->Context().Text().size())
    {
      return true;
    }

    ++_layoutAttributesPosition;
    if (_layoutAttributesPosition == _layoutAttributes.size())
    {
      return false;
    }

    logicalAttributes = _layoutAttributes[_layoutAttributesPosition];
    _logicalMetricsListOffset = 0;
    _logicalCharacterOffset = 0;
    return true;
  }

  bool SVGTextLayoutEngine::CurrentLogicalCharacterMetrics(SVGTextLayoutAttributes *&logicalAttributes,
                                                           SVGTextMetrics &logicalMetrics) noexcept
  {
    List<SVGTextMetrics> *textMetricsValues = &logicalAttributes->TextMetricsValues();
    auto textMetricsSize = static_cast<uint32>(textMetricsValues->size());
    while (true)
    {
      if (_logicalMetricsListOffset == textMetricsSize)
      {
        if (!CurrentLogicalCharacterAttributes(logicalAttributes))
        {
          return false;
        }

        textMetricsValues = &logicalAttributes->TextMetricsValues();
        textMetricsSize = static_cast<uint32>(textMetricsValues->size());
        continue;
      }

      krys_debug_assert(textMetricsSize);
      krys_assert(_logicalMetricsListOffset < textMetricsSize);
      logicalMetrics = textMetricsValues->at(_logicalMetricsListOffset);
      if (logicalMetrics.IsEmpty() || (!logicalMetrics.Width() && !logicalMetrics.Height()))
      {
        AdvanceToNextLogicalCharacter(logicalMetrics);
        continue;
      }

      // Stop if we found the next valid logical text metrics object.
      return true;
    }

    krys_debug_assert(false);
    return true;
  }

  bool SVGTextLayoutEngine::CurrentVisualCharacterMetrics(const inline_iterator::SVGTextBox &textBox,
                                                          const List<SVGTextMetrics> &visualMetricsValues,
                                                          SVGTextMetrics &visualMetrics) noexcept
  {
    krys_debug_assert(!visualMetricsValues.empty());
    auto textMetricsSize = static_cast<uint32>(visualMetricsValues.size());
    uint32 boxStart = textBox.Start();
    uint32 boxLength = textBox.Length();

    while (_visualMetricsListOffset < textMetricsSize)
    {
      // Advance to text box start location.
      if (_visualCharacterOffset < boxStart)
      {
        AdvanceToNextVisualCharacter(visualMetricsValues[_visualMetricsListOffset]);
        continue;
      }

      // Stop if we've finished processing this text box.
      if (_visualCharacterOffset >= boxStart + boxLength)
      {
        return false;
      }

      visualMetrics = visualMetricsValues[_visualMetricsListOffset];
      return true;
    }

    return false;
  }

  void SVGTextLayoutEngine::AdvanceToNextLogicalCharacter(const SVGTextMetrics &logicalMetrics) noexcept
  {
    ++_logicalMetricsListOffset;
    _logicalCharacterOffset += logicalMetrics.Length();
  }

  void SVGTextLayoutEngine::AdvanceToNextVisualCharacter(const SVGTextMetrics &visualMetrics) noexcept
  {
    ++_visualMetricsListOffset;
    _visualCharacterOffset += visualMetrics.Length();
  }

  void SVGTextLayoutEngine::LayoutTextOnLineOrPath(inline_iterator::SVGTextBoxIterator textBox,
                                                   const RenderSVGInlineText &text,
                                                   const RenderStyle &style) noexcept
  {
    if (_inPathLayout && _textPath.IsEmpty())
    {
      return;
    }

    RenderElement *textParent = text.Parent();
    krys_debug_assert(textParent);

    bool definesTextLength = ParentDefinesTextLength(textParent);

    _visualMetricsListOffset = 0;
    _visualCharacterOffset = 0;

    auto &visualMetricsValues = text.LayoutAttributes()->TextMetricsValues();
    krys_debug_assert(!visualMetricsValues.empty());

    auto characters = text.Text();
    const FontCascade &font = style.GetFontCascade();

    SVGTextLayoutEngineSpacing spacingLayout(font);
    SVGTextLayoutEngineBaseline baselineLayout(font);

    bool didStartTextFragment = false;
    bool applySpacingToNextCharacter = false;

    float lastAngle = 0;
    float baselineShift = baselineLayout.CalculateBaselineShift(style);
    baselineShift -= baselineLayout.CalculateAlignmentBaselineShift(_isVerticalText, text);

    // Main layout algorithm.
    while (true)
    {
      // Find the start of the current text box in this list, respecting ligatures.
      SVGTextMetrics visualMetrics(SVGTextMetrics::MetricsType::SkippedSpaceMetrics);
      if (!CurrentVisualCharacterMetrics(*textBox, visualMetricsValues, visualMetrics))
      {
        break;
      }

      if (visualMetrics.IsEmpty())
      {
        AdvanceToNextVisualCharacter(visualMetrics);
        continue;
      }

      SVGTextLayoutAttributes *logicalAttributes = nullptr;
      if (!CurrentLogicalCharacterAttributes(logicalAttributes))
      {
        break;
      }

      krys_debug_assert(logicalAttributes);
      SVGTextMetrics logicalMetrics(SVGTextMetrics::MetricsType::SkippedSpaceMetrics);
      if (!CurrentLogicalCharacterMetrics(logicalAttributes, logicalMetrics))
      {
        break;
      }

      SVGCharacterDataMap &characterDataMap = logicalAttributes->CharacterDataMap();
      SVGCharacterData data;
      auto it = characterDataMap.find(_logicalCharacterOffset + 1);
      if (it != characterDataMap.end())
      {
        data = it->second;
      }

      float x = data.X;
      float y = data.Y;
      auto previousBoxOnLine = textBox->NextLineLeftwardOnLine();

      bool hasXOrY = !SVGTextLayoutAttributes::IsEmptyValue(x) || !SVGTextLayoutAttributes::IsEmptyValue(y);

      // If we start a new chunk following an chunk that had a textLength set, use that
      // textLength to determine the chunk start position, instead of glyph advance values.
      auto moveToExpectedChunkStartPositionIfNeeded = [&]()
      {
        if (_inPathLayout || !_lastChunkHasTextLength || !previousBoxOnLine)
        {
          return;
        }

        if (_isVerticalText)
        {
          if (!SVGTextLayoutAttributes::IsEmptyValue(y))
          {
            return;
          }
        }
        else
        {
          if (!SVGTextLayoutAttributes::IsEmptyValue(x))
          {
            return;
          }
        }

        RefPtr textContentElement =
          ShareRefPtr(svg::SVGTextContentElement::ElementFromRenderer(&previousBoxOnLine->Renderer()));
        if (!textContentElement)
        {
          return;
        }

        svg::SVGLengthContext lengthContext(textContentElement.get());
        auto specifiedTextLength = textContentElement->SpecifiedTextLength().Value(lengthContext);

        if (_lastChunkIsVerticalText)
        {
          y = _lastChunkStartPosition + specifiedTextLength;
        }
        else
        {
          x = _lastChunkStartPosition + specifiedTextLength;
        }
      };

      bool startsNewTextChunk = [&]()
      {
        // If we're at a position that could start a new text chunk, but doesn't for intrinsic reasons (no x/y
        // information specified for the current character), check further if there are other conditions met
        // that enforce a new text chunk -- e.g. previous sibiling on the same line specified 'textLength'
        // (consider: <text><tspan textLength="100">AB</tspan> <tspan dy="1em">... The space character is not
        // allowed to be part of the 'AB' text chunk -- there is not explicit x/y given for the space
        // character but because of the textLength attribute, we have to keep the space in a separated chunk,
        // and position it such that it renders after the user-specified textLength.
        if (logicalAttributes->Context().CharacterStartsNewTextChunk(
              static_cast<int32>(_logicalCharacterOffset)))
        {
          return true;
        }

        // If we encounter an inline text box that follows an inline flow box with specified textLength,
        // and if the text box content is not positioned by explicit x/y attributes, then we have
        // to correct the position of the text box, to account for the textLength adjustments
        // that will be applied on chunk-level in the next SVG text layout phase. Failing to do so,
        // will lay out the remaining content at the nominal position, as if no textLength was given.
        if (_lastChunkHasTextLength && previousBoxOnLine)
        {
          return true;
        }

        return false;
      }();

      // When we've advanced to the box start offset, determine using the original x/y values
      // whether this character starts a new text chunk before doing any further processing.
      if (_visualCharacterOffset == textBox->Start())
      {
        moveToExpectedChunkStartPositionIfNeeded();
        if (startsNewTextChunk)
        {
          _lineLayoutChunkStarts.insert(inline_iterator::MakeKey(*textBox));
        }
      }

      float angle = SVGTextLayoutAttributes::IsEmptyValue(data.Rotate) ? 0 : data.Rotate;

      // Calculate glyph orientation angle.
      char32 currentCharacter = krys::text::CodePointAt(characters, _visualCharacterOffset);
      float orientationAngle =
        baselineLayout.CalculateGlyphOrientationAngle(_isVerticalText, style, currentCharacter);

      // Calculate glyph advance & x/y orientation shifts.
      float xOrientationShift = 0;
      float yOrientationShift = 0;
      float glyphAdvance = baselineLayout.CalculateGlyphAdvanceAndOrientation(
        _isVerticalText, visualMetrics, orientationAngle, xOrientationShift, yOrientationShift);

      // Assign current text position to x/y values, if needed.
      UpdateCharacterPositionIfNeeded(x, y);

      // Apply dx/dy value adjustments to current text position, if needed.
      UpdateRelativePositionAdjustmentsIfNeeded(data.Dx, data.Dy);

      // Calculate CSS 'letter-spacing' and 'word-spacing' for next character, if needed.
      float spacing = spacingLayout.CalculateCSSSpacing(currentCharacter);

      float textPathOffset = 0;
      if (_inPathLayout)
      {
        float scaledGlyphAdvance = glyphAdvance * _textPathScaling;
        if (_isVerticalText)
        {
          // If there's an absolute y position available, it marks the beginning of a new position along the
          // path.
          if (!SVGTextLayoutAttributes::IsEmptyValue(y))
          {
            _textPathCurrentOffset = y + _textPathStartOffset;
          }

          _textPathCurrentOffset += _dy;
          _dy = 0;

          // Apply dx/dy correction and setup translations that move to the glyph midpoint.
          xOrientationShift += _dx + baselineShift;
          yOrientationShift -= scaledGlyphAdvance / 2;
        }
        else
        {
          // If there's an absolute x position available, it marks the beginning of a new position along the
          // path.
          if (!SVGTextLayoutAttributes::IsEmptyValue(x))
          {
            _textPathCurrentOffset = x + _textPathStartOffset;
          }

          _textPathCurrentOffset += _dx;
          _dx = 0;

          // Apply dx/dy correction and setup translations that move to the glyph midpoint.
          xOrientationShift -= scaledGlyphAdvance / 2;
          yOrientationShift += _dy - baselineShift;
        }

        // Calculate current offset along path.
        textPathOffset = _textPathCurrentOffset + scaledGlyphAdvance / 2;

        // Move to next character.
        _textPathCurrentOffset += scaledGlyphAdvance + _textPathSpacing + spacing * _textPathScaling;

        // Skip character, if we're before the path.
        if (textPathOffset < 0)
        {
          AdvanceToNextLogicalCharacter(logicalMetrics);
          AdvanceToNextVisualCharacter(visualMetrics);
          continue;
        }

        // Stop processing, if the next character lies behind the path.
        if (textPathOffset > _textPathLength)
        {
          break;
        }

        auto traversalState = _textPath.TraversalStateAtLength(textPathOffset);
        krys_debug_assert(traversalState.Success());

        FloatPoint point = traversalState.Current();
        x = point.X();
        y = point.Y();

        angle = traversalState.NormalAngle();

        // For vertical text on path, the actual angle has to be rotated 90 degrees anti-clockwise, not the
        // orientation angle!
        if (_isVerticalText)
        {
          angle -= 90;
        }
      }
      else
      {
        // Apply all previously calculated shift values.
        if (_isVerticalText)
        {
          x += baselineShift;
        }
        else
        {
          y -= baselineShift;
        }

        x += _dx;
        y += _dy;
      }

      // Remember the position / direction of the start position of the new text chunk.
      if (startsNewTextChunk)
      {
        _lastChunkStartPosition = _isVerticalText ? y : x;
        _lastChunkIsVerticalText = _isVerticalText;
        _lastChunkHasTextLength = definesTextLength;
      }

      // Determine whether we have to start a new fragment.
      bool shouldStartNewFragment = hasXOrY || _dx || _dy || _isVerticalText || _inPathLayout || angle
                                    || angle != lastAngle || orientationAngle || applySpacingToNextCharacter
                                    || definesTextLength;

      // If we already started a fragment, close it now.
      if (didStartTextFragment && shouldStartNewFragment)
      {
        applySpacingToNextCharacter = false;
        RecordTextFragment(textBox, visualMetricsValues);
      }

      // Eventually start a new fragment, if not yet done.
      if (!didStartTextFragment || shouldStartNewFragment)
      {
        krys_debug_assert(!_currentTextFragment.CharacterOffset);
        krys_debug_assert(!_currentTextFragment.Length);

        didStartTextFragment = true;
        _currentTextFragment.CharacterOffset = _visualCharacterOffset;
        _currentTextFragment.MetricsListOffset = _visualMetricsListOffset;
        _currentTextFragment.X = x;
        _currentTextFragment.Y = y;

        // Build fragment transformation.
        if (angle)
        {
          _currentTextFragment.Transform.Rotate(angle);
        }

        if (xOrientationShift || yOrientationShift)
        {
          _currentTextFragment.Transform.Translate(xOrientationShift, yOrientationShift);
        }

        if (orientationAngle)
        {
          _currentTextFragment.Transform.Rotate(orientationAngle);
        }

        _currentTextFragment.IsTextOnPath = _inPathLayout && _textPathScaling != 1;
        if (_currentTextFragment.IsTextOnPath)
        {
          if (_isVerticalText)
          {
            _currentTextFragment.LengthAdjustTransform.ScaleNonUniform(1, _textPathScaling);
          }
          else
          {
            _currentTextFragment.LengthAdjustTransform.ScaleNonUniform(_textPathScaling, 1);
          }
        }
      }

      // Update current text position, after processing of the current character finished.
      if (_inPathLayout)
      {
        UpdateCurrentTextPosition(x, y, glyphAdvance);
      }
      else
      {
        // Apply CSS 'letter-spacing' and 'word-spacing' to next character, if needed.
        if (spacing)
        {
          applySpacingToNextCharacter = true;
        }

        float xNew = x - _dx;
        float yNew = y - _dy;

        if (_isVerticalText)
        {
          xNew -= baselineShift;
        }
        else
        {
          yNew += baselineShift;
        }

        UpdateCurrentTextPosition(xNew, yNew, glyphAdvance + spacing);
      }

      AdvanceToNextLogicalCharacter(logicalMetrics);
      AdvanceToNextVisualCharacter(visualMetrics);
      lastAngle = angle;
    }

    if (!didStartTextFragment)
    {
      return;
    }

    // Close last open fragment, if needed.
    RecordTextFragment(textBox, visualMetricsValues);
  }
}
