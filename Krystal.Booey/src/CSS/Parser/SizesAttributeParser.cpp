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
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::css
{
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
    return mq::MediaQueryEvaluator {mq::ScreenAtom(), *_document, &style}.Evaluate(mediaCondition);
  }

  bool SizesAttributeParser::Parse(TokenRange range, const ParserContext &context) noexcept
  {
    // 1. Let unparsed sizes list be the result of parsing a comma-separated list of component values from the
    //    value of element's sizes attribute (or the empty string, if the attribute is absent).
    // 2. Let size be null.
    // 3. For each unparsed size in unparsed sizes list:
    // Split on a comma token and parse the result tokens as (media-condition, length) pairs
    while (!range.IsAtEnd())
    {
      // 3a. Remove all consecutive <whitespace-token>s from the end of unparsed size. If unparsed size is now
      //     empty, then that is a parse error; continue.
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

      // 3b. If the last component value in unparsed size is a valid non-negative <source-size-value>, then
      // set
      //     size to its value and remove the component value from unparsed size. Any CSS function other than
      //     the math functions is invalid. Otherwise, there is a parse error; continue.
      auto length = CalculateLengthInPixels(lengthTokenStart.RangeUntil(lengthTokenEnd));
      if (!length)
      {
        continue;
      }

      // 3c. If size is auto, and img is not null, and img allows auto-sizes:
      // (There are no auto sizes: auto isn't a valid size.)
      // 3d. Remove all consecutive <whitespace-token>s from the end of unparsed size. If unparsed size is now
      //     empty:
      // 3d.i. If this was not the last item in unparsed sizes list, that is a parse error.
      // 3d.ii. If size is not auto, then return size. Otherwise, continue.
      // (An empty media condition matches.)
      // 3e. Parse the remaining component values in unparsed size as a <media-condition>. If it does not
      // parse
      //     correctly, or it does parse correctly but the <media-condition> evaluates to false, continue.
      auto mediaCondition =
        mq::MediaQueryParser::ParseCondition(mediaConditionStart.RangeUntil(lengthTokenStart), context);
      if (!mediaCondition)
      {
        continue;
      }
      bool matches = MediaConditionMatches(*mediaCondition);
      mq::MediaQueryEvaluator evaluator {mq::ScreenAtom()};
      if (evaluator.CollectDynamicDependencies(*mediaCondition) != mq::MediaQueryDynamicDependency::None)
      {
        _dynamicMediaQueryResults.push_back({mq::MediaQueryList {*mediaCondition}, matches});
      }
      if (!matches)
      {
        continue;
      }
      // 3f. If size is not auto, then return size. Otherwise, continue.
      _length = *length;
      _lengthWasSet = true;
      return true;
    }

    // 4. Return 100vw.
    // (EffectiveSizeDefaultValue.)
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
