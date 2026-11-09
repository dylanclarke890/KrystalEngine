// Copyright 2014 The Chromium Authors. All rights reserved.
// Copyright (C) 2016-2025 Apple Inc. All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//    * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//    * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "Krystal.Booey/CSS/Parser/SizesAttributeParser.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/SizesCalcParser.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Extras.hpp"

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

    ToLengthConversionData conversionData(style, &style, renderer->ParentStyle(), renderer);
    // "sizes" is evaluated as it's parsed, before style is resolved, so the font selector may not be
    // consistent yet. The font-relative units measure the view's fonts without it: its style's font is the
    // standard family, which needs no font selector.
    if (unit == LengthUnit::Ex || unit == LengthUnit::Cap || unit == LengthUnit::Ch || unit == LengthUnit::Ic)
    {
      RefPtr<FontSelector> fontSelector = ShareRefPtr(style.GetFontCascade().GetFontSelector());
      style.GetFontCascade().Update(nullptr);
      auto result = ClampTo<float>(style::ComputeNonCalcLengthDouble(value, unit, conversionData));
      style.GetFontCascade().Update(krys::move(fontSelector));
      return result;
    }
    return ClampTo<float>(style::ComputeNonCalcLengthDouble(value, unit, conversionData));
  }

  SizesAttributeParser::SizesAttributeParser(CSSOMStringView attribute,
                                             const dom::Document &document) noexcept
      : _document(CreateWeakPtr(&document))
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
