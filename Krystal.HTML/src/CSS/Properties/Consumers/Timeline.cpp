#include "Krystal.HTML/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenizer.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParsing.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSScrollValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"
#include "Krystal.HTML/CSS/Values/CSSViewValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  bool IsAnimationRangeKeyword(CSSValueId id) noexcept
  {
    return IdentMatches<CSSValueId::Normal, CSSValueId::Cover, CSSValueId::Contain, CSSValueId::Entry,
                        CSSValueId::Exit, CSSValueId::EntryCrossing, CSSValueId::ExitCrossing>(id);
  }

  RefPtr<CSSValue> ConsumeAnimationTimelineScroll(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept
  {
    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // <scroller> = root | nearest | self
    // <axis> = block | inline | x | y
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation

    if (tokens.Peek().Type() != CSSTokenType::Function || tokens.Peek().FunctionId() != CSSValueId::Scroll)
    {
      return nullptr;
    }

    auto args = ConsumeFunction(tokens);

    if (args.IsAtEnd())
    {
      return CSSScrollValue::Create(nullptr, nullptr);
    }

    auto scroller = CSSPropertyParsing::ConsumeScroller(args);
    auto axis = CSSPropertyParsing::ConsumeAxis(args);

    // Try <scroller> again since the order of <scroller> and <axis> is not guaranteed.
    if (!scroller)
    {
      scroller = CSSPropertyParsing::ConsumeScroller(args);
    }

    // If there are values left to consume, these are not valid <scroller> or <axis> and the function is
    // invalid.
    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSScrollValue::Create(Krys::Move(scroller), Krys::Move(axis));
  }

  RefPtr<CSSValue> ConsumeAnimationTimelineView(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // <axis> = block | inline | x | y
    // <'view-timeline-inset'> = [ [ auto | <length-percentage> ]{1,2} ]#
    // https://drafts.csswg.org/scroll-animations-1/#view-notation

    if (tokens.Peek().Type() != CSSTokenType::Function || tokens.Peek().FunctionId() != CSSValueId::View)
    {
      return nullptr;
    }

    auto args = ConsumeFunction(tokens);

    if (!args.IsAtEnd())
    {
      return CSSViewValue::Create();
    }

    auto axis = CSSPropertyParsing::ConsumeAxis(args);
    auto startInset = CSSPropertyParsing::ConsumeSingleViewTimelineInset(args, state);
    auto endInset = CSSPropertyParsing::ConsumeSingleViewTimelineInset(args, state);

    // Try <axis> again since the order of <axis> and <'view-timeline-inset'> is not guaranteed.
    if (!axis)
    {
      axis = CSSPropertyParsing::ConsumeAxis(args);
    }

    // If there are values left to consume, these are not valid <axis> or <'view-timeline-inset'> and the
    // function is invalid.
    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSViewValue::Create(Krys::Move(axis), Krys::Move(startInset), Krys::Move(endInset));
  }

  RefPtr<CSSValue> ConsumeSingleViewTimelineInsetItem(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept
  {
    // <single-view-timeline-inset-item-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset

    auto startInset = CSSPropertyParsing::ConsumeSingleViewTimelineInset(tokens, state);
    if (!startInset)
    {
      return nullptr;
    }

    if (auto endInset = CSSPropertyParsing::ConsumeSingleViewTimelineInset(tokens, state))
    {
      if (endInset != startInset)
      {
        return CSSValuePair::CreateNonCoalescing(Krys::Move(startInset), Krys::Move(endInset));
      }
    }

    return startInset;
  }

  RefPtr<CSSValue> ParseSingleViewTimelineInsetItem(const CSSOMString &string,
                                                    const CSSParserContext &context) noexcept
  {
    auto inputStream = CSSInputStream(CSSOMString(string));
    auto tokenizer = CSSTokenizer(inputStream);
    auto tokens = tokenizer.TokenRange();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = CSSPropertyParserState {.Context = context};
    auto result = ConsumeSingleViewTimelineInsetItem(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRange(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                               SingleAnimationRangeType type) noexcept
  {
    // <'animation-tokens-{start|end}'> = normal | <length-percentage> | <timeline-tokens-name>
    // <length-percentage>? https://drafts.csswg.org/scroll-animations-1/#propdef-animation-tokens-start

    auto IsDefault = [&](auto &value)
    {
      if (!value.IsPercentage() || value.IsCalculated())
      {
        return false;
      }

      auto percentageValue = value.ResolveAsPercentageNoConversionDataRequired();
      if (type == SingleAnimationRangeType::Start)
      {
        return percentageValue == 0;
      }

      return percentageValue == 100;
    };

    if (auto name = ConsumeIdent(tokens))
    {
      if (name->ValueId() == CSSValueId::Normal)
      {
        return name;
      }

      if (!IsAnimationRangeKeyword(name->ValueId()))
      {
        return nullptr;
      }

      if (auto offset = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state))
      {
        if (IsDefault(*offset))
        {
          return name;
        }

        return CSSValuePair::CreateNonCoalescing(Krys::Move(name), Krys::Move(offset));
      }

      return name;
    }

    return CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRangeStart(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::Start);
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRangeEnd(CSSTokenRange &tokens,
                                                  CSSPropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::End);
  }

  RefPtr<CSSValue> ParseSingleAnimationRange(const CSSOMString &string, const CSSParserContext &context,
                                             SingleAnimationRangeType type) noexcept
  {
    auto inputStream = CSSInputStream(CSSOMString(string));
    auto tokenizer = CSSTokenizer(inputStream);
    auto tokens = tokenizer.TokenRange();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = CSSPropertyParserState {.Context = context};
    auto result = ConsumeSingleAnimationRange(tokens, state, type);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }
}