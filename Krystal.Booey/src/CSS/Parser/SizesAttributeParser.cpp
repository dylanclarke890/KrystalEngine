#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/SizesCalcParser.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::css
{
  namespace
  {
    KRYS_NODISCARD const CSSOMStringAtom &ScreenAtom() noexcept
    {
      static const CSSOMStringAtom screen {u8"screen"};
      return screen;
    }
  }

  float SizesAttributeParser::ComputeLength(double value, LengthUnit unit,
                                            const dom::Document &document) noexcept
  {
    auto *renderer = document.GetRenderView();
    if (!renderer)
    {
      return 0;
    }
    auto &style = renderer->Style();

    // FIXME: Font-relative units (ex, cap, ch, ic) resolve against the view's style before its fonts are
    // consistent; clear the font selector while resolving them, as upstream does.
    ToLengthConversionData conversionData(style, &style, nullptr, renderer);
    return ClampTo<float>(style::ComputeNonCalcLengthDouble(value, unit, conversionData));
  }

  SizesAttributeParser::SizesAttributeParser(CSSOMStringView attribute,
                                             const dom::Document &document) noexcept
      : _document(WeakPtr<const dom::Document>::WithRef(document))
  {
    InputStream inputStream {CSSOMString(attribute)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    _isValid = Parse(tokenizer.Tokens(), ParserContext(document));
  }

  float SizesAttributeParser::Length() const noexcept
  {
    if (_isValid)
    {
      return EffectiveSize();
    }
    return EffectiveSizeDefaultValue();
  }

  Maybe<float> SizesAttributeParser::CalculateLengthInPixels(TokenRange range) const noexcept
  {
    const Token &startToken = range.Peek();
    TokenType type = startToken.Type();
    if (type == TokenType::Dimension)
    {
      auto lengthUnit = ToLengthUnit(ParseUnitType(startToken.Unit()));
      if (!lengthUnit)
      {
        return null;
      }
      float result = ComputeLength(startToken.NumericValue(), *lengthUnit, *_document);
      if (result >= 0)
      {
        return result;
      }
    }
    else if (type == TokenType::Function)
    {
      SizesCalcParser calcParser(range, *_document);
      if (!calcParser.IsValid())
      {
        return null;
      }
      return calcParser.Result();
    }
    else if (type == TokenType::Number && !startToken.NumericValue())
    {
      return 0.f;
    }

    return null;
  }

  bool SizesAttributeParser::MediaConditionMatches(const mq::MediaQuery &mediaCondition) const noexcept
  {
    // A Media Condition cannot have a media type other than screen.
    auto *renderer = _document->GetRenderView();
    if (!renderer)
    {
      return false;
    }
    auto &style = renderer->Style();
    return mq::MediaQueryEvaluator {ScreenAtom(), *_document, &style}.Evaluate(mediaCondition);
  }

  bool SizesAttributeParser::Parse(TokenRange range, const ParserContext &context) noexcept
  {
    // Split on a comma token and parse the result tokens as (media-condition, length) pairs
    while (!range.IsAtEnd())
    {
      auto mediaConditionStart = range;
      // The length is the last component value before the comma which isn't whitespace or a comment
      auto lengthTokenStart = range;
      auto lengthTokenEnd = range;
      while (!range.IsAtEnd() && range.Peek().Type() != TokenType::Comma)
      {
        lengthTokenStart = range;
        range.DiscardComponentValue();
        lengthTokenEnd = range;
        range.DiscardWhitespace();
      }
      range.Discard();

      auto length = CalculateLengthInPixels(lengthTokenStart.RangeUntil(lengthTokenEnd));
      if (!length)
      {
        continue;
      }
      auto mediaCondition =
        mq::MediaQueryParser::ParseCondition(mediaConditionStart.RangeUntil(lengthTokenStart), context);
      if (!mediaCondition)
      {
        continue;
      }
      bool matches = MediaConditionMatches(*mediaCondition);
      mq::MediaQueryEvaluator evaluator {ScreenAtom()};
      if (evaluator.CollectDynamicDependencies(*mediaCondition) != mq::MediaQueryDynamicDependency::None)
      {
        _dynamicMediaQueryResults.push_back({mq::MediaQueryList {*mediaCondition}, matches});
      }
      if (!matches)
      {
        continue;
      }
      _length = *length;
      _lengthWasSet = true;
      return true;
    }
    return false;
  }

  float SizesAttributeParser::EffectiveSize() const noexcept
  {
    if (_lengthWasSet)
    {
      return _length;
    }
    return EffectiveSizeDefaultValue();
  }

  float SizesAttributeParser::EffectiveSizeDefaultValue() const noexcept
  {
    auto *renderer = _document->GetRenderView();
    if (!renderer)
    {
      return 0;
    }
    auto &style = renderer->Style();
    // The default is truncated to whole pixels.
    return static_cast<float>(static_cast<uint32>(ClampTo<float>(
      style::ComputeNonCalcLengthDouble(100.0, LengthUnit::Vw, {style, &style, nullptr, renderer}))));
  }
}
