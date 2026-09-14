#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ScrollValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/ViewValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  bool IsAnimationRangeKeyword(ValueId id) noexcept
  {
    return IdentMatches<ValueId::Normal, ValueId::Cover, ValueId::Contain, ValueId::Entry,
                        ValueId::Exit, ValueId::EntryCrossing, ValueId::ExitCrossing>(id);
  }

  RefPtr<Value> ConsumeAnimationTimelineScroll(TokenRange &tokens, PropertyParserState &state) noexcept
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
      return ScrollValue::Create(nullptr, nullptr);
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

    return ScrollValue::Create(krys::move(scroller), krys::move(axis));
  }

  RefPtr<Value> ConsumeAnimationTimelineView(TokenRange &tokens, PropertyParserState &state) noexcept
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
      return ViewValue::Create();
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

    return ViewValue::Create(krys::move(axis), krys::move(startInset), krys::move(endInset));
  }

  RefPtr<Value> ConsumeSingleViewTimelineInsetItem(TokenRange &tokens,
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
        return ValuePair::CreateNonCoalescing(krys::move(startInset), krys::move(endInset));
      }
    }

    return startInset;
  }

  RefPtr<Value> ParseSingleViewTimelineInsetItem(const CSSOMString &string,
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

  RefPtr<Value> ConsumeSingleAnimationRange(TokenRange &tokens, PropertyParserState &state,
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

      if (auto offset = PrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state))
      {
        if (IsDefault(*offset))
        {
          return name;
        }

        return ValuePair::CreateNonCoalescing(krys::move(name), krys::move(offset));
      }

      return name;
    }

    return PrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
  }

  RefPtr<Value> ConsumeSingleAnimationRangeStart(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::Start);
  }

  RefPtr<Value> ConsumeSingleAnimationRangeEnd(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    return ConsumeSingleAnimationRange(tokens, state, SingleAnimationRangeType::End);
  }

  RefPtr<Value> ParseSingleAnimationRange(const CSSOMString &string, const ParserContext &context,
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