#include "Krystal.Booey/Rendering/SVG/SVGTextMetricsBuilder.hpp"
#include "Krystal.Booey/Graphics/ComplexTextController.hpp"
#include "Krystal.Booey/Graphics/FontCascadeCache.hpp"
#include "Krystal.Booey/Graphics/GlyphBuffer.hpp"
#include "Krystal.Booey/Graphics/WidthIterator.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/Text/TextUtil.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGText.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD uint32 CodePointLengthAt(utf8_stringview text, uint32 offset) noexcept
    {
      return krys::text::DecodeUTF8(Span<const char8>(text.data() + offset, text.size() - offset)).Length;
    }

    KRYS_NODISCARD uint32 NumberOfCodePoints(utf8_stringview text) noexcept
    {
      uint32 count = 0;
      for (uint32 offset = 0; offset < text.size(); offset += CodePointLengthAt(text, offset))
      {
        ++count;
      }
      return count;
    }
  }

  SVGTextMetricsBuilder::SVGTextMetricsBuilder() noexcept : _run(utf8_stringview())
  {
  }

  uint32 SVGTextMetricsBuilder::CurrentCharacterLength() const noexcept
  {
    return CodePointLengthAt(_run.Text(), _textPosition);
  }

  template <typename TIterator>
  bool SVGTextMetricsBuilder::Advance(TIterator &iterator) noexcept
  {
    _textPosition += _currentMetrics.Length();
    if (_textPosition >= _run.Length())
    {
      return false;
    }

    AdvanceIterator(iterator);
    return _currentMetrics.Length() > 0;
  }

  void SVGTextMetricsBuilder::AdvanceIterator(WidthIterator &simpleWidthIterator) noexcept
  {
    GlyphBuffer glyphBuffer;
    auto before = simpleWidthIterator.CurrentCharacterIndex();
    simpleWidthIterator.Advance(_textPosition + CurrentCharacterLength(), glyphBuffer);
    auto after = simpleWidthIterator.CurrentCharacterIndex();
    if (before == after)
    {
      _currentMetrics = SVGTextMetrics();
      return;
    }

    float currentWidth = simpleWidthIterator.RunWidthSoFar() - _totalWidth;
    _totalWidth = simpleWidthIterator.RunWidthSoFar();

    _currentMetrics = SVGTextMetrics(*_text, after - before, currentWidth);
  }

  void SVGTextMetricsBuilder::AdvanceIterator(ComplexTextController &complexTextController) noexcept
  {
    uint32 metricsLength = CurrentCharacterLength();
    float beforeWidth = 0;
    float afterWidth = 0;

    complexTextController.Advance(_textPosition, nullptr);
    beforeWidth = complexTextController.RunWidthSoFar();

    complexTextController.Advance(_textPosition + metricsLength, nullptr);
    afterWidth = complexTextController.RunWidthSoFar();

    _currentMetrics = SVGTextMetrics(*_text, metricsLength, afterWidth - beforeWidth);
    _complexStartToCurrentMetrics = SVGTextMetrics(*_text, _textPosition + metricsLength, afterWidth);

    krys_debug_assert(_currentMetrics.Length() == metricsLength);

    // Frequent case for Arabic text: when measuring a single character the arabic isolated form is taken
    // when rendering the glyph "in context" (with it's surrounding characters) it changes due to shaping.
    // So whenever currentWidth != currentMetrics.width(), we are processing a text run whose length is
    // not equal to the sum of the individual lengths of the glyphs, when measuring them isolated.
    float currentWidth = _complexStartToCurrentMetrics.Width() - _totalWidth;
    if (currentWidth != _currentMetrics.Width())
    {
      _currentMetrics.SetWidth(currentWidth);
    }

    _totalWidth = _complexStartToCurrentMetrics.Width();
  }

  void SVGTextMetricsBuilder::InitializeMeasurementWithTextRenderer(RenderSVGInlineText &text) noexcept
  {
    _text = CreateWeakPtr(&text);
    _textPosition = 0;
    _currentMetrics = SVGTextMetrics();
    _complexStartToCurrentMetrics = SVGTextMetrics();
    _totalWidth = 0;

    const FontCascade &scaledFont = text.ScaledFont();
    _run = SVGTextMetrics::ConstructTextRun(text);
    _isComplexText = scaledFont.GetCodePath(_run) == FontCascade::CodePath::Complex;

    if (_isComplexText)
    {
      FontCascadeCache::Singleton().Invalidate();
    }

    _canUseSimplifiedTextMeasuring = false;
    if (!_isComplexText)
    {
      if (auto cachedValue = text.CanUseSimplifiedTextMeasuring())
      {
        _canUseSimplifiedTextMeasuring = cachedValue.value();
      }
      else
      {
        // Currently SVG implementation does not support first-line, so we always pass nullptr for
        // firstLineStyle. When supporting first-line, we also need to update firstLineStyle's FontCascade to
        // be aligned with scaledFont in RenderSVGInlineText.
        _canUseSimplifiedTextMeasuring = layout::TextUtil::CanUseSimplifiedTextMeasuring(
          _run.Text(), scaledFont, text.Style().CollapseWhiteSpace(), nullptr);
        text.SetCanUseSimplifiedTextMeasuring(_canUseSimplifiedTextMeasuring);
      }
    }
  }

  struct MeasureTextData
  {
    explicit MeasureTextData(SVGCharacterDataMap *characterDataMap) noexcept
        : AllCharactersMap(characterDataMap)
    {
    }

    SVGCharacterDataMap *AllCharactersMap;
    bool ProcessRenderer {false};
  };

  std::tuple<uint32, char32>
    SVGTextMetricsBuilder::MeasureTextRenderer(RenderSVGInlineText &text, const MeasureTextData &data,
                                               std::tuple<uint32, char32> state) noexcept
  {
    SVGTextLayoutAttributes *attributes = text.LayoutAttributes();
    krys_debug_assert(attributes);
    List<SVGTextMetrics> &textMetricsValues = attributes->TextMetricsValues();
    if (data.ProcessRenderer)
    {
      if (data.AllCharactersMap)
      {
        attributes->Clear();
      }
      else
      {
        textMetricsValues.clear();
      }
    }

    InitializeMeasurementWithTextRenderer(text);

    auto &scaledFont = text.ScaledFont();
    if (_canUseSimplifiedTextMeasuring && data.ProcessRenderer)
    {
      // If we are not specifying specific configuration for characters, data.AllCharactersMap has only 1
      // entry for default case. This is extremely common, and that's why we crafted a fast path here.
      // FIXME: For any cases, we are handling one character by one character in SVGTextMetrics. But many
      // texts do not have characterDataMap. We should handle multiple characters in one SVGTextMetrics. This
      // also makes RTL work.
      // FIXME: This function is called even though width information is not changed at all. RenderSVGText /
      // RenderSVGInlineText should track the potential changes to width etc. and invoke this function only
      // when it is actually changed.
      if (data.AllCharactersMap && _run.Direction() == TextDirection::LTR
          && data.AllCharactersMap->size() == 1)
      {
        constexpr uint32 defaultPosition = 1;
        // "1" is the default value and always exists.
        krys_debug_assert(data.AllCharactersMap->contains(defaultPosition));
        auto characterData = data.AllCharactersMap->at(defaultPosition);

        auto [valueListPosition, lastCharacter] = state;
        bool preserveWhiteSpace = text.Style().GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve;
        auto view = _run.Text();
        auto length = static_cast<uint32>(view.size());
        uint32 characterIndex = 0;
        uint32 skippedCharacters = 0;
        float scalingFactor = text.ScalingFactor();
        krys_debug_assert(scalingFactor);
        float scaledHeight = scaledFont.MetricsOfPrimaryFont().Height() / scalingFactor;

        for (uint32 i = 0; i < length; ++characterIndex)
        {
          auto decoded = krys::text::DecodeUTF8(Span<const char8>(view.data() + i, view.size() - i));
          char32 currentCharacter = decoded.CodePoint;
          uint32 characterLength = decoded.Length;
          if (currentCharacter == U' ' && !preserveWhiteSpace && (!lastCharacter || lastCharacter == U' '))
          {
            if (data.ProcessRenderer)
            {
              textMetricsValues.push_back(SVGTextMetrics(SVGTextMetrics::MetricsType::SkippedSpaceMetrics));
            }
            ++skippedCharacters;
            i += characterLength;
            continue;
          }

          if ((valueListPosition + characterIndex - skippedCharacters + 1) == defaultPosition)
          {
            attributes->CharacterDataMap()[i + 1] = characterData;
          }

          float width = scaledFont.WidthForTextUsingSimplifiedMeasuring(view.substr(i, characterLength),
                                                                        TextDirection::LTR);
          float scaledWidth = width / scalingFactor;
          textMetricsValues.push_back(SVGTextMetrics(characterLength, scaledWidth, scaledHeight));
          lastCharacter = currentCharacter;
          i += characterLength;
        }

        return std::tuple {valueListPosition + characterIndex - skippedCharacters, lastCharacter};
      }
    }

    if (_isComplexText)
    {
      ComplexTextController iterator(scaledFont, _run, true);
      return MeasureTextRendererWithIterator(iterator, text, data, state);
    }

    WidthIterator iterator(scaledFont, _run);
    return MeasureTextRendererWithIterator(iterator, text, data, state);
  }

  template <typename TIterator>
  std::tuple<uint32, char32>
    SVGTextMetricsBuilder::MeasureTextRendererWithIterator(TIterator &iterator, RenderSVGInlineText &text,
                                                           const MeasureTextData &data,
                                                           std::tuple<uint32, char32> state) noexcept
  {
    auto [valueListPosition, lastCharacter] = state;
    bool preserveWhiteSpace = text.Style().GetWhiteSpaceCollapse() == WhiteSpaceCollapse::Preserve;
    auto *attributes = text.LayoutAttributes();
    auto &textMetricsValues = attributes->TextMetricsValues();
    // The positioning data is indexed by character, while the metrics cover byte ranges.
    uint32 characterIndex = 0;
    uint32 skippedCharacters = 0;
    while (Advance(iterator))
    {
      char32 currentCharacter = krys::text::CodePointAt(_run.Text(), _textPosition);
      uint32 numberOfCharacters =
        NumberOfCodePoints(_run.Text().substr(_textPosition, _currentMetrics.Length()));
      if (currentCharacter == U' ' && !preserveWhiteSpace && (!lastCharacter || lastCharacter == U' '))
      {
        if (data.ProcessRenderer)
        {
          textMetricsValues.push_back(SVGTextMetrics(SVGTextMetrics::MetricsType::SkippedSpaceMetrics));
        }
        skippedCharacters += numberOfCharacters;
        characterIndex += numberOfCharacters;
        continue;
      }

      if (data.ProcessRenderer)
      {
        if (data.AllCharactersMap)
        {
          auto it = data.AllCharactersMap->find(valueListPosition + characterIndex - skippedCharacters + 1);
          if (it != data.AllCharactersMap->end())
          {
            attributes->CharacterDataMap()[_textPosition + 1] = it->second;
          }
        }
        textMetricsValues.push_back(_currentMetrics);
      }

      characterIndex += numberOfCharacters;
      lastCharacter = currentCharacter;
    }

    return std::tuple {valueListPosition + characterIndex - skippedCharacters, lastCharacter};
  }

  void SVGTextMetricsBuilder::WalkTree(RenderElement &start, RenderSVGInlineText *stopAtLeaf,
                                       MeasureTextData &data) noexcept
  {
    uint32 valueListPosition = 0;
    char32 lastCharacter = 0;
    CheckedPtr<RenderObject> child = ShareCheckedPtr(start.FirstChild());
    while (child)
    {
      if (auto *text = DynamicDowncast<RenderSVGInlineText>(*child))
      {
        data.ProcessRenderer = !stopAtLeaf || stopAtLeaf == text;
        std::tie(valueListPosition, lastCharacter) =
          MeasureTextRenderer(*text, data, std::tuple {valueListPosition, lastCharacter});
        if (stopAtLeaf && stopAtLeaf == text)
        {
          return;
        }
      }
      else if (auto *renderer = DynamicDowncast<RenderSVGInline>(*child))
      {
        // Visit children of text content elements.
        if (auto *inlineChild = renderer->FirstChild())
        {
          child = ShareCheckedPtr(inlineChild);
          continue;
        }
      }
      child = ShareCheckedPtr(child->NextInPreOrderAfterChildren(&start));
    }
  }

  void SVGTextMetricsBuilder::MeasureTextRenderer(RenderSVGText &textRoot,
                                                  RenderSVGInlineText *stopAtLeaf) noexcept
  {
    MeasureTextData data(nullptr);
    WalkTree(textRoot, stopAtLeaf, data);
  }

  void SVGTextMetricsBuilder::BuildMetricsAndLayoutAttributes(RenderSVGText &textRoot,
                                                              RenderSVGInlineText *stopAtLeaf,
                                                              SVGCharacterDataMap &allCharactersMap) noexcept
  {
    MeasureTextData data(&allCharactersMap);
    WalkTree(textRoot, stopAtLeaf, data);
  }
}
