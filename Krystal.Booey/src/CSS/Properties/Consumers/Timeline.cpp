#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"
#include "Krystal.Booey/CSS/Values/CSSViewValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  bool IsAnimationRangeKeyword(ValueId id) noexcept
  {
    return IdentMatches<ValueId::Normal, ValueId::Cover, ValueId::Contain, ValueId::Entry,
                        ValueId::Exit, ValueId::EntryCrossing, ValueId::ExitCrossing>(id);
  }

  RefPtr<CSSValue> ConsumeAnimationTimelineScroll(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <scroll()> = scroll( [ <scroller> || <axis> ]? )
    // <scroller> = root | nearest | self
    // <axis> = block | inline | x | y
    // https://drafts.csswg.org/scroll-animations-1/#scroll-notation

    if (tokens.Peek().Type() != TokenType::Function || tokens.Peek().FunctionId() != ValueId::Scroll)
    {
      return nullptr;
    }

    auto args = ConsumeFunction(tokens);

    if (args.IsAtEnd())
    {
      return CSSScrollValue::Create(nullptr, nullptr);
    }

    auto scroller = PropertyParsing::ConsumeScroller(args);
    auto axis = PropertyParsing::ConsumeAxis(args);

    // Try <scroller> again since the order of <scroller> and <axis> is not guaranteed.
    if (!scroller)
    {
      scroller = PropertyParsing::ConsumeScroller(args);
    }

    // If there are values left to consume, these are not valid <scroller> or <axis> and the function is
    // invalid.
    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSScrollValue::Create(krys::move(scroller), krys::move(axis));
  }

  RefPtr<CSSValue> ConsumeAnimationTimelineView(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <view()> = view( [ <axis> || <'view-timeline-inset'> ]? )
    // <axis> = block | inline | x | y
    // <'view-timeline-inset'> = [ [ auto | <length-percentage> ]{1,2} ]#
    // https://drafts.csswg.org/scroll-animations-1/#view-notation

    if (tokens.Peek().Type() != TokenType::Function || tokens.Peek().FunctionId() != ValueId::View)
    {
      return nullptr;
    }

    auto args = ConsumeFunction(tokens);

    if (!args.IsAtEnd())
    {
      return CSSViewValue::Create();
    }

    auto axis = PropertyParsing::ConsumeAxis(args);
    auto startInset = PropertyParsing::ConsumeSingleViewTimelineInset(args, state);
    auto endInset = PropertyParsing::ConsumeSingleViewTimelineInset(args, state);

    // Try <axis> again since the order of <axis> and <'view-timeline-inset'> is not guaranteed.
    if (!axis)
    {
      axis = PropertyParsing::ConsumeAxis(args);
    }

    // If there are values left to consume, these are not valid <axis> or <'view-timeline-inset'> and the
    // function is invalid.
    if (!args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSViewValue::Create(krys::move(axis), krys::move(startInset), krys::move(endInset));
  }

  RefPtr<CSSValue> ConsumeSingleViewTimelineInsetItem(TokenRange &tokens,
                                                      PropertyParserState &state) noexcept
  {
    // <single-view-timeline-inset-item-item> = <single-view-timeline-inset>{1,2}
    // https://drafts.csswg.org/scroll-animations-1/#propdef-view-timeline-inset

    auto startInset = PropertyParsing::ConsumeSingleViewTimelineInset(tokens, state);
    if (!startInset)
    {
      return nullptr;
    }

    if (auto endInset = PropertyParsing::ConsumeSingleViewTimelineInset(tokens, state))
    {
      if (endInset != startInset)
      {
        return CSSValuePair::CreateNonCoalescing(krys::move(startInset), krys::move(endInset));
      }
    }

    return startInset;
  }

  RefPtr<CSSValue> ParseSingleViewTimelineInsetItem(const CSSOMString &string,
                                                    const ParserContext &context) noexcept
  {
    auto inputStream = InputStream(CSSOMString(string));
    auto tokenizer = Tokenizer(inputStream);
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
    auto result = ConsumeSingleViewTimelineInsetItem(tokens, state);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return result;
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRange(TokenRange &tokens, PropertyParserState &state,
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
      if (name->ValueId() == ValueId::Normal)
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

        return CSSValuePair::CreateNonCoalescing(krys::move(name), krys::move(offset));
      }

      return name;
    }

    return CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRangeStart(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::Start);
  }

  RefPtr<CSSValue> ConsumeSingleAnimationRangeEnd(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::End);
  }

  RefPtr<CSSValue> ParseSingleAnimationRange(const CSSOMString &string, const ParserContext &context,
                                             SingleAnimationRangeType type) noexcept
  {
    auto inputStream = InputStream(CSSOMString(string));
    auto tokenizer = Tokenizer(inputStream);
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
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