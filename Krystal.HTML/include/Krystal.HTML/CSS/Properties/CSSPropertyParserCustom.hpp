#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/Types/ParsedCSSPropertyList.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Align.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AnglePercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Animation.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Box.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Content.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Display.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/FlexDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/FrequencyDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserResult.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthand.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthandFunctions.hpp"
#include "Krystal.HTML/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"

namespace Krys::HTML
{
  struct BorderShorthandComponents
  {
    RefPtr<CSSValue> Width;
    RefPtr<CSSValue> Style;
    RefPtr<CSSValue> Color;
  };

  KRYS_NODISCARD inline Maybe<BorderShorthandComponents>
    ConsumeBorderShorthandComponents(CSSTokenRange &range, CSSPropertyParserState &state) noexcept
  {
    BorderShorthandComponents components {};

    while (!components.Width || !components.Style || !components.Color)
    {
      if (!components.Width)
      {
        components.Width =
          CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyId::BorderLeftWidth, state);
        if (components.Width)
        {
          continue;
        }
      }
      if (!components.Style)
      {
        components.Style =
          CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyId::BorderLeftStyle, state);
        if (components.Style)
          continue;
      }
      if (!components.Color)
      {
        components.Color =
          CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyId::BorderLeftColor, state);
        if (components.Color)
          continue;
      }
      break;
    }

    if (!components.Width && !components.Style && !components.Color)
    {
      return {};
    }

    if (!range.IsAtEnd())
    {
      return {};
    }

    return components;
  }

  KRYS_NODISCARD constexpr CSSValueId MapFromPageBreakBetween(CSSValueId value) noexcept
  {
    if (value == CSSValueId::Always)
    {
      return CSSValueId::Page;
    }

    if (value == CSSValueId::Auto || value == CSSValueId::Avoid || value == CSSValueId::Left
        || value == CSSValueId::Right)
    {
      return value;
    }

    return CSSValueId::Invalid;
  }

  KRYS_NODISCARD constexpr CSSValueId MapFromColumnBreakBetween(CSSValueId value) noexcept
  {
    if (value == CSSValueId::Always)
    {
      return CSSValueId::Column;
    }

    if (value == CSSValueId::Auto)
    {
      return value;
    }

    if (value == CSSValueId::Avoid)
    {
      return CSSValueId::AvoidColumn;
    }

    return CSSValueId::Invalid;
  }

  KRYS_NODISCARD constexpr CSSValueId MapFromColumnRegionOrPageBreakInside(CSSValueId value) noexcept
  {
    if (value == CSSValueId::Auto || value == CSSValueId::Avoid)
    {
      return value;
    }

    return CSSValueId::Invalid;
  }

  class PropertyParserCustom
  {
  public:
    KRYS_NODISCARD static bool
      ConsumeStandardSpaceSeparatedShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                             const CSSPropertyShorthand &shorthand,
                                             CSSPropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() <= 6); // Existing shorthands have at most 6 longhands.

      Array<RefPtr<CSSValue>, 6> longhands;
      auto shorthandProperties = shorthand.Properties();

      do
      {
        bool foundLonghand = false;
        for (size_t i = 0uz; !foundLonghand && i < shorthand.Properties().size(); ++i)
        {
          if (longhands[i])
          {
            continue;
          }

          longhands[i] =
            CSSPropertyParsing::ParseStylePropertyLonghand(tokens, shorthandProperties[i], state);
          if (longhands[i])
          {
            foundLonghand = true;
          }
        }

        if (!foundLonghand)
        {
          return false;
        }
      } while (!tokens.IsAtEnd());

      for (size_t i = 0uz; i < shorthand.Size(); ++i)
      {
        result.AddPropertyForCurrentShorthand(state, shorthandProperties[i], Krys::Move(longhands[i]));
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeCoalescingPairShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() == 2uz);

      auto longhands = shorthand.Properties();
      auto start = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!start)
      {
        return false;
      }

      auto end = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      auto endImplicit = IsImplicit(!end);

      if (endImplicit)
      {
        end = start;
      }

      result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(start));
      result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(end), endImplicit);

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeCoalescingQuadShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() == 4);

      auto longhands = shorthand.Properties();
      auto top = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!top)
      {
        return false;
      }

      RefPtr right = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      RefPtr<CSSValue> bottom;
      RefPtr<CSSValue> left;
      if (right)
      {
        bottom = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[2], state);
        if (bottom)
        {
          left = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[3], state);
        }
      }

      auto rightImplicit = IsImplicit(!right);
      auto bottomImplicit = IsImplicit(!bottom);
      auto leftImplicit = IsImplicit(!left);

      if (rightImplicit)
      {
        right = top;
      }

      if (bottomImplicit)
      {
        bottom = top;
      }

      if (leftImplicit)
      {
        left = right;
      }

      result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(top));
      result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(right), rightImplicit);
      result.AddPropertyForCurrentShorthand(state, longhands[2], Krys::Move(bottom), bottomImplicit);
      result.AddPropertyForCurrentShorthand(state, longhands[3], Krys::Move(left), leftImplicit);

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeBorderShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                      const CSSPropertyShorthand &shorthand,
                                                      CSSPropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderWidth,
                                                   Krys::Move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderStyle,
                                                   Krys::Move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderColor,
                                                   Krys::Move(components->Color), state.Important);

      for (auto longhand : BorderImageShorthand())
      {
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr);
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderInlineShorthand(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            const CSSPropertyShorthand &shorthand,
                                                            CSSPropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderInlineWidth,
                                                   Krys::Move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderInlineStyle,
                                                   Krys::Move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderInlineColor,
                                                   Krys::Move(components->Color), state.Important);

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderImageShorthand(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state,
                                                           const CSSPropertyShorthand &shorthand,
                                                           CSSPropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderImageComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderImageSource,
                                            Krys::Move(components->Source));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderImageSlice,
                                            Krys::Move(components->Slice));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderImageWidth,
                                            Krys::Move(components->Width));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderImageOutset,
                                            Krys::Move(components->Outset));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderImageRepeat,
                                            Krys::Move(components->Repeat));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderBlockShorthand(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state,
                                                           const CSSPropertyShorthand &shorthand,
                                                           CSSPropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderBlockWidth,
                                                   Krys::Move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderBlockStyle,
                                                   Krys::Move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, CSSPropertyId::BorderBlockColor,
                                                   Krys::Move(components->Color), state.Important);
      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderRadiusShorthand(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            const CSSPropertyShorthand &shorthand,
                                                            CSSPropertyParserResult &result) noexcept
    {
      auto borderRadius = ConsumeUnresolvedBorderRadius(tokens, state);
      if (!borderRadius.has_value())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::BorderTopLeftRadius,
        WebCore::CSS::createCSSValue(state.pool, borderRadius->TopLeft()));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::BorderTopRightRadius,
        WebCore::CSS::createCSSValue(state.pool, borderRadius->TopRight()));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::BorderBottomRightRadius,
        WebCore::CSS::createCSSValue(state.pool, borderRadius->BottomRight()));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::BorderBottomLeftRadius,
        WebCore::CSS::createCSSValue(state.pool, borderRadius->BottomLeft()));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeAnimationShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      auto ConsumeAnimationValueForShorthand = [&](CSSPropertyId property) -> RefPtr<CSSValue>
      {
        switch (property)
        {
          case CSSPropertyId::AnimationDelay:
          {
            return CSSPrimitiveValueResolver<Time<>>::ConsumeAndResolve(tokens, state);
          }
          case CSSPropertyId::AnimationDirection:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationDirection(tokens);
          }
          case CSSPropertyId::AnimationDuration:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationDuration(tokens, state);
          }
          case CSSPropertyId::AnimationFillMode:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationFillMode(tokens);
          }
          case CSSPropertyId::AnimationIterationCount:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationIterationCount(tokens, state);
          }
          case CSSPropertyId::AnimationName:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationName(tokens, state);
          }
          case CSSPropertyId::AnimationPlayState:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationPlayState(tokens);
          }
          case CSSPropertyId::AnimationComposition:
          {
            return CSSPropertyParsing::ConsumeSingleAnimationComposition(tokens);
          }
          case CSSPropertyId::AnimationTimeline:
          case CSSPropertyId::AnimationRangeStart:
          case CSSPropertyId::AnimationRangeEnd:
          {
            return nullptr; // reset-only longhands
          }
          case CSSPropertyId::AnimationTimingFunction:
          {
            return ConsumeEasingFunction(tokens, state);
          }
          case CSSPropertyId::TransitionBehavior:
          {
            return CSSPropertyParsing::ConsumeTransitionBehaviorValue(tokens);
          }
          default:
          {
            assert(false);
            return nullptr;
          }
        }
      };

      auto shorthandProperties = shorthand.Properties();

      const size_t longhandCount = shorthand.Size();
      const size_t maxLonghandCount = 11;
      Array<CSSValueListBuilder, maxLonghandCount> longhands;
      assert(longhandCount <= maxLonghandCount);

      auto isResetOnlyLonghand = [](CSSPropertyId longhand)
      {
        switch (longhand)
        {
          case CSSPropertyId::AnimationTimeline:
          case CSSPropertyId::AnimationRangeStart:
          case CSSPropertyId::AnimationRangeEnd:
          {
            return true;
          }
          default:
          {
            return false;
          }
        }
      };

      do
      {
        Array<bool, maxLonghandCount> parsedLonghand = {};
        do
        {
          bool foundProperty = false;
          for (size_t i = 0; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
              continue;

            if (auto value = ConsumeAnimationValueForShorthand(shorthandProperties[i]))
            {
              parsedLonghand[i] = true;
              foundProperty = true;
              longhands[i].push_back(ShareRef(*value));
              break;
            }
          }
          if (!foundProperty)
            return false;
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Comma);

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i] && !isResetOnlyLonghand(shorthandProperties[i]))
          {
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
          }
          parsedLonghand[i] = false;
        }
      } while (ConsumeCommaIncludingWhitespace(tokens));

      for (size_t i = 0; i < longhandCount; ++i)
      {
        auto &list = longhands[i];
        if (list.empty()) // reset-only property
        {
          result.AddPropertyForCurrentShorthand(state, shorthandProperties[i], nullptr);
        }
        else
        {
          result.AddPropertyForCurrentShorthand(state, shorthandProperties[i],
                                                CSSValueList::createCommaSeparated(Krys::Move(list)));
        }
      }

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeTransitionShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      auto isValidTransitionPropertyList = [](CSSPropertyId property, const CSSValueListBuilder &valueList)
      {
        // If there is more than one <single-transition> in the shorthand, and any of the transitions
        // has none as the <single-transition-property>, then the declaration is invalid.
        if (property != CSSPropertyId::TransitionProperty || valueList.size() < 2)
        {
          return true;
        }

        for (auto &value : valueList)
        {
          if (IsValueId(value, CSSValueId::None))
          {
            return false;
          }
        }

        return true;
      };

      auto consumeTransitionValueForShorthand = [&](CSSPropertyId property) -> RefPtr<CSSValue>
      {
        switch (property)
        {
          case CSSPropertyId::TransitionDelay:
          {
            return CSSPrimitiveValueResolver<Time<>>::ConsumeAndResolve(tokens, state);
          }
          case CSSPropertyId::TransitionDuration:
          {
            return CSSPrimitiveValueResolver<Time<NonNegative>>::ConsumeAndResolve(tokens, state);
          }
          case CSSPropertyId::TransitionProperty:
          {
            return consumeSingleTransitionPropertyOrNone(tokens, state);
          }
          case CSSPropertyId::TransitionTimingFunction:
          {
            return consumeEasingFunction(tokens, state);
          }
          case CSSPropertyId::TransitionBehavior:
          {
            return CSSPropertyParsing::ConsumeTransitionBehaviorValue(tokens);
          }
          default:
          {
            assert(false);
            return nullptr;
          }
        }
      };

      auto shorthandProperties = shorthand.Properties();

      const size_t longhandCount = shorthand.Size();
      const size_t maxLonghandCount = 11;
      Array<CSSValueListBuilder, maxLonghandCount> longhands;
      assert(longhandCount <= maxLonghandCount);

      do
      {
        Array<bool, maxLonghandCount> parsedLonghand = {};
        do
        {
          bool foundProperty = false;
          for (size_t i = 0; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
            {
              continue;
            }

            if (auto value = consumeTransitionValueForShorthand(shorthandProperties[i]))
            {
              parsedLonghand[i] = true;
              foundProperty = true;
              longhands[i].push_back(ShareRef(*value));
              break;
            }
          }

          if (!foundProperty)
          {
            return false;
          }
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Comma);

        for (size_t i = 0; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i])
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
          parsedLonghand[i] = false;
        }
      } while (ConsumeCommaIncludingWhitespace(tokens));

      for (size_t i = 0; i < longhandCount; ++i)
      {
        if (!isValidTransitionPropertyList(shorthandProperties[i], longhands[i]))
          return false;
      }

      for (size_t i = 0; i < longhandCount; ++i)
        result.AddPropertyForCurrentShorthand(state, shorthandProperties[i],
                                              CSSValueList::createCommaSeparated(Krys::Move(longhands[i])));

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeBackgroundShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.id() == state.currentProperty);

      auto consumeBackgroundComponent = [&](CSSPropertyId property) -> RefPtr<CSSValue>
      {
        switch (property)
        {
          // background-*
          case CSSPropertyId::BackgroundClip:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundClip(tokens, state);
          }
          case CSSPropertyId::BackgroundBlendMode:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundBlendMode(tokens);
          }
          case CSSPropertyId::BackgroundAttachment:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundAttachment(tokens);
          }
          case CSSPropertyId::BackgroundOrigin:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundOrigin(tokens);
          }
          case CSSPropertyId::BackgroundImage:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundImage(tokens, state);
          }
          case CSSPropertyId::BackgroundRepeat:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundRepeat(tokens, state);
          }
          case CSSPropertyId::BackgroundPositionX:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundPositionX(tokens, state);
          }
          case CSSPropertyId::BackgroundPositionY:
          {
            return CSSPropertyParsing::ConsumeSingleBackgroundPositionY(tokens, state);
          }
          case CSSPropertyId::BackgroundSize:
          {
            return ConsumeSingleBackgroundSize(tokens, state);
          }
          case CSSPropertyId::BackgroundColor:
          {
            return ConsumeColor(tokens, state);
          }

          // mask-*
          case CSSPropertyId::MaskComposite:
          {
            return CSSPropertyParsing::ConsumeSingleMaskComposite(tokens);
          }
          case CSSPropertyId::MaskOrigin:
          {
            return CSSPropertyParsing::ConsumeSingleMaskOrigin(tokens);
          }
          case CSSPropertyId::MaskClip:
          {
            return CSSPropertyParsing::ConsumeSingleMaskClip(tokens);
          }
          case CSSPropertyId::MaskImage:
          {
            return CSSPropertyParsing::ConsumeSingleMaskImage(tokens, state);
          }
          case CSSPropertyId::MaskMode:
          {
            return CSSPropertyParsing::ConsumeSingleMaskMode(tokens);
          }
          case CSSPropertyId::MaskRepeat:
          {
            return CSSPropertyParsing::ConsumeSingleMaskRepeat(tokens, state);
          }
          case CSSPropertyId::MaskSize:
          {
            return ConsumeSingleMaskSize(tokens, state);
          }
          default:
          {
            return nullptr;
          }
        };
      };

      auto shorthandProperties = shorthand.Properties();
      size_t longhandCount = shorthand.Size();

      // mask resets mask-border properties outside of this method.
      if (shorthand.Id() == CSSPropertyId::Mask)
      {
        longhandCount -= MaskBorderShorthand().Size();
      }

      Array<CSSValueListBuilder, 10> longhands;
      assert(longhandCount <= 10);

      do
      {
        Array<bool, 10> parsedLonghand = {};
        bool lastParsedWasPosition = false;
        bool clipIsBorderArea = false;
        RefPtr<CSSValue> originValue;
        do
        {
          bool foundProperty = false;
          for (size_t i = 0; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
              continue;

            RefPtr<CSSValue> value;
            RefPtr<CSSValue> valueY;
            CSSPropertyId property = shorthandProperties[i];

            if (property == CSSPropertyId::BackgroundPositionX)
            {
              // Note: This assumes y properties (for example background-position-y) follow the x properties
              // in the shorthand array.
              auto position = ConsumeBackgroundPositionUnresolved(tokens, state);
              if (!position)
              {
                continue;
              }

              auto [positionX, positionY] = split(Krys::Move(*position));
              value = CSSPositionXValue::create(Krys::Move(positionX));
              valueY = CSSPositionYValue::create(Krys::Move(positionY));
            }
            else if (property == CSSPropertyId::BackgroundSize)
            {
              if (!consumeSlashIncludingWhitespace(tokens))
                continue;
              if (!lastParsedWasPosition)
                return false;
              value = consumeSingleBackgroundSize(tokens, state);
              if (!value)
                return false;
            }
            else if (property == CSSPropertyId::MaskSize)
            {
              if (!consumeSlashIncludingWhitespace(tokens))
                continue;
              if (!lastParsedWasPosition)
                return false;
              value = consumeSingleMaskSize(tokens, state);
              if (!value)
                return false;
            }
            else if (property == CSSPropertyId::BackgroundPositionY)
            {
              continue;
            }
            else
            {
              value = consumeBackgroundComponent(property);
            }
            if (value)
            {
              if (property == CSSPropertyId::BackgroundOrigin || property == CSSPropertyId::MaskOrigin)
                originValue = value;
              else if (property == CSSPropertyId::BackgroundClip)
                clipIsBorderArea = value->ValueId() == CSSValueId::BorderArea;
              parsedLonghand[i] = true;
              foundProperty = true;
              longhands[i].push_back(Krys::Move(value));
              lastParsedWasPosition = valueY != nullptr;
              if (valueY)
              {
                parsedLonghand[i + 1] = true;
                longhands[i + 1].push_back(Krys::Move(valueY));
              }
            }
          }
          if (!foundProperty)
            return false;
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::CommaToken);

        for (size_t i = 0; i < longhandCount; ++i)
        {
          auto property = shorthandProperties[i];
          if (property == CSSPropertyId::BackgroundColor && !tokens.IsAtEnd())
          {
            if (parsedLonghand[i])
              return false; // Colors are only allowed in the last layer.
            continue;
          }
          if ((property == CSSPropertyId::BackgroundClip || property == CSSPropertyId::MaskClip)
              && !parsedLonghand[i] && originValue)
          {
            longhands[i].push_back(Krys::Move(originValue));
            continue;
          }
          if (clipIsBorderArea && (property == CSSPropertyId::BackgroundOrigin) && !parsedLonghand[i])
          {
            longhands[i].push_back(CSSPrimitiveValue::Create(CSSValueId::BorderBox));
            continue;
          }
          if (!parsedLonghand[i])
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
        }
      } while (ConsumeCommaIncludingWhitespace(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      for (size_t i = 0; i < longhandCount; ++i)
      {
        auto property = shorthandProperties[i];
        if (longhands[i].size() == 1)
        {
          result.AddPropertyForCurrentShorthand(state, property, Krys::Move(longhands[i][0]));
        }
        else
        {
          result.AddPropertyForCurrentShorthand(state, property,
                                                CSSValueList::createCommaSeparated(Krys::Move(longhands[i])));
        }
      }
      return true;
    }

    KRYS_NODISCARD static bool ConsumeBackgroundPositionShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.id() == state.currentProperty);

      CSSValueListBuilder x;
      CSSValueListBuilder y;
      do
      {
        auto position = consumeBackgroundPositionUnresolved(range, state);
        if (!position)
          return false;
        auto [positionX, positionY] = split(Krys::Move(*position));
        x.append(CSSPositionXValue::create(Krys::Move(positionX)));
        y.append(CSSPositionYValue::create(Krys::Move(positionY)));
      } while (ConsumeCommaIncludingWhitespace(range));

      if (!tokens.IsAtEnd())
        return false;

      RefPtr<CSSValue> resultX;
      RefPtr<CSSValue> resultY;
      if (x.size() == 1)
      {
        resultX = Krys::Move(x[0]);
        resultY = Krys::Move(y[0]);
      }
      else
      {
        resultX = CSSValueList::createCommaSeparated(Krys::Move(x));
        resultY = CSSValueList::createCommaSeparated(Krys::Move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], resultX.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, longhands[1], resultY.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumeMaskShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      if (!consumeBackgroundShorthand(range, state, shorthand, result))
        return false;
      for (auto longhand : maskBorderShorthand())
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr);
      return true;
    }

    KRYS_NODISCARD static bool ConsumeMaskPositionShorthand(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            const CSSPropertyShorthand &shorthand,
                                                            CSSPropertyParserResult &result) noexcept
    {
      CSSValueListBuilder x;
      CSSValueListBuilder y;
      do
      {
        auto position = consumePositionUnresolved(range, state);
        if (!position)
          return false;
        auto [positionX, positionY] = split(Krys::Move(*position));
        x.append(CSSPositionXValue::create(Krys::Move(positionX)));
        y.append(CSSPositionYValue::create(Krys::Move(positionY)));
      } while (ConsumeCommaIncludingWhitespace(range));

      if (!tokens.IsAtEnd())
        return false;

      RefPtr<CSSValue> resultX;
      RefPtr<CSSValue> resultY;
      if (x.size() == 1)
      {
        resultX = Krys::Move(x[0]);
        resultY = Krys::Move(y[0]);
      }
      else
      {
        resultX = CSSValueList::createCommaSeparated(Krys::Move(x));
        resultY = CSSValueList::createCommaSeparated(Krys::Move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], resultX.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, longhands[1], resultY.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumeOverflowShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                        const CSSPropertyShorthand &shorthand,
                                                        CSSPropertyParserResult &result) noexcept
    {
      CSSValueID xValueID = range.consumeIncludingWhitespace().id();
      if (!CSSPropertyParsing::isKeywordValidForStyleProperty(CSSPropertyOverflowY, xValueID, state))
        return false;

      CSSValueID yValueID;
      if (tokens.IsAtEnd())
      {
        yValueID = xValueID;

        // FIXME: -webkit-paged-x or -webkit-paged-y only apply to overflow-y. If this value has been
        // set using the shorthand, then for now overflow-x will default to auto, but once we implement
        // pagination controls, it should default to hidden. If the overflow-y value is anything but
        // paged-x or paged-y, then overflow-x and overflow-y should have the same value.
        if (xValueID == CSSValueWebkitPagedX || xValueID == CSSValueWebkitPagedY)
          xValueID = CSSValueAuto;
      }
      else
        yValueID = range.consumeIncludingWhitespace().id();

      if (!CSSPropertyParsing::isKeywordValidForStyleProperty(CSSPropertyOverflowY, yValueID, state))
        return false;
      if (!tokens.IsAtEnd())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyOverflowX, CSSPrimitiveValue::create(xValueID));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOverflowY, CSSPrimitiveValue::create(yValueID));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeColumnsShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                       const CSSPropertyShorthand &shorthand,
                                                       CSSPropertyParserResult &result) noexcept
    {
      RefPtr<CSSValue> columnWidth;
      RefPtr<CSSValue> columnCount;

      for (unsigned propertiesParsed = 0; propertiesParsed < 2 && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (range.peek().id() == CSSValueAuto)
        {
          // 'auto' is a valid value for any of the two longhands, and at this point
          // we don't know which one(s) it is meant for. We need to see if there are other values first.
          consumeIdent(range);
        }
        else
        {
          if (!columnWidth
              && (columnWidth =
                    CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyColumnWidth, state)))
            continue;
          if (!columnCount
              && (columnCount =
                    CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyColumnCount, state)))
            continue;
          // If we didn't find at least one match, this is an invalid shorthand and we have to ignore it.
          return false;
        }
      }

      if (!tokens.IsAtEnd())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyColumnWidth, Krys::Move(columnWidth));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyColumnCount, Krys::Move(columnCount));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridItemPositionShorthand(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state,
                                                                const CSSPropertyShorthand &shorthand,
                                                                CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.id() == state.currentProperty);
      assert(shorthand.Size() == 2);

      RefPtr startValue = consumeGridLine(range, state);
      if (!startValue)
        return false;

      RefPtr<CSSValue> endValue;
      if (consumeSlashIncludingWhitespace(range))
      {
        endValue = consumeGridLine(range, state);
        if (!endValue)
          return false;
      }
      else
      {
        endValue = isCustomIdentValue(*startValue) ? startValue : CSSPrimitiveValue::create(CSSValueAuto);
      }
      if (!tokens.IsAtEnd())
        return false;

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], startValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, longhands[1], endValue.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridTemplateShorthand(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            const CSSPropertyShorthand &shorthand,
                                                            CSSPropertyParserResult &result) noexcept
    {
      CSSParserTokenRange rangeCopy = range;
      RefPtr<CSSValue> rowsValue = consumeIdent<CSSValueNone>(range);

      // 1- 'none' case.
      if (rowsValue && tokens.IsAtEnd())
      {
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateRows,
                                              CSSPrimitiveValue::create(CSSValueNone));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateColumns,
                                              CSSPrimitiveValue::create(CSSValueNone));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateAreas,
                                              CSSPrimitiveValue::create(CSSValueNone));
        return true;
      }

      // 2- <grid-template-rows> / <grid-template-columns>
      if (!rowsValue)
        rowsValue = consumeGridTrackList(range, state, GridTemplate);

      if (rowsValue)
      {
        if (!consumeSlashIncludingWhitespace(range))
          return false;
        RefPtr columnsValue = consumeGridTemplatesRowsOrColumns(range, state);
        if (!columnsValue || !tokens.IsAtEnd())
          return false;

        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateRows, rowsValue.releaseNonNull());
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateColumns,
                                              columnsValue.releaseNonNull());
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateAreas,
                                              CSSPrimitiveValue::create(CSSValueNone));
        return true;
      }

      // 3- [ <line-names>? <string> <track-size>? <line-names>? ]+ [ / <track-list> ]?

      range = rangeCopy;

      CSS::GridNamedAreaMap gridAreaMap;
      CSSValueListBuilder templateRows;

      // Persists between loop iterations so we can use the same value for
      // consecutive <line-names> values
      RefPtr<CSSGridLineNamesValue> lineNames;

      do
      {
        // Handle leading <custom-ident>*.
        auto previousLineNames = std::exchange(lineNames, consumeGridLineNames(range, state));
        if (lineNames)
        {
          if (!previousLineNames)
            templateRows.append(lineNames.releaseNonNull());
          else
          {
            Vector<String> combinedLineNames;
            combinedLineNames.append(previousLineNames->names());
            combinedLineNames.append(lineNames->names());
            templateRows.last() = CSSGridLineNamesValue::create(combinedLineNames);
          }
        }

        // Handle a template-area's row.
        auto row = consumeUnresolvedGridTemplateAreasRow(range, state);
        if (!row || !CSS::addRow(gridAreaMap, *row))
          return false;

        // Handle template-rows's track-size.
        if (RefPtr value = consumeGridTrackSize(range, state))
          templateRows.append(value.releaseNonNull());
        else
          templateRows.append(CSSPrimitiveValue::create(CSSValueAuto));

        // This will handle the trailing/leading <custom-ident>* in the grammar.
        lineNames = consumeGridLineNames(range, state);
        if (lineNames)
          templateRows.append(*lineNames);
      } while (!tokens.IsAtEnd()
               && !(tokens.Peek().Type() == DelimiterToken && range.peek().delimiter() == '/'));

      RefPtr<CSSValue> columnsValue;
      if (!tokens.IsAtEnd())
      {
        if (!consumeSlashIncludingWhitespace(range))
          return false;
        columnsValue = consumeGridTrackList(range, state, GridTemplateNoRepeat);
        if (!columnsValue || !tokens.IsAtEnd())
          return false;
      }
      else
      {
        columnsValue = CSSPrimitiveValue::create(CSSValueNone);
      }
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateRows,
                                            CSSValueList::createSpaceSeparated(Krys::Move(templateRows)));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateColumns,
                                            columnsValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateAreas,
                                            CSSGridTemplateAreasValue::create({Krys::Move(gridAreaMap)}));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 6);

      auto consumeImplicitGridAutoFlow = [](CSSParserTokenRange &range,
                                            CSSValueID flowDirection) -> RefPtr<CSSValue>
      {
        // [ auto-flow && dense? ]
        bool autoFlow = consumeIdentRaw<CSSValueAutoFlow>(range).has_value();
        bool dense = consumeIdentRaw<CSSValueDense>(range).has_value();
        if (!autoFlow && (!dense || !consumeIdentRaw<CSSValueAutoFlow>(range)))
          return nullptr;
        if (!dense)
          return CSSValueList::createSpaceSeparated(CSSPrimitiveValue::create(flowDirection));
        if (flowDirection == CSSValueRow)
          return CSSValueList::createSpaceSeparated(CSSPrimitiveValue::create(CSSValueDense));
        return CSSValueList::createSpaceSeparated(CSSPrimitiveValue::create(flowDirection),
                                                  CSSPrimitiveValue::create(CSSValueDense));
      };

      CSSParserTokenRange rangeCopy = range;

      // 1- <grid-template>
      if (consumeGridTemplateShorthand(range, state, gridTemplateShorthand(), result))
      {
        // It can only be specified the explicit or the implicit grid properties in a single grid declaration.
        // The sub-properties not specified are set to their initial value, as normal for shorthands.
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoFlow,
                                              CSSPrimitiveValue::create(CSSValueRow));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoColumns,
                                              CSSPrimitiveValue::create(CSSValueAuto));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoRows,
                                              CSSPrimitiveValue::create(CSSValueAuto));

        return true;
      }

      range = rangeCopy;

      RefPtr<CSSValue> autoColumnsValue;
      RefPtr<CSSValue> autoRowsValue;
      RefPtr<CSSValue> templateRows;
      RefPtr<CSSValue> templateColumns;
      RefPtr<CSSValue> gridAutoFlow;

      if (range.peek().id() == CSSValueAutoFlow || range.peek().id() == CSSValueDense)
      {
        // 2- [ auto-flow && dense? ] <grid-auto-rows>? / <grid-template-columns>
        gridAutoFlow = consumeImplicitGridAutoFlow(range, CSSValueRow);
        if (!gridAutoFlow || tokens.IsAtEnd())
          return false;
        if (consumeSlashIncludingWhitespace(range))
          autoRowsValue = CSSPrimitiveValue::create(CSSValueAuto);
        else
        {
          autoRowsValue = consumeGridTrackList(range, state, GridAuto);
          if (!autoRowsValue)
            return false;
          if (!consumeSlashIncludingWhitespace(range))
            return false;
        }
        if (tokens.IsAtEnd())
          return false;
        templateColumns = consumeGridTemplatesRowsOrColumns(range, state);
        if (!templateColumns)
          return false;
        templateRows = CSSPrimitiveValue::create(CSSValueNone);
        autoColumnsValue = CSSPrimitiveValue::create(CSSValueAuto);
      }
      else
      {
        // 3- <grid-template-rows> / [ auto-flow && dense? ] <grid-auto-columns>?
        templateRows = consumeGridTemplatesRowsOrColumns(range, state);
        if (!templateRows)
          return false;
        if (!consumeSlashIncludingWhitespace(range) || tokens.IsAtEnd())
          return false;
        gridAutoFlow = consumeImplicitGridAutoFlow(range, CSSValueColumn);
        if (!gridAutoFlow)
          return false;
        if (tokens.IsAtEnd())
          autoColumnsValue = CSSPrimitiveValue::create(CSSValueAuto);
        else
        {
          autoColumnsValue = consumeGridTrackList(range, state, GridAuto);
          if (!autoColumnsValue)
            return false;
        }
        templateColumns = CSSPrimitiveValue::create(CSSValueNone);
        autoRowsValue = CSSPrimitiveValue::create(CSSValueAuto);
      }

      if (!tokens.IsAtEnd())
        return false;

      // It can only be specified the explicit or the implicit grid properties in a single grid declaration.
      // The sub-properties not specified are set to their initial value, as normal for shorthands.
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateColumns,
                                            templateColumns.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateRows,
                                            templateRows.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridTemplateAreas,
                                            CSSPrimitiveValue::create(CSSValueNone));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoFlow, gridAutoFlow.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoColumns,
                                            autoColumnsValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridAutoRows, autoRowsValue.releaseNonNull());

      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridAreaShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                        const CSSPropertyShorthand &shorthand,
                                                        CSSPropertyParserResult &result) noexcept
    {
      RefPtr rowStartValue = consumeGridLine(range, state);
      if (!rowStartValue)
        return false;
      RefPtr<CSSValue> columnStartValue;
      RefPtr<CSSValue> rowEndValue;
      RefPtr<CSSValue> columnEndValue;
      if (consumeSlashIncludingWhitespace(range))
      {
        columnStartValue = consumeGridLine(range, state);
        if (!columnStartValue)
          return false;
        if (consumeSlashIncludingWhitespace(range))
        {
          rowEndValue = consumeGridLine(range, state);
          if (!rowEndValue)
            return false;
          if (consumeSlashIncludingWhitespace(range))
          {
            columnEndValue = consumeGridLine(range, state);
            if (!columnEndValue)
              return false;
          }
        }
      }
      if (!tokens.IsAtEnd())
        return false;
      if (!columnStartValue)
        columnStartValue =
          isCustomIdentValue(*rowStartValue) ? rowStartValue : CSSPrimitiveValue::create(CSSValueAuto);
      if (!rowEndValue)
        rowEndValue =
          isCustomIdentValue(*rowStartValue) ? rowStartValue : CSSPrimitiveValue::create(CSSValueAuto);
      if (!columnEndValue)
        columnEndValue =
          isCustomIdentValue(*columnStartValue) ? columnStartValue : CSSPrimitiveValue::create(CSSValueAuto);

      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridRowStart, rowStartValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridColumnStart,
                                            columnStartValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridRowEnd, rowEndValue.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyGridColumnEnd, columnEndValue.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumeAlignShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                     const CSSPropertyShorthand &shorthand,
                                                     CSSPropertyParserResult &result) noexcept
    {
      // Used to implement the rules in CSS Align for the following shorthands:
      //   <'place-content'> https://drafts.csswg.org/css-align/#propdef-place-content
      //   <'place-items'>   https://drafts.csswg.org/css-align/#propdef-place-items
      //   <'place-self'>    https://drafts.csswg.org/css-align/#propdef-place-self
      //   <'gap'>           https://drafts.csswg.org/css-align/#propdef-gap

      assert(shorthand.id() == state.currentProperty);
      assert(shorthand.Size() == 2);
      auto longhands = shorthand.Properties();

      auto rangeCopy = range;

      RefPtr prop1 = CSSPropertyParsing::ParseStylePropertyLonghand(range, longhands[0], state);
      if (!prop1)
        return false;

      // If there are no more tokens, that prop2 should use re-use the original range. This is the equivalent
      // of copying and validating prop1.
      if (tokens.IsAtEnd())
        range = rangeCopy;

      RefPtr prop2 = CSSPropertyParsing::ParseStylePropertyLonghand(range, longhands[1], state);
      if (!prop2 || !tokens.IsAtEnd())
        return false;

      result.AddPropertyForCurrentShorthand(state, longhands[0], prop1.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, longhands[1], prop2.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumeBlockStepShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      // https://drafts.csswg.org/css-rhythm/#block-step
      RefPtr<CSSValue> size;
      RefPtr<CSSValue> insert;
      RefPtr<CSSValue> align;
      RefPtr<CSSValue> round;

      for (unsigned propertiesParsed = 0; propertiesParsed < 4 && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!size && (size = CSSPropertyParsing::consumeBlockStepSize(range, state)))
          continue;
        if (!insert && (insert = CSSPropertyParsing::consumeBlockStepInsert(range)))
          continue;
        if (!align && (align = CSSPropertyParsing::consumeBlockStepAlign(range)))
          continue;
        if (!round && (round = CSSPropertyParsing::consumeBlockStepRound(range)))
          continue;

        // There has to be at least one valid longhand.
        return false;
      }

      if (!tokens.IsAtEnd())
        return false;

      // Fill in default values if one was missing.
      if (!size)
        size = CSSPrimitiveValue::create(CSSValueNone);
      if (!insert)
        insert = CSSPrimitiveValue::create(CSSValueMarginBox);
      if (!align)
        align = CSSPrimitiveValue::create(CSSValueAuto);
      if (!round)
        round = CSSPrimitiveValue::create(CSSValueUp);

      result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockStepSize, Krys::Move(size));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockStepInsert, Krys::Move(insert));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockStepAlign, Krys::Move(align));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockStepRound, Krys::Move(round));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      if (CSSPropertyParserHelpers::isSystemFontShorthand(range.peek().id()))
      {
        auto systemFont = range.consumeIncludingWhitespace().id();
        if (!tokens.IsAtEnd())
          return false;

        // We can't store properties (weight, size, etc.) of the system font here,
        // since those values can change (e.g. accessibility font sizes, or accessibility bold).
        // Parsing (correctly) doesn't re-run in response to updateStyleAfterChangeInEnvironment().
        // Instead, we store sentinel values, later replaced by environment-sensitive values
        // inside Style::BuilderCustom and Style::BuilderConverter.
        result.addPropertyForAllLonghandsOfCurrentShorthand(state, CSSPrimitiveValue::create(systemFont),
                                                            IsImplicit::Yes);
        return true;
      }

      CSSParserTokenRangeGuard guard {range};

      Array<RefPtr<CSSValue>, 7> values;
      auto &fontStyle = values[0];
      auto &fontVariantCaps = values[1];
      auto &fontWeight = values[2];
      auto &fontWidth = values[3];
      auto &fontSize = values[4];
      auto &lineHeight = values[5];
      auto &fontFamily = values[6];

      // Optional font-style, font-variant, font-width and font-weight, in any order.
      for (unsigned i = 0; i < 4 && !tokens.IsAtEnd(); ++i)
      {
        if (consumeIdent<CSSValueNormal>(range))
          continue;
        if (!fontStyle
            && (fontStyle =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontStyle, state)))
          continue;
        if (!fontVariantCaps && (fontVariantCaps = consumeIdent<CSSValueSmallCaps>(range)))
          continue;
        if (!fontWeight
            && (fontWeight =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontWeight, state)))
          continue;
        if (!fontWidth && (fontWidth = CSSPropertyParsing::consumeFontWidthAbsolute(range)))
          continue;
        break;
      }

      if (tokens.IsAtEnd())
        return false;

      fontSize = CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontSize, state);
      if (!fontSize || tokens.IsAtEnd())
        return false;

      if (consumeSlashIncludingWhitespace(range))
      {
        if (!consumeIdent<CSSValueNormal>(range))
        {
          lineHeight = CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyLineHeight, state);
          if (!lineHeight)
            return false;
        }
        if (tokens.IsAtEnd())
          return false;
      }

      fontFamily = CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontFamily, state);
      if (!fontFamily || !tokens.IsAtEnd())
        return false;

      guard.commit();

      auto shorthandProperties = shorthand.Properties();
      for (auto [value, longhand] : zippedRange(values, shorthandProperties.first(values.size())))
        result.AddPropertyForCurrentShorthand(state, longhand, Krys::Move(value), IsImplicit::Yes);
      for (auto longhand : shorthandProperties.subspan(values.size()))
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr, IsImplicit::Yes);

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontVariantShorthand(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state,
                                                           const CSSPropertyShorthand &shorthand,
                                                           CSSPropertyParserResult &result) noexcept
    {
      if (identMatches<CSSValueNormal, CSSValueNone>(range.peek().id()))
      {
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantLigatures, consumeIdent(range));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantCaps, nullptr);
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantAlternates, nullptr);
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantNumeric, nullptr);
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantEastAsian, nullptr);
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantPosition, nullptr);
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantEmoji, nullptr);
        return tokens.IsAtEnd();
      }

      RefPtr<CSSValue> capsValue;
      RefPtr<CSSValue> alternatesValue;
      RefPtr<CSSValue> positionValue;
      RefPtr<CSSValue> eastAsianValue;
      RefPtr<CSSValue> emojiValue;
      CSSFontVariantLigaturesParser ligaturesParser;
      CSSFontVariantNumericParser numericParser;
      auto implicitLigatures = IsImplicit::Yes;
      auto implicitNumeric = IsImplicit::Yes;
      do
      {
        if (range.peek().id() == CSSValueNormal)
          return false;

        if (!capsValue
            && (capsValue =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontVariantCaps, state)))
          continue;

        if (!positionValue
            && (positionValue = CSSPropertyParsing::ParseStylePropertyLonghand(
                  range, CSSPropertyFontVariantPosition, state)))
          continue;

        if (!alternatesValue
            && (alternatesValue = CSSPropertyParsing::ParseStylePropertyLonghand(
                  range, CSSPropertyFontVariantAlternates, state)))
          continue;

        auto ligaturesParseResult = ligaturesParser.consumeLigature(range);
        auto numericParseResult = numericParser.consumeNumeric(range);
        if (ligaturesParseResult == CSSFontVariantLigaturesParser::ParseResult::ConsumedValue)
        {
          implicitLigatures = IsImplicit::No;
          continue;
        }
        if (numericParseResult == CSSFontVariantNumericParser::ParseResult::ConsumedValue)
        {
          implicitNumeric = IsImplicit::No;
          continue;
        }

        if (ligaturesParseResult == CSSFontVariantLigaturesParser::ParseResult::DisallowedValue
            || numericParseResult == CSSFontVariantNumericParser::ParseResult::DisallowedValue)
          return false;

        if (!eastAsianValue
            && (eastAsianValue = CSSPropertyParsing::ParseStylePropertyLonghand(
                  range, CSSPropertyFontVariantEastAsian, state)))
          continue;

        if (state.context.propertySettings.cssFontVariantEmojiEnabled && !emojiValue
            && (emojiValue =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyFontVariantEmoji, state)))
          continue;

        // Saw some value that didn't match anything else.
        return false;
      } while (!tokens.IsAtEnd());

      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantLigatures,
                                            ligaturesParser.finalizeValue().releaseNonNull(),
                                            implicitLigatures);
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantCaps, Krys::Move(capsValue));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantAlternates,
                                            Krys::Move(alternatesValue));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantNumeric,
                                            numericParser.finalizeValue().releaseNonNull(), implicitNumeric);
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantEastAsian,
                                            Krys::Move(eastAsianValue));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantPosition, Krys::Move(positionValue));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFontVariantEmoji, Krys::Move(emojiValue));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontSynthesisShorthand(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state,
                                                             const CSSPropertyShorthand &shorthand,
                                                             CSSPropertyParserResult &result) noexcept
    {
      // none | [ weight || style || small-caps ]
      if (range.peek().id() == CSSValueNone)
      {
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontSynthesisSmallCaps,
                                              consumeIdent(range).releaseNonNull());
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontSynthesisStyle,
                                              CSSPrimitiveValue::create(CSSValueNone));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyFontSynthesisWeight,
                                              CSSPrimitiveValue::create(CSSValueNone));
        return tokens.IsAtEnd();
      }

      bool foundWeight = false;
      bool foundStyle = false;
      bool foundSmallCaps = false;

      auto checkAndMarkExistence = [](bool *found)
      {
        if (*found)
          return false;
        return *found = true;
      };

      while (!tokens.IsAtEnd())
      {
        RefPtr ident = consumeIdent<CSSValueWeight, CSSValueStyle, CSSValueSmallCaps>(range);
        if (!ident)
          return false;
        switch (ident->valueID())
        {
          case CSSValueWeight:
            if (!checkAndMarkExistence(&foundWeight))
              return false;
            break;
          case CSSValueStyle:
            if (!checkAndMarkExistence(&foundStyle))
              return false;
            break;
          case CSSValueSmallCaps:
            if (!checkAndMarkExistence(&foundSmallCaps))
              return false;
            break;
          default: ASSERT_NOT_REACHED(); return false;
        }
      }

      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyFontSynthesisWeight,
        CSSPrimitiveValue::create(foundWeight ? CSSValueAuto : CSSValueNone));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyFontSynthesisStyle,
        CSSPrimitiveValue::create(foundStyle ? CSSValueAuto : CSSValueNone));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyFontSynthesisSmallCaps,
        CSSPrimitiveValue::create(foundSmallCaps ? CSSValueAuto : CSSValueNone));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeTextDecorationSkipShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      if (auto skip = consumeIdentRaw<CSSValueNone, CSSValueAuto, CSSValueInk>(range))
      {
        switch (*skip)
        {
          case CSSValueNone:
            result.AddPropertyForCurrentShorthand(state, CSSPropertyTextDecorationSkipInk,
                                                  CSSPrimitiveValue::create(CSSValueNone));
            return tokens.IsAtEnd();
          case CSSValueAuto:
          case CSSValueInk:
            result.AddPropertyForCurrentShorthand(state, CSSPropertyTextDecorationSkipInk,
                                                  CSSPrimitiveValue::create(CSSValueAuto));
            return tokens.IsAtEnd();
          default: ASSERT_NOT_REACHED(); return false;
        }
      }
      return false;
    }

    KRYS_NODISCARD static bool ConsumeMaskBorderShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      auto components = consumeBorderImageComponents(range, state);
      if (!components)
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaskBorderSource,
                                            Krys::Move(components->source));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaskBorderSlice, Krys::Move(components->slice));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaskBorderWidth, Krys::Move(components->width));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaskBorderOutset,
                                            Krys::Move(components->outset));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaskBorderRepeat,
                                            Krys::Move(components->repeat));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeFlexShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      // <'flex'>        = none | [ <'flex-grow'> <'flex-shrink'>? || <'flex-basis'> ]
      // <'flex-grow'>   = <number [0,∞]>
      //     NOTE: When omitted from shorthand, it is set to 1.
      // <'flex-shrink'> = <number [0,∞]>
      //     NOTE: When omitted from shorthand, it is set to 1.
      // <'flex-basis'>  = content | <'width'>
      //    NOTE: When omitted from shorthand, it is set to 0.
      // https://drafts.csswg.org/css-flexbox/#propdef-flex

      auto isFlexBasisIdent = [](CSSValueID id)
      {
        switch (id)
        {
          case CSSValueAuto:
          case CSSValueContent:
          case CSSValueIntrinsic:
          case CSSValueMinIntrinsic:
          case CSSValueMinContent:
          case CSSValueMaxContent:
          case CSSValueFitContent:   return true;
          default:                   return false;
        }
      };

      RefPtr<CSSPrimitiveValue> flexGrow;
      RefPtr<CSSPrimitiveValue> flexShrink;
      RefPtr<CSSPrimitiveValue> flexBasis;

      if (range.peek().id() == CSSValueNone)
      {
        flexGrow = CSSPrimitiveValue::create(0);
        flexShrink = CSSPrimitiveValue::create(0);
        flexBasis = CSSPrimitiveValue::create(CSSValueAuto);
        range.consumeIncludingWhitespace();
      }
      else
      {
        unsigned index = 0;
        while (!tokens.IsAtEnd() && index++ < 3)
        {
          if (auto number = CSSPrimitiveValueResolver<Number<Nonnegative>>::consumeAndResolve(range, state))
          {
            if (!flexGrow)
              flexGrow = Krys::Move(number);
            else if (!flexShrink)
              flexShrink = Krys::Move(number);
            else if (number->isZero() == true) // flex only allows a basis of 0 (sans units) if flex-grow and
                                               // flex-shrink values have already been set.
              flexBasis = CSSPrimitiveValue::create(0, CSSUnitType::CSS_PX);
            else
              return false;
          }
          else if (!flexBasis)
          {
            if (isFlexBasisIdent(range.peek().id()))
              flexBasis = consumeIdent(range);
            if (!flexBasis)
              flexBasis =
                CSSPrimitiveValueResolver<LengthPercentage<Nonnegative>>::consumeAndResolve(range, state);
            if (index == 2 && !tokens.IsAtEnd())
              return false;
          }
        }
        if (index == 0)
          return false;
        if (!flexGrow)
          flexGrow = CSSPrimitiveValue::create(1);
        if (!flexShrink)
          flexShrink = CSSPrimitiveValue::create(1);

        // FIXME: Using % here is a hack to work around intrinsic sizing implementation being
        // a mess (e.g., turned off for nested column flexboxes, failing to relayout properly even
        // if turned back on for nested columns, etc.). We have layout test coverage of both
        // scenarios.
        if (!flexBasis)
          flexBasis = CSSPrimitiveValue::create(0, CSSUnitType::CSS_PERCENTAGE);
      }

      if (!tokens.IsAtEnd())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyFlexGrow, flexGrow.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFlexShrink, flexShrink.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyFlexBasis, flexBasis.releaseNonNull());
      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakAfterShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      auto keyword = consumeIdentRaw(range);
      if (!keyword || !tokens.IsAtEnd())
        return false;
      auto value = mapFromPageBreakBetween(*keyword);
      if (value == CSSValueInvalid)
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyBreakAfter, CSSPrimitiveValue::create(value));
      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakBeforeShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      auto keyword = consumeIdentRaw(range);
      if (!keyword || !tokens.IsAtEnd())
        return false;
      auto value = mapFromPageBreakBetween(*keyword);
      if (value == CSSValueInvalid)
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyBreakBefore, CSSPrimitiveValue::create(value));
      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakInsideShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      auto keyword = consumeIdentRaw(range);
      if (!keyword || !tokens.IsAtEnd())
        return false;
      auto value = mapFromColumnRegionOrPageBreakInside(*keyword);
      if (value == CSSValueInvalid)
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyBreakInside, CSSPrimitiveValue::create(value));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeTransformOriginShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      if (auto position = consumeOneOrTwoComponentPositionUnresolved(range, state))
      {
        range.consumeWhitespace();
        bool atEnd = tokens.IsAtEnd();
        auto resultZ = CSSPrimitiveValueResolver<Length<>>::consumeAndResolve(range, state);
        if ((!resultZ && !atEnd) || !tokens.IsAtEnd())
          return false;

        auto [positionX, positionY] = split(Krys::Move(*position));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyTransformOriginX,
                                              CSSPositionXValue::create(Krys::Move(positionX)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyTransformOriginY,
                                              CSSPositionYValue::create(Krys::Move(positionY)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyTransformOriginZ, resultZ);
        return true;
      }
      return false;
    }

    KRYS_NODISCARD static bool ConsumePerspectiveOriginShorthand(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state,
                                                                 const CSSPropertyShorthand &shorthand,
                                                                 CSSPropertyParserResult &result) noexcept
    {
      if (auto position = consumePositionUnresolved(range, state))
      {
        auto [positionX, positionY] = split(Krys::Move(*position));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyPerspectiveOriginX,
                                              CSSPositionXValue::create(Krys::Move(positionX)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyPerspectiveOriginY,
                                              CSSPositionYValue::create(Krys::Move(positionY)));
        return true;
      }

      return false;
    }

    KRYS_NODISCARD static bool ConsumeOffsetShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                      const CSSPropertyShorthand &shorthand,
                                                      CSSPropertyParserResult &result) noexcept
    {
      // The offset shorthand is defined as:
      // [ <'offset-position'>?
      //   [ <'offset-path'>
      //     [ <'offset-distance'> || <'offset-rotate'> ]?
      //   ]?
      // ]!
      // [ / <'offset-anchor'> ]?

      // Parse out offset-position.
      auto offsetPosition =
        CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetPosition, state);

      // Parse out offset-path.
      auto offsetPath = CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetPath, state);

      // Either one of offset-position and offset-path must be present.
      if (!offsetPosition && !offsetPath)
        return false;

      // Only parse offset-distance and offset-rotate if offset-path is specified.
      RefPtr<CSSValue> offsetDistance;
      RefPtr<CSSValue> offsetRotate;
      if (offsetPath)
      {
        // Try to parse offset-distance first. If successful, parse the following offset-rotate.
        // Otherwise, parse in the reverse order.
        if ((offsetDistance =
               CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetDistance, state)))
          offsetRotate =
            CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetRotate, state);
        else
        {
          offsetRotate =
            CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetRotate, state);
          offsetDistance =
            CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetDistance, state);
        }
      }

      // Parse out offset-anchor. Only parse if the prefix slash is present.
      RefPtr<CSSValue> offsetAnchor;
      if (consumeSlashIncludingWhitespace(range))
      {
        // offset-anchor must follow the slash.
        if (!(offsetAnchor =
                CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyOffsetAnchor, state)))
          return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyOffsetPath, Krys::Move(offsetPath));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOffsetDistance, Krys::Move(offsetDistance));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOffsetPosition, Krys::Move(offsetPosition));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOffsetAnchor, Krys::Move(offsetAnchor));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOffsetRotate, Krys::Move(offsetRotate));

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeListStyleShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      RefPtr<CSSValue> position;
      RefPtr<CSSValue> image;
      RefPtr<CSSValue> type;
      unsigned noneCount = 0;

      while (!tokens.IsAtEnd())
      {
        if (range.peek().id() == CSSValueNone)
        {
          ++noneCount;
          consumeIdent(range);
          continue;
        }
        if (!position
            && (position =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyListStylePosition, state)))
          continue;

        if (!image
            && (image =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyListStyleImage, state)))
          continue;

        if (!type
            && (type =
                  CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyListStyleType, state)))
          continue;

        return false;
      }

      if (noneCount > (static_cast<unsigned>(!image + !type)))
        return false;

      if (noneCount == 2)
      {
        // Using implicit none for list-style-image is how we serialize "none" instead of "none none".
        image = nullptr;
        type = CSSPrimitiveValue::create(CSSValueNone);
      }
      else if (noneCount == 1)
      {
        // Use implicit none for list-style-image, but non-implicit for type.
        if (!type)
          type = CSSPrimitiveValue::create(CSSValueNone);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyListStylePosition, Krys::Move(position));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyListStyleImage, Krys::Move(image));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyListStyleType, Krys::Move(type));
      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeOverscrollBehaviorShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);

      if (tokens.IsAtEnd())
        return false;

      RefPtr overscrollBehaviorX = CSSPropertyParsing::consumeOverscrollBehaviorX(range);
      if (!overscrollBehaviorX)
        return false;

      RefPtr<CSSValue> overscrollBehaviorY;
      range.consumeWhitespace();
      if (tokens.IsAtEnd())
        overscrollBehaviorY = overscrollBehaviorX;
      else
      {
        overscrollBehaviorY = CSSPropertyParsing::consumeOverscrollBehaviorY(range);
        range.consumeWhitespace();
        if (!tokens.IsAtEnd())
          return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyOverscrollBehaviorX,
                                            Krys::Move(overscrollBehaviorX));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyOverscrollBehaviorY,
                                            Krys::Move(overscrollBehaviorY));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainerShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      RefPtr name = CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyContainerName, state);
      if (!name)
        return false;

      bool sawSlash = false;

      auto consumeSlashType = [&]() -> RefPtr<CSSValue>
      {
        if (tokens.IsAtEnd())
          return nullptr;
        if (!consumeSlashIncludingWhitespace(range))
          return nullptr;
        sawSlash = true;
        return CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyContainerType, state);
      };

      auto type = consumeSlashType();

      if (!tokens.IsAtEnd() || (sawSlash && !type))
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyContainerName, name.releaseNonNull());
      result.AddPropertyForCurrentShorthand(state, CSSPropertyContainerType, Krys::Move(type));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainIntrinsicSizeShorthand(CSSTokenRange &tokens,
                                                                    CSSPropertyParserState &state,
                                                                    const CSSPropertyShorthand &shorthand,
                                                                    CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);
      assert(isExposed(CSSPropertyContainIntrinsicSize, &state.context.propertySettings));

      if (tokens.IsAtEnd())
        return false;

      RefPtr containIntrinsicWidth = CSSPropertyParsing::consumeContainIntrinsicWidth(range, state);
      if (!containIntrinsicWidth)
        return false;

      RefPtr<CSSValue> containIntrinsicHeight;
      range.consumeWhitespace();
      if (tokens.IsAtEnd())
        containIntrinsicHeight = containIntrinsicWidth;
      else
      {
        containIntrinsicHeight = CSSPropertyParsing::consumeContainIntrinsicHeight(range, state);
        range.consumeWhitespace();
        if (!tokens.IsAtEnd() || !containIntrinsicHeight)
          return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyContainIntrinsicWidth,
                                            Krys::Move(containIntrinsicWidth));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyContainIntrinsicHeight,
                                            Krys::Move(containIntrinsicHeight));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeAnimationRangeShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      CSSValueListBuilder startList;
      CSSValueListBuilder endList;
      do
      {
        RefPtr start = consumeSingleAnimationRangeStart(range, state);
        if (!start)
          return false;

        RefPtr<CSSValue> end;
        range.consumeWhitespace();
        if (tokens.IsAtEnd() || tokens.Peek().Type() == CommaToken)
        {
          // From the spec: If <'animation-range-end'> is omitted and <'animation-range-start'> includes a
          // component, then animation-range-end is set to that same and 100%. Otherwise, any omitted longhand
          // is set to its initial value.
          auto rangeEndValueForStartValue = [](const CSSValue &value)
          {
            auto isRangeOffset = [](auto &value)
            {
              return value.isLength() || value.isPercentage() || value.isCalculatedPercentageWithLength();
            };

            if (RefPtr primitiveValue = dynamicDowncast<CSSPrimitiveValue>(value);
                primitiveValue && isRangeOffset(downcast<CSSPrimitiveValue>(value)))
              return CSSPrimitiveValue::create(CSSValueNormal);
            return CSSPrimitiveValue::create(value.valueID());
          };

          if (RefPtr startPrimitiveValue = dynamicDowncast<CSSPrimitiveValue>(start))
            end = rangeEndValueForStartValue(*startPrimitiveValue);
          else
          {
            RefPtr startPair = downcast<CSSValuePair>(start);
            end = rangeEndValueForStartValue(startPair->first());
          }
        }
        else
        {
          end = consumeSingleAnimationRangeEnd(range, state);
          range.consumeWhitespace();
          if (!end)
            return false;
        }
        startList.append(start.releaseNonNull());
        endList.append(end.releaseNonNull());
      } while (ConsumeCommaIncludingWhitespace(range));

      if (!tokens.IsAtEnd())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyAnimationRangeStart,
                                            CSSValueList::createCommaSeparated(Krys::Move(startList)));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyAnimationRangeEnd,
                                            CSSValueList::createCommaSeparated(Krys::Move(endList)));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeScrollTimelineShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      CSSValueListBuilder namesList;
      CSSValueListBuilder axesList;

      do
      {
        // A valid scroll-timeline-name is required.
        if (RefPtr name = CSSPropertyParsing::consumeSingleScrollTimelineName(range))
          namesList.append(name.releaseNonNull());
        else
          return false;

        // A scroll-timeline-axis is optional.
        if (tokens.Peek().Type() == CommaToken || tokens.IsAtEnd())
          axesList.append(CSSPrimitiveValue::create(CSSValueBlock));
        else if (auto axis = CSSPropertyParsing::consumeAxis(range))
          axesList.append(axis.releaseNonNull());
        else
          return false;
      } while (ConsumeCommaIncludingWhitespace(range));

      if (namesList.isEmpty())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyScrollTimelineName,
                                            CSSValueList::createCommaSeparated(Krys::Move(namesList)));
      if (!axesList.isEmpty())
        result.AddPropertyForCurrentShorthand(state, CSSPropertyScrollTimelineAxis,
                                              CSSValueList::createCommaSeparated(Krys::Move(axesList)));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeViewTimelineShorthand(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state,
                                                            const CSSPropertyShorthand &shorthand,
                                                            CSSPropertyParserResult &result) noexcept
    {
      CSSValueListBuilder namesList;
      CSSValueListBuilder axesList;
      CSSValueListBuilder insetsList;

      auto defaultAxis = [] -> Ref<CSSValue>
      {
        return CSSPrimitiveValue::create(CSSValueBlock);
      };
      auto defaultInsets = [] -> Ref<CSSValue>
      {
        return CSSPrimitiveValue::create(CSSValueAuto);
      };

      do
      {
        // A valid view-timeline-name is required.
        if (RefPtr name = CSSPropertyParsing::consumeSingleScrollTimelineName(range))
          namesList.append(name.releaseNonNull());
        else
          return false;

        // Both a view-timeline-axis and a view-timeline-inset are optional.
        if (tokens.Peek().Type() != CommaToken && !tokens.IsAtEnd())
        {
          RefPtr axis = CSSPropertyParsing::consumeAxis(range);
          RefPtr insets = consumeSingleViewTimelineInsetItem(range, state);
          // Since the order of view-timeline-axis and view-timeline-inset is not guaranteed, let's try
          // view-timeline-axis again.
          if (!axis)
            axis = CSSPropertyParsing::consumeAxis(range);
          if (!axis && !insets)
            return false;
          axesList.append(axis ? axis.releaseNonNull() : defaultAxis());
          insetsList.append(insets ? insets.releaseNonNull() : defaultInsets());
        }
        else
        {
          axesList.append(defaultAxis());
          insetsList.append(defaultInsets());
        }
      } while (ConsumeCommaIncludingWhitespace(range));

      if (namesList.isEmpty())
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyViewTimelineName,
                                            CSSValueList::createCommaSeparated(Krys::Move(namesList)));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyViewTimelineAxis,
                                            CSSValueList::createCommaSeparated(Krys::Move(axesList)));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyViewTimelineInset,
                                            CSSValueList::createCommaSeparated(Krys::Move(insetsList)));
      return true;
    }

    KRYS_NODISCARD static bool ConsumeLineClampShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      assert(state.context.propertySettings.cssLineClampEnabled);

      if (range.peek().id() == CSSValueNone)
      {
        // Sets max-lines to none, continue to auto, and block-ellipsis to none.
        result.AddPropertyForCurrentShorthand(state, CSSPropertyMaxLines,
                                              CSSPrimitiveValue::create(CSSValueNone));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyContinue,
                                              CSSPrimitiveValue::create(CSSValueAuto));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockEllipsis,
                                              CSSPrimitiveValue::create(CSSValueNone));
        consumeIdent(range);
        return tokens.IsAtEnd();
      }

      RefPtr<CSSValue> maxLines;
      RefPtr<CSSValue> blockEllipsis;

      for (unsigned propertiesParsed = 0; propertiesParsed < 2 && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!maxLines && (maxLines = CSSPropertyParsing::consumeMaxLines(range, state)))
          continue;
        if (!blockEllipsis && (blockEllipsis = CSSPropertyParsing::consumeBlockEllipsis(range)))
          continue;
        // There has to be at least one valid longhand.
        return false;
      }

      if (!blockEllipsis)
        blockEllipsis = CSSPrimitiveValue::create(CSSValueAuto);

      if (!maxLines)
        maxLines = CSSPrimitiveValue::create(CSSValueNone);

      result.AddPropertyForCurrentShorthand(state, CSSPropertyMaxLines, Krys::Move(maxLines));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyContinue,
                                            CSSPrimitiveValue::create(CSSValueDiscard));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyBlockEllipsis, Krys::Move(blockEllipsis));
      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeTextBoxShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                       const CSSPropertyShorthand &shorthand,
                                                       CSSPropertyParserResult &result) noexcept
    {
      if (tokens.Peek().ValueId() == CSSValueId::Normal)
      {
        // if the single keyword normal is specified, it sets text-box-trim to none and text-box-edge to auto.
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextBoxTrim,
                                              CSSPrimitiveValue::Create(CSSValueId::None));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextBoxEdge,
                                              CSSPrimitiveValue::Create(CSSValueId::Auto));

        ConsumeIdent(tokens);
        return tokens.IsAtEnd();
      }

      RefPtr<CSSValue> textBoxTrim;
      RefPtr<CSSValue> textBoxEdge;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!textBoxTrim && (textBoxTrim = CSSPropertyParsing::ConsumeTextBoxTrim(tokens)))
        {
          continue;
        }

        if (!textBoxEdge && (textBoxEdge = ConsumeTextBoxEdge(tokens, state)))
        {
          continue;
        }

        // There has to be at least one valid longhand.
        return false;
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      // Omitting the text-box-edge value sets it to auto (the initial value)
      if (!textBoxEdge)
        textBoxEdge = CSSPrimitiveValue::Create(CSSValueId::Auto);

      // Omitting the text-box-trim value sets it to both (not the initial value)
      if (!textBoxTrim)
      {
        textBoxTrim = CSSPrimitiveValue::Create(CSSValueId::TrimBoth);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextBoxTrim, Krys::Move(textBoxTrim));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextBoxEdge, Krys::Move(textBoxEdge));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTextWrapShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                        const CSSPropertyShorthand &shorthand,
                                                        CSSPropertyParserResult &result) noexcept
    {
      RefPtr<CSSValue> mode;
      RefPtr<CSSValue> style;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!mode && (mode = CSSPropertyParsing::ConsumeTextWrapMode(tokens)))
        {
          continue;
        }

        if (!style && (style = CSSPropertyParsing::ConsumeTextWrapStyle(tokens, state)))
        {
          continue;
        }

        // If we didn't find at least one match, this is an invalid shorthand and we have to ignore it.
        return false;
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      // Fill in default values if one was missing from the multi-value syntax.
      if (!mode)
      {
        mode = CSSPrimitiveValue::Create(CSSValueId::Wrap);
      }

      if (!style)
      {
        style = CSSPrimitiveValue::Create(CSSValueId::Auto);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextWrapMode, Krys::Move(mode));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextWrapStyle, Krys::Move(style));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeWhiteSpaceShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      RefPtr<CSSValue> whiteSpaceCollapse;
      RefPtr<CSSValue> textWrapMode;

      // Single value syntax.
      auto singleValueKeyword =
        ConsumeIdentRaw<CSSValueId::Normal, CSSValueId::Pre, CSSValueId::PreLine, CSSValueId::PreWrap>(
          tokens);

      if (singleValueKeyword)
      {
        switch (*singleValueKeyword)
        {
          case CSSValueId::Normal:
          {
            whiteSpaceCollapse = CSSPrimitiveValue::Create(CSSValueId::Collapse);
            textWrapMode = CSSPrimitiveValue::Create(CSSValueId::Wrap);
            break;
          }
          case CSSValueId::Pre:
          {
            whiteSpaceCollapse = CSSPrimitiveValue::Create(CSSValueId::Preserve);
            textWrapMode = CSSPrimitiveValue::Create(CSSValueId::Nowrap);
            break;
          }
          case CSSValueId::PreLine:
          {
            whiteSpaceCollapse = CSSPrimitiveValue::Create(CSSValueId::PreserveBreaks);
            textWrapMode = CSSPrimitiveValue::Create(CSSValueId::Wrap);
            break;
          }
          case CSSValueId::PreWrap:
          {
            whiteSpaceCollapse = CSSPrimitiveValue::Create(CSSValueId::Preserve);
            textWrapMode = CSSPrimitiveValue::Create(CSSValueId::Wrap);
            break;
          }
          default:
          {
            assert(false);
            return false;
          }
        }
      }
      else
      {
        // Multi-value syntax.
        for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
        {
          if (!whiteSpaceCollapse
              && (whiteSpaceCollapse = CSSPropertyParsing::ConsumeWhiteSpaceCollapse(tokens)))
          {
            continue;
          }

          if (!textWrapMode && (textWrapMode = CSSPropertyParsing::ConsumeTextWrapMode(tokens)))
          {
            continue;
          }

          // If we didn't find at least one match, this is an invalid shorthand and we have to ignore it.
          return false;
        }
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      // Fill in default values if one was missing from the multi-value syntax.
      if (!whiteSpaceCollapse)
      {
        whiteSpaceCollapse = CSSPrimitiveValue::Create(CSSValueId::Collapse);
      }

      if (!textWrapMode)
      {
        textWrapMode = CSSPrimitiveValue::Create(CSSValueId::Wrap);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::WhiteSpaceCollapse,
                                            Krys::Move(whiteSpaceCollapse));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextWrapMode, Krys::Move(textWrapMode));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePositionTryShorthand(CSSTokenRange &tokens,
                                                           CSSPropertyParserState &state,
                                                           const CSSPropertyShorthand &shorthand,
                                                           CSSPropertyParserResult &result) noexcept
    {
      auto order =
        CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::PositionTryOrder, state);
      auto fallbacks =
        CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::PositionTryFallbacks, state);
      if (!fallbacks)
        return false;

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::PositionTryOrder, Krys::Move(order));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::PositionTryFallbacks,
                                            Krys::Move(fallbacks));
      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeMarkerShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                      const CSSPropertyShorthand &shorthand,
                                                      CSSPropertyParserResult &result) noexcept
    {
      auto marker = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::MarkerStart, state);
      if (!marker || !tokens.IsAtEnd())
      {
        return false;
      }

      Ref<CSSValue> markerRef = Krys::Move(marker);

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MarkerStart, markerRef);
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MarkerMid, markerRef);
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MarkerEnd, Krys::Move(markerRef));

      return true;
    }
  };
}