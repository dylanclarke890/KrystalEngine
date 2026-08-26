#pragma once

#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRangeGuard.hpp"
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
#include "Krystal.HTML/CSS/Properties/Consumers/Inline.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/String.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserResult.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthand.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyShorthandFunctions.hpp"
#include "Krystal.HTML/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.HTML/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.HTML/CSS/Values/CSSPositionValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include "Krystal.Lib/ZippedRange.hpp"

namespace Krys::HTML
{
  using namespace CSSPropertyParserHelpers;

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
        {
          continue;
        }
      }

      if (!components.Color)
      {
        components.Color =
          CSSPropertyParsing::ParseStylePropertyLonghand(range, CSSPropertyId::BorderLeftColor, state);

        if (components.Color)
        {
          continue;
        }
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
      assert(shorthand.Size() == 4uz);

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

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderTopLeftRadius,
                                            CreateCSSValue(state.ValuePool, borderRadius->TopLeft()));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderTopRightRadius,
                                            CreateCSSValue(state.ValuePool, borderRadius->TopRight()));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderBottomRightRadius,
                                            CreateCSSValue(state.ValuePool, borderRadius->BottomRight()));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BorderBottomLeftRadius,
                                            CreateCSSValue(state.ValuePool, borderRadius->BottomLeft()));

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
            // TODO: Implement ConsumeEasingFunction
            return nullptr;
            // return ConsumeEasingFunction(tokens, state);
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

      const size_t longhandCount = shorthand.Size();
      const size_t maxLonghandCount = 11uz;
      assert(longhandCount <= maxLonghandCount);

      auto shorthandProperties = shorthand.Properties();
      Array<CSSValueListBuilder, maxLonghandCount> longhands;

      auto IsResetOnlyLonghand = [](CSSPropertyId longhand)
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
          for (size_t i = 0uz; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
            {
              continue;
            }

            if (auto value = ConsumeAnimationValueForShorthand(shorthandProperties[i]))
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

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i] && !IsResetOnlyLonghand(shorthandProperties[i]))
          {
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
          }

          parsedLonghand[i] = false;
        }
      } while (ConsumeComma(tokens));

      for (size_t i = 0uz; i < longhandCount; ++i)
      {
        auto &list = longhands[i];
        if (list.empty()) // reset-only property
        {
          result.AddPropertyForCurrentShorthand(state, shorthandProperties[i], nullptr);
        }
        else
        {
          result.AddPropertyForCurrentShorthand(state, shorthandProperties[i],
                                                CSSValueList::CreateCommaSeparated(Krys::Move(list)));
        }
      }

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeTransitionShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      auto IsValidTransitionPropertyList = [](CSSPropertyId property, const CSSValueListBuilder &valueList)
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

      auto ConsumeTransitionValueForShorthand = [&](CSSPropertyId property) -> RefPtr<CSSValue>
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
            return nullptr;
            // return ConsumeSingleTransitionPropertyOrNone(tokens, state);
          }
          case CSSPropertyId::TransitionTimingFunction:
          {
            return nullptr;
            // return ConsumeEasingFunction(tokens, state);
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

      const size_t longhandCount = shorthand.Size();
      const size_t maxLonghandCount = 11uz;
      assert(longhandCount <= maxLonghandCount);

      Array<CSSValueListBuilder, maxLonghandCount> longhands;
      auto shorthandProperties = shorthand.Properties();

      do
      {
        Array<bool, maxLonghandCount> parsedLonghand = {};
        do
        {
          bool foundProperty = false;
          for (size_t i = 0uz; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
            {
              continue;
            }

            if (auto value = ConsumeTransitionValueForShorthand(shorthandProperties[i]))
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

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i])
          {
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
          }
          parsedLonghand[i] = false;
        }
      } while (ConsumeComma(tokens));

      for (size_t i = 0uz; i < longhandCount; ++i)
      {
        if (!IsValidTransitionPropertyList(shorthandProperties[i], longhands[i]))
        {
          return false;
        }
      }

      for (size_t i = 0uz; i < longhandCount; ++i)
      {
        result.AddPropertyForCurrentShorthand(state, shorthandProperties[i],
                                              CSSValueList::CreateCommaSeparated(Krys::Move(longhands[i])));
      }

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeBackgroundShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Id() == state.CurrentProperty);

      auto ConsumeBackgroundComponent = [&](CSSPropertyId property) -> RefPtr<CSSValue>
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

      Array<CSSValueListBuilder, 10uz> longhands;
      assert(longhandCount <= 10uz);

      do
      {
        bool lastParsedWasPosition = false;
        bool clipIsBorderArea = false;

        Array<bool, 10uz> parsedLonghand = {};
        RefPtr<CSSValue> originValue;
        do
        {
          bool foundProperty = false;
          for (size_t i = 0uz; i < longhandCount; ++i)
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

              auto [positionX, positionY] = Split(Krys::Move(*position));
              value = CSSPositionXValue::Create(Krys::Move(positionX));
              valueY = CSSPositionYValue::Create(Krys::Move(positionY));
            }
            else if (property == CSSPropertyId::BackgroundSize)
            {
              if (!ConsumeSlash(tokens))
              {
                continue;
              }

              if (!lastParsedWasPosition)
              {
                return false;
              }

              value = ConsumeSingleBackgroundSize(tokens, state);
              if (!value)
              {
                return false;
              }
            }
            else if (property == CSSPropertyId::MaskSize)
            {
              if (!ConsumeSlash(tokens))
              {
                continue;
              }

              if (!lastParsedWasPosition)
              {
                return false;
              }

              value = ConsumeSingleMaskSize(tokens, state);
              if (!value)
              {
                return false;
              }
            }
            else if (property == CSSPropertyId::BackgroundPositionY)
            {
              continue;
            }
            else
            {
              value = ConsumeBackgroundComponent(property);
            }
            if (value)
            {
              if (property == CSSPropertyId::BackgroundOrigin || property == CSSPropertyId::MaskOrigin)
              {
                originValue = value;
              }
              else if (property == CSSPropertyId::BackgroundClip)
              {
                clipIsBorderArea = value->ValueId() == CSSValueId::BorderArea;
              }

              parsedLonghand[i] = true;
              foundProperty = true;

              longhands[i].push_back(Krys::Move(value));
              lastParsedWasPosition = valueY != nullptr;
              if (valueY)
              {
                parsedLonghand[i + 1uz] = true;
                longhands[i + 1uz].push_back(Krys::Move(valueY));
              }
            }
          }

          if (!foundProperty)
          {
            return false;
          }
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Comma);

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          auto property = shorthandProperties[i];
          if (property == CSSPropertyId::BackgroundColor && !tokens.IsAtEnd())
          {
            if (parsedLonghand[i])
            {
              return false; // Colors are only allowed in the last layer.
            }
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
          {
            longhands[i].push_back(ShareRef(CSSPrimitiveValue::ImplicitInitialValue()));
          }
        }
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      for (size_t i = 0uz; i < longhandCount; ++i)
      {
        auto property = shorthandProperties[i];
        if (longhands[i].size() == 1uz)
        {
          result.AddPropertyForCurrentShorthand(state, property, Krys::Move(longhands[i][0]));
        }
        else
        {
          result.AddPropertyForCurrentShorthand(state, property,
                                                CSSValueList::CreateCommaSeparated(Krys::Move(longhands[i])));
        }
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBackgroundPositionShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Id() == state.CurrentProperty);

      CSSValueListBuilder x;
      CSSValueListBuilder y;
      do
      {
        auto position = ConsumeBackgroundPositionUnresolved(tokens, state);
        if (!position)
        {
          return false;
        }

        auto [positionX, positionY] = Split(Krys::Move(*position));
        x.push_back(CSSPositionXValue::Create(Krys::Move(positionX)));
        y.push_back(CSSPositionYValue::Create(Krys::Move(positionY)));
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr<CSSValue> resultX;
      RefPtr<CSSValue> resultY;
      if (x.size() == 1uz)
      {
        resultX = Krys::Move(x[0]);
        resultY = Krys::Move(y[0]);
      }
      else
      {
        resultX = CSSValueList::CreateCommaSeparated(Krys::Move(x));
        resultY = CSSValueList::CreateCommaSeparated(Krys::Move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(resultX));
      result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(resultY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeMaskShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      if (!ConsumeBackgroundShorthand(tokens, state, shorthand, result))
      {
        return false;
      }

      for (auto longhand : MaskBorderShorthand())
      {
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr);
      }

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
        auto position = ConsumePositionUnresolved(tokens, state);
        if (!position)
          return false;
        auto [positionX, positionY] = Split(Krys::Move(*position));
        x.push_back(CSSPositionXValue::Create(Krys::Move(positionX)));
        y.push_back(CSSPositionYValue::Create(Krys::Move(positionY)));
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr<CSSValue> resultX;
      RefPtr<CSSValue> resultY;
      if (x.size() == 1uz)
      {
        resultX = Krys::Move(x[0]);
        resultY = Krys::Move(y[0]);
      }
      else
      {
        resultX = CSSValueList::CreateCommaSeparated(Krys::Move(x));
        resultY = CSSValueList::CreateCommaSeparated(Krys::Move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(resultX));
      result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(resultY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeOverflowShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                        const CSSPropertyShorthand &shorthand,
                                                        CSSPropertyParserResult &result) noexcept
    {
      CSSValueId xValueId = tokens.Consume().ValueId();
      tokens.DiscardWhitespace();

      if (!CSSPropertyParsing::IsKeywordValidForStyleProperty(CSSPropertyId::OverflowY, xValueId, state))
      {
        return false;
      }

      CSSValueId yValueId;
      if (tokens.IsAtEnd())
      {
        yValueId = xValueId;
      }
      else
      {
        yValueId = tokens.Consume().ValueId();
        tokens.DiscardWhitespace();
      }

      if (!CSSPropertyParsing::IsKeywordValidForStyleProperty(CSSPropertyId::OverflowY, yValueId, state))
      {
        return false;
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OverflowX,
                                            CSSPrimitiveValue::Create(xValueId));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OverflowY,
                                            CSSPrimitiveValue::Create(yValueId));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeColumnsShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                       const CSSPropertyShorthand &shorthand,
                                                       CSSPropertyParserResult &result) noexcept
    {
      RefPtr<CSSValue> columnWidth;
      RefPtr<CSSValue> columnCount;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (tokens.Peek().ValueId() == CSSValueId::Auto)
        {
          // 'auto' is a valid value for any of the two longhands, and at this point
          // we don't know which one(s) it is meant for. We need to see if there are other values first.
          ConsumeIdent(tokens);
        }
        else
        {
          if (!columnWidth
              && (columnWidth = CSSPropertyParsing::ParseStylePropertyLonghand(
                    tokens, CSSPropertyId::ColumnWidth, state)))
          {
            continue;
          }

          if (!columnCount
              && (columnCount = CSSPropertyParsing::ParseStylePropertyLonghand(
                    tokens, CSSPropertyId::ColumnCount, state)))
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

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ColumnWidth, Krys::Move(columnWidth));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ColumnCount, Krys::Move(columnCount));

      return true;
    }

    // KRYS_NODISCARD static bool ConsumeGridItemPositionShorthand(CSSTokenRange &tokens,
    //                                                             CSSPropertyParserState &state,
    //                                                             const CSSPropertyShorthand &shorthand,
    //                                                             CSSPropertyParserResult &result) noexcept
    //{
    //   assert(shorthand.Id() == state.CurrentProperty);
    //   assert(shorthand.Size() == 2uz);

    // RefPtr<CSSValue> startValue = ConsumeGridLine(tokens, state);
    // if (!startValue)
    // {
    //   return false;
    // }

    // RefPtr<CSSValue> endValue;
    // if (ConsumeSlash(tokens))
    // {
    //   endValue = ConsumeGridLine(tokens, state);
    //   if (!endValue)
    //   {
    //     return false;
    //   }
    // }
    // else
    // {
    //   endValue = IsCustomIdentValue(*startValue) ? startValue :
    //   CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }

    // if (!tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // auto longhands = shorthand.Properties();
    // result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(startValue));
    // result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(endValue));

    // return true;
    // }

    // KRYS_NODISCARD static bool ConsumeGridTemplateShorthand(CSSTokenRange &tokens,
    //                                                         CSSPropertyParserState &state,
    //                                                         const CSSPropertyShorthand &shorthand,
    //                                                         CSSPropertyParserResult &result) noexcept
    //{
    //   CSSTokenRange rangeCopy = tokens;
    //   RefPtr<CSSValue> rowsValue = ConsumeIdent<CSSValueId::None>(rangeCopy);

    // // 1- 'none' case.
    // if (rowsValue && tokens.IsAtEnd())
    // {
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateRows,
    //                                         CSSPrimitiveValue::Create(CSSValueId::None));
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateColumns,
    //                                         CSSPrimitiveValue::Create(CSSValueId::None));
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateAreas,
    //                                         CSSPrimitiveValue::Create(CSSValueId::None));

    // return true;
    // }

    // // 2- <grid-template-rows> / <grid-template-columns>
    // if (!rowsValue)
    // {
    //   rowsValue = ConsumeGridTrackList(tokens, state, GridTemplate);
    // }

    // if (rowsValue)
    // {
    //   if (!ConsumeSlash(tokens))
    //   {
    //     return false;
    //   }

    // RefPtr columnsValue = ConsumeGridTemplatesRowsOrColumns(tokens, state);
    // if (!columnsValue || !tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateRows, Krys::Move(rowsValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateColumns,
    //                                       Krys::Move(columnsValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateAreas,
    //                                       CSSPrimitiveValue::Create(CSSValueId::None));

    // return true;
    // }

    // // 3- [ <line-names>? <string> <track-size>? <line-names>? ]+ [ / <track-list> ]?

    // tokens = rangeCopy;

    // CSS::GridNamedAreaMap gridAreaMap;
    // CSSValueListBuilder templateRows;

    // // Persists between loop iterations so we can use the same value for
    // // consecutive <line-names> values
    // RefPtr<CSSGridLineNamesValue> lineNames;

    // do
    // {
    //   // Handle leading <custom-ident>*.
    //   auto previousLineNames = std::exchange(lineNames, ConsumeGridLineNames(tokens, state));
    //   if (lineNames)
    //   {
    //     if (!previousLineNames)
    //       templateRows.append(Krys::Move(lineNames));
    //     else
    //     {
    //       SmallList<CSSOMString> combinedLineNames;
    //       combinedLineNames.append(previousLineNames->names());
    //       combinedLineNames.append(lineNames->names());
    //       templateRows.back() = CSSGridLineNamesValue::create(combinedLineNames);
    //     }
    //   }

    // // Handle a template-area's row.
    // auto row = ConsumeUnresolvedGridTemplateAreasRow(tokens, state);
    // if (!row || !CSS::addRow(gridAreaMap, *row))
    //   return false;

    // // Handle template-rows's track-size.
    // if (RefPtr value = ConsumeGridTrackSize(tokens, state))
    // {
    //   templateRows.push_back(Krys::Move(value));
    // }
    // else
    // {
    //   templateRows.push_back(CSSPrimitiveValue::Create(CSSValueId::Auto));
    // }

    // // This will handle the trailing/leading <custom-ident>* in the grammar.
    // lineNames = ConsumeGridLineNames(tokens, state);
    // if (lineNames)
    //   templateRows.append(*lineNames);
    // } while (!tokens.IsAtEnd()
    //        && !(tokens.Peek().Type() == CSSTokenType::Delim && tokens.Peek().IdentCodePoints() == u8"/"));

    // RefPtr<CSSValue> columnsValue;
    // if (!tokens.IsAtEnd())
    // {
    //   if (!ConsumeSlash(tokens))
    //     return false;
    //   columnsValue = ConsumeGridTrackList(tokens, state, GridTemplateNoRepeat);
    //   if (!columnsValue || !tokens.IsAtEnd())
    //     return false;
    // }
    // else
    // {
    //   columnsValue = CSSPrimitiveValue::Create(CSSValueId::None);
    // }
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateRows,
    //                                       CSSValueList::CreateSpaceSeparated(Krys::Move(templateRows)));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateColumns,
    //                                       Krys::Move(columnsValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateAreas,
    //                                       CSSGridTemplateAreasValue::Create({Krys::Move(gridAreaMap)}));

    // return true;
    // }

    // KRYS_NODISCARD static bool ConsumeGridShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
    //                                                 const CSSPropertyShorthand &shorthand,
    //                                                 CSSPropertyParserResult &result) noexcept
    //{
    //   assert(shorthand.Size() == 6);

    // auto ConsumeImplicitGridAutoFlow = [](CSSTokenRange &range,
    //                                       CSSValueId flowDirection) -> RefPtr<CSSValue>
    // {
    //   // [ auto-flow && dense? ]
    //   bool autoFlow = ConsumeIdentRaw<CSSValueId::AutoFlow>(range).has_value();
    //   bool dense = ConsumeIdentRaw<CSSValueId::Dense>(range).has_value();
    //   if (!autoFlow && (!dense || !ConsumeIdentRaw<CSSValueId::AutoFlow>(range)))
    //   {
    //     return nullptr;
    //   }

    // if (!dense)
    // {
    //   return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(flowDirection));
    // }

    // if (flowDirection == CSSValueId::Row)
    // {
    //   return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(CSSValueId::Dense));
    // }

    // return CSSValueList::CreateSpaceSeparated(CSSPrimitiveValue::Create(flowDirection),
    //                                           CSSPrimitiveValue::Create(CSSValueId::Dense));
    // };

    // CSSTokenRange rangeCopy = tokens;

    // // 1- <grid-template>
    // if (ConsumeGridTemplateShorthand(tokens, state, GridTemplateShorthand(), result))
    // {
    //   // It can only be specified the explicit or the implicit grid properties in a single grid
    //   declaration.
    //   // The sub-properties not specified are set to their initial value, as normal for shorthands.
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoFlow,
    //                                         CSSPrimitiveValue::Create(CSSValueId::Row));
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoColumns,
    //                                         CSSPrimitiveValue::Create(CSSValueId::Auto));
    //   result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoRows,
    //                                         CSSPrimitiveValue::Create(CSSValueId::Auto));

    // return true;
    // }

    // tokens = rangeCopy;

    // RefPtr<CSSValue> autoColumnsValue;
    // RefPtr<CSSValue> autoRowsValue;
    // RefPtr<CSSValue> templateRows;
    // RefPtr<CSSValue> templateColumns;
    // RefPtr<CSSValue> gridAutoFlow;

    // if (tokens.Peek().ValueId() == CSSValueId::AutoFlow || tokens.Peek().ValueId() == CSSValueId::Dense)
    // {
    //   // 2- [ auto-flow && dense? ] <grid-auto-rows>? / <grid-template-columns>
    //   gridAutoFlow = ConsumeImplicitGridAutoFlow(tokens, CSSValueId::Row);
    //   if (!gridAutoFlow || tokens.IsAtEnd())
    //     return false;
    //   if (ConsumeSlash(tokens))
    //     autoRowsValue = CSSPrimitiveValue::Create(CSSValueId::Auto);
    //   else
    //   {
    //     autoRowsValue = ConsumeGridTrackList(tokens, state, GridAuto);
    //     if (!autoRowsValue)
    //     {
    //       return false;
    //     }

    // if (!ConsumeSlash(tokens))
    // {
    //   return false;
    // }
    // }

    // if (tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // templateColumns = ConsumeGridTemplatesRowsOrColumns(tokens, state);
    // if (!templateColumns)
    // {
    //   return false;
    // }

    // templateRows = CSSPrimitiveValue::Create(CSSValueId::None);
    // autoColumnsValue = CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }
    // else
    // {
    // // 3- <grid-template-rows> / [ auto-flow && dense? ] <grid-auto-columns>?
    // templateRows = ConsumeGridTemplatesRowsOrColumns(tokens, state);
    // if (!templateRows)
    // {
    //   return false;
    // }

    // if (!ConsumeSlash(tokens) || tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // gridAutoFlow = ConsumeImplicitGridAutoFlow(tokens, CSSValueId::Column);
    // if (!gridAutoFlow)
    // {
    //   return false;
    // }

    // if (tokens.IsAtEnd())
    // {
    //   autoColumnsValue = CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }
    // else
    // {
    //   autoColumnsValue = ConsumeGridTrackList(tokens, state, GridAuto);
    //   if (!autoColumnsValue)
    //   {
    //     return false;
    //   }
    // }

    // templateColumns = CSSPrimitiveValue::Create(CSSValueId::None);
    // autoRowsValue = CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }

    // if (!tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // // It can only be specified the explicit or the implicit grid properties in a single grid declaration.
    // // The sub-properties not specified are set to their initial value, as normal for shorthands.
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateColumns,
    //                                       Krys::Move(templateColumns));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridTemplateRows,
    // Krys::Move(templateRows)); result.AddPropertyForCurrentShorthand(state,
    // CSSPropertyId::GridTemplateAreas,
    //                                       CSSPrimitiveValue::Create(CSSValueId::None));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoFlow, Krys::Move(gridAutoFlow));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoColumns,
    //                                       Krys::Move(autoColumnsValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridAutoRows, Krys::Move(autoRowsValue));

    // return true;
    // }

    // KRYS_NODISCARD static bool ConsumeGridAreaShorthand(CSSTokenRange &tokens, CSSPropertyParserState
    // &state,
    //                                                     const CSSPropertyShorthand &shorthand,
    //                                                     CSSPropertyParserResult &result) noexcept
    //{
    //   RefPtr rowStartValue = ConsumeGridLine(tokens, state);
    //   if (!rowStartValue)
    //   {
    //     return false;
    //   }

    // RefPtr<CSSValue> columnStartValue;
    // RefPtr<CSSValue> rowEndValue;
    // RefPtr<CSSValue> columnEndValue;

    // if (ConsumeSlash(tokens))
    // {
    //   columnStartValue = ConsumeGridLine(tokens, state);
    //   if (!columnStartValue)
    //   {
    //     return false;
    //   }

    // if (ConsumeSlash(tokens))
    // {
    //   rowEndValue = ConsumeGridLine(tokens, state);
    //   if (!rowEndValue)
    //   {
    //     return false;
    //   }

    // if (ConsumeSlash(tokens))
    // {
    //   columnEndValue = ConsumeGridLine(tokens, state);
    //   if (!columnEndValue)
    //   {
    //     return false;
    //   }
    // }
    // }
    // }

    // if (!tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // if (!columnStartValue)
    // {
    //   columnStartValue =
    //     IsCustomIdentValue(*rowStartValue) ? rowStartValue : CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }

    // if (!rowEndValue)
    // {
    //   rowEndValue =
    //     IsCustomIdentValue(*rowStartValue) ? rowStartValue : CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }

    // if (!columnEndValue)
    // {
    //   columnEndValue = IsCustomIdentValue(*columnStartValue) ? columnStartValue
    //                                                          : CSSPrimitiveValue::Create(CSSValueId::Auto);
    // }

    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridRowStart, Krys::Move(rowStartValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridColumnStart,
    //                                       Krys::Move(columnStartValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridRowEnd, Krys::Move(rowEndValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::GridColumnEnd, Krys::Move(columnEndValue));

    // return true;
    // }

    KRYS_NODISCARD static bool ConsumeAlignShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                     const CSSPropertyShorthand &shorthand,
                                                     CSSPropertyParserResult &result) noexcept
    {
      // Used to implement the rules in CSS Align for the following shorthands:
      //   <'place-content'> https://drafts.csswg.org/css-align/#propdef-place-content
      //   <'place-items'>   https://drafts.csswg.org/css-align/#propdef-place-items
      //   <'place-self'>    https://drafts.csswg.org/css-align/#propdef-place-self
      //   <'gap'>           https://drafts.csswg.org/css-align/#propdef-gap

      assert(shorthand.Id() == state.CurrentProperty);
      assert(shorthand.Size() == 2);
      auto longhands = shorthand.Properties();

      auto rangeCopy = tokens;

      RefPtr prop1 = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!prop1)
      {
        return false;
      }

      // If there are no more tokens, that prop2 should use re-use the original range. This is the equivalent
      // of copying and validating prop1.
      if (tokens.IsAtEnd())
      {
        tokens = rangeCopy;
      }

      RefPtr prop2 = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      if (!prop2 || !tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, longhands[0], Krys::Move(prop1));
      result.AddPropertyForCurrentShorthand(state, longhands[1], Krys::Move(prop2));

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

      for (size_t propertiesParsed = 0uz; propertiesParsed < 4uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!size && (size = CSSPropertyParsing::ConsumeBlockStepSize(tokens, state)))
        {
          continue;
        }

        if (!insert && (insert = CSSPropertyParsing::ConsumeBlockStepInsert(tokens)))
        {
          continue;
        }

        if (!align && (align = CSSPropertyParsing::ConsumeBlockStepAlign(tokens)))
        {
          continue;
        }

        if (!round && (round = CSSPropertyParsing::ConsumeBlockStepRound(tokens)))
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

      // Fill in default values if one was missing.
      if (!size)
      {
        size = CSSPrimitiveValue::Create(CSSValueId::None);
      }

      if (!insert)
      {
        insert = CSSPrimitiveValue::Create(CSSValueId::MarginBox);
      }

      if (!align)
      {
        align = CSSPrimitiveValue::Create(CSSValueId::Auto);
      }

      if (!round)
      {
        round = CSSPrimitiveValue::Create(CSSValueId::Up);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockStepSize, Krys::Move(size));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockStepInsert, Krys::Move(insert));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockStepAlign, Krys::Move(align));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockStepRound, Krys::Move(round));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                    const CSSPropertyShorthand &shorthand,
                                                    CSSPropertyParserResult &result) noexcept
    {
      // if (IsSystemFontShorthand(tokens.Peek().ValueId()))
      //{
      //   auto systemFont = tokens.Consume().ValueId();
      //   tokens.DiscardWhitespace();

      // if (!tokens.IsAtEnd())
      // {
      //   return false;
      // }

      // // We can't store properties (weight, size, etc.) of the system font here,
      // // since those values can change (e.g. accessibility font sizes, or accessibility bold).
      // // Parsing (correctly) doesn't re-run in response to updateStyleAfterChangeInEnvironment().
      // // Instead, we store sentinel values, later replaced by environment-sensitive values
      // // inside Style::BuilderCustom and Style::BuilderConverter.
      // result.AddPropertyForAllLonghandsOfCurrentShorthand(state, CSSPrimitiveValue::Create(systemFont),
      //                                                     IsImplicit(true));

      // return true;
      // }

      CSSTokenRangeGuard guard {tokens};

      Array<RefPtr<CSSValue>, 7> values;
      auto &fontStyle = values[0];
      auto &fontVariantCaps = values[1];
      auto &fontWeight = values[2];
      auto &fontWidth = values[3];
      auto &fontSize = values[4];
      auto &lineHeight = values[5];
      auto &fontFamily = values[6];

      // Optional font-style, font-variant, font-width and font-weight, in any order.
      for (size_t i = 0uz; i < 4 && !tokens.IsAtEnd(); ++i)
      {
        if (ConsumeIdent<CSSValueId::Normal>(tokens))
        {
          continue;
        }

        if (!fontStyle
            && (fontStyle =
                  CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::FontStyle, state)))
        {
          continue;
        }

        if (!fontVariantCaps && (fontVariantCaps = ConsumeIdent<CSSValueId::SmallCaps>(tokens)))
        {
          continue;
        }

        if (!fontWeight
            && (fontWeight =
                  CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::FontWeight, state)))
        {
          continue;
        }

        if (!fontWidth && (fontWidth = CSSPropertyParsing::ConsumeFontWidthAbsolute(tokens)))
        {
          continue;
        }

        break;
      }

      if (tokens.IsAtEnd())
      {
        return false;
      }

      fontSize = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::FontSize, state);
      if (!fontSize || tokens.IsAtEnd())
      {
        return false;
      }

      if (ConsumeSlash(tokens))
      {
        if (!ConsumeIdent<CSSValueId::Normal>(tokens))
        {
          lineHeight =
            CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::LineHeight, state);
          if (!lineHeight)
          {
            return false;
          }
        }

        if (tokens.IsAtEnd())
        {
          return false;
        }
      }

      fontFamily = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::FontFamily, state);
      if (!fontFamily || !tokens.IsAtEnd())
      {
        return false;
      }

      guard.Commit();

      auto shorthandProperties = shorthand.Properties();
      for (auto [value, longhand] : zippedRange(values, shorthandProperties.first(values.size())))
      {
        result.AddPropertyForCurrentShorthand(state, longhand, Krys::Move(value), IsImplicit(true));
      }

      for (auto longhand : shorthandProperties.subspan(values.size()))
      {
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr, IsImplicit(true));
      }

      return true;
    }

    // KRYS_NODISCARD static bool ConsumeFontVariantShorthand(CSSTokenRange &tokens,
    //                                                        CSSPropertyParserState &state,
    //                                                        const CSSPropertyShorthand &shorthand,
    //                                                        CSSPropertyParserResult &result) noexcept
    //{
    //   if (IdentMatches<CSSValueId::Normal, CSSValueId::None>(tokens.Peek().ValueId()))
    //   {
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantLigatures,
    //                                           ConsumeIdent(tokens));
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantCaps, nullptr);
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantAlternates, nullptr);
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantNumeric, nullptr);
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantEastAsian, nullptr);
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantPosition, nullptr);
    //     result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantEmoji, nullptr);
    //     return tokens.IsAtEnd();
    //   }

    // RefPtr<CSSValue> capsValue;
    // RefPtr<CSSValue> alternatesValue;
    // RefPtr<CSSValue> positionValue;
    // RefPtr<CSSValue> eastAsianValue;
    // RefPtr<CSSValue> emojiValue;
    // CSSFontVariantLigaturesParser ligaturesParser;
    // CSSFontVariantNumericParser numericParser;
    // auto implicitLigatures = IsImplicit(true);
    // auto implicitNumeric = IsImplicit(true);
    // do
    // {
    //   if (tokens.Peek().ValueId() == CSSValueId::Normal)
    //   {
    //     return false;
    //   }

    // if (!capsValue
    //     && (capsValue = CSSPropertyParsing::ParseStylePropertyLonghand(
    //           tokens, CSSPropertyId::FontVariantCaps, state)))
    // {
    //   continue;
    // }

    // if (!positionValue
    //     && (positionValue = CSSPropertyParsing::ParseStylePropertyLonghand(
    //           tokens, CSSPropertyId::FontVariantPosition, state)))
    // {
    //   continue;
    // }

    // if (!alternatesValue
    //     && (alternatesValue = CSSPropertyParsing::ParseStylePropertyLonghand(
    //           tokens, CSSPropertyId::FontVariantAlternates, state)))
    // {
    //   continue;
    // }

    // auto ligaturesParseResult = ligaturesParser.consumeLigature(tokens);
    // auto numericParseResult = numericParser.consumeNumeric(tokens);
    // if (ligaturesParseResult == CSSFontVariantLigaturesParser::ParseResult::ConsumedValue)
    // {
    //   implicitLigatures = IsImplicit(false);
    //   continue;
    // }
    // if (numericParseResult == CSSFontVariantNumericParser::ParseResult::ConsumedValue)
    // {
    //   implicitNumeric = IsImplicit(false);
    //   continue;
    // }

    // if (ligaturesParseResult == CSSFontVariantLigaturesParser::ParseResult::DisallowedValue
    //     || numericParseResult == CSSFontVariantNumericParser::ParseResult::DisallowedValue)
    // {
    //   return false;
    // }

    // if (!eastAsianValue
    //     && (eastAsianValue = CSSPropertyParsing::ParseStylePropertyLonghand(
    //           tokens, CSSPropertyId::FontVariantEastAsian, state)))
    // {
    //   continue;
    // }

    // if (state.Context.PropertySettings.cssFontVariantEmojiEnabled && !emojiValue
    //     && (emojiValue = CSSPropertyParsing::ParseStylePropertyLonghand(
    //           tokens, CSSPropertyId::FontVariantEmoji, state)))
    // {
    //   continue;
    // }

    // // Saw some value that didn't match anything else.
    // return false;
    // } while (!tokens.IsAtEnd());

    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantLigatures,
    //                                       ligaturesParser.finalizeValue().releaseNonNull(),
    //                                       implicitLigatures);
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantCaps, Krys::Move(capsValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantAlternates,
    //                                       Krys::Move(alternatesValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantNumeric,
    //                                       numericParser.finalizeValue().releaseNonNull(), implicitNumeric);
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantEastAsian,
    //                                       Krys::Move(eastAsianValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantPosition,
    //                                       Krys::Move(positionValue));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontVariantEmoji, Krys::Move(emojiValue));

    // return true;
    // }

    KRYS_NODISCARD static bool ConsumeFontSynthesisShorthand(CSSTokenRange &tokens,
                                                             CSSPropertyParserState &state,
                                                             const CSSPropertyShorthand &shorthand,
                                                             CSSPropertyParserResult &result) noexcept
    {
      // none | [ weight || style || small-caps ]
      if (tokens.Peek().ValueId() == CSSValueId::None)
      {
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontSynthesisSmallCaps,
                                              ConsumeIdent(tokens));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontSynthesisStyle,
                                              CSSPrimitiveValue::Create(CSSValueId::None));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FontSynthesisWeight,
                                              CSSPrimitiveValue::Create(CSSValueId::None));
        return tokens.IsAtEnd();
      }

      bool foundWeight = false;
      bool foundStyle = false;
      bool foundSmallCaps = false;

      auto CheckAndMarkExistence = [](bool *found)
      {
        if (*found)
        {
          return false;
        }

        return *found = true;
      };

      while (!tokens.IsAtEnd())
      {
        RefPtr ident = ConsumeIdent<CSSValueId::Weight, CSSValueId::Style, CSSValueId::SmallCaps>(tokens);
        if (!ident)
        {
          return false;
        }

        switch (ident->ValueId())
        {
          case CSSValueId::Weight:
          {
            if (!CheckAndMarkExistence(&foundWeight))
            {
              return false;
            }

            break;
          }
          case CSSValueId::Style:
          {
            if (!CheckAndMarkExistence(&foundStyle))
            {
              return false;
            }

            break;
          }
          case CSSValueId::SmallCaps:
          {
            if (!CheckAndMarkExistence(&foundSmallCaps))
            {
              return false;
            }

            break;
          }
          default:
          {
            assert(false);
            return false;
          }
        }
      }

      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::FontSynthesisWeight,
        CSSPrimitiveValue::Create(foundWeight ? CSSValueId::Auto : CSSValueId::None));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::FontSynthesisStyle,
        CSSPrimitiveValue::Create(foundStyle ? CSSValueId::Auto : CSSValueId::None));
      result.AddPropertyForCurrentShorthand(
        state, CSSPropertyId::FontSynthesisSmallCaps,
        CSSPrimitiveValue::Create(foundSmallCaps ? CSSValueId::Auto : CSSValueId::None));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTextDecorationSkipShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      if (auto skip = ConsumeIdentRaw<CSSValueId::None, CSSValueId::Auto, CSSValueId::Ink>(tokens))
      {
        switch (*skip)
        {
          case CSSValueId::None:
          {
            result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextDecorationSkipInk,
                                                  CSSPrimitiveValue::Create(CSSValueId::None));
            return tokens.IsAtEnd();
          }
          case CSSValueId::Auto:
          case CSSValueId::Ink:
          {
            result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TextDecorationSkipInk,
                                                  CSSPrimitiveValue::Create(CSSValueId::Auto));
            return tokens.IsAtEnd();
          }
          default:
          {
            assert(false);
            return false;
          }
        }
      }

      return false;
    }

    KRYS_NODISCARD static bool ConsumeMaskBorderShorthand(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state,
                                                          const CSSPropertyShorthand &shorthand,
                                                          CSSPropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderImageComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaskBorderSource,
                                            Krys::Move(components->Source));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaskBorderSlice,
                                            Krys::Move(components->Slice));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaskBorderWidth,
                                            Krys::Move(components->Width));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaskBorderOutset,
                                            Krys::Move(components->Outset));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaskBorderRepeat,
                                            Krys::Move(components->Repeat));

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

      auto IsFlexBasisIdent = [](CSSValueId id)
      {
        switch (id)
        {
          case CSSValueId::Auto:
          case CSSValueId::Content:
          case CSSValueId::Intrinsic:
          case CSSValueId::MinIntrinsic:
          case CSSValueId::MinContent:
          case CSSValueId::MaxContent:
          case CSSValueId::FitContent:
          {
            return true;
          }
          default:
          {
            return false;
          }
        }
      };

      RefPtr<CSSPrimitiveValue> flexGrow;
      RefPtr<CSSPrimitiveValue> flexShrink;
      RefPtr<CSSPrimitiveValue> flexBasis;

      if (tokens.Peek().ValueId() == CSSValueId::None)
      {
        flexGrow = CSSPrimitiveValue::Create(0);
        flexShrink = CSSPrimitiveValue::Create(0);
        flexBasis = CSSPrimitiveValue::Create(CSSValueId::Auto);

        tokens.Discard();
        tokens.DiscardWhitespace();
      }
      else
      {
        size_t index = 0uz;
        while (!tokens.IsAtEnd() && index++ < 3uz)
        {
          if (auto number = CSSPrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state))
          {
            if (!flexGrow)
            {
              flexGrow = Krys::Move(number);
            }
            else if (!flexShrink)
            {
              flexShrink = Krys::Move(number);
            }
            else if (number->IsZero() == true) // flex only allows a basis of 0 (sans units) if flex-grow and
                                               // flex-shrink values have already been set.
            {
              flexBasis = CSSPrimitiveValue::Create(0, CSSUnitType::px);
            }
            else
            {
              return false;
            }
          }
          else if (!flexBasis)
          {
            if (IsFlexBasisIdent(tokens.Peek().ValueId()))
            {
              flexBasis = ConsumeIdent(tokens);
            }

            if (!flexBasis)
            {
              flexBasis =
                CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
            }

            if (index == 2uz && !tokens.IsAtEnd())
            {
              return false;
            }
          }
        }
        if (index == 0uz)
        {
          return false;
        }

        if (!flexGrow)
        {
          flexGrow = CSSPrimitiveValue::Create(1);
        }

        if (!flexShrink)
        {
          flexShrink = CSSPrimitiveValue::Create(1);
        }

        // FIXME: Using % here is a hack to work around intrinsic sizing implementation being
        // a mess (e.g., turned off for nested column flexboxes, failing to relayout properly even
        // if turned back on for nested columns, etc.). We have layout test coverage of both
        // scenarios.
        if (!flexBasis)
        {
          flexBasis = CSSPrimitiveValue::Create(0, CSSUnitType::Percentage);
        }
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FlexGrow, Krys::Move(flexGrow));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FlexShrink, Krys::Move(flexShrink));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::FlexBasis, Krys::Move(flexBasis));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakAfterShorthand(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state,
                                                              const CSSPropertyShorthand &shorthand,
                                                              CSSPropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromPageBreakBetween(*keyword);
      if (value == CSSValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BreakAfter,
                                            CSSPrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakBeforeShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromPageBreakBetween(*keyword);
      if (value == CSSValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BreakBefore,
                                            CSSPrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakInsideShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromColumnRegionOrPageBreakInside(*keyword);
      if (value == CSSValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BreakInside,
                                            CSSPrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTransformOriginShorthand(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state,
                                                               const CSSPropertyShorthand &shorthand,
                                                               CSSPropertyParserResult &result) noexcept
    {
      if (auto position = ConsumeOneOrTwoComponentPositionUnresolved(tokens, state))
      {
        tokens.DiscardWhitespace();

        bool atEnd = tokens.IsAtEnd();
        auto resultZ = CSSPrimitiveValueResolver<Length<>>::ConsumeAndResolve(tokens, state);
        if ((!resultZ && !atEnd) || !tokens.IsAtEnd())
        {
          return false;
        }

        auto [positionX, positionY] = Split(Krys::Move(*position));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TransformOriginX,
                                              CSSPositionXValue::Create(Krys::Move(positionX)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TransformOriginY,
                                              CSSPositionYValue::Create(Krys::Move(positionY)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::TransformOriginZ, resultZ);

        return true;
      }

      return false;
    }

    KRYS_NODISCARD static bool ConsumePerspectiveOriginShorthand(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &state,
                                                                 const CSSPropertyShorthand &shorthand,
                                                                 CSSPropertyParserResult &result) noexcept
    {
      if (auto position = ConsumePositionUnresolved(tokens, state))
      {
        auto [positionX, positionY] = Split(Krys::Move(*position));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::PerspectiveOriginX,
                                              CSSPositionXValue::Create(Krys::Move(positionX)));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::PerspectiveOriginY,
                                              CSSPositionYValue::Create(Krys::Move(positionY)));
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
        CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetPosition, state);

      // Parse out offset-path.
      auto offsetPath =
        CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetPath, state);

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
               CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetDistance, state)))
          offsetRotate =
            CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetRotate, state);
        else
        {
          offsetRotate =
            CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetRotate, state);
          offsetDistance =
            CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetDistance, state);
        }
      }

      // Parse out offset-anchor. Only parse if the prefix slash is present.
      RefPtr<CSSValue> offsetAnchor;
      if (ConsumeSlash(tokens))
      {
        // offset-anchor must follow the slash.
        if (!(offsetAnchor =
                CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::OffsetAnchor, state)))
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OffsetPath, Krys::Move(offsetPath));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OffsetDistance, Krys::Move(offsetDistance));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OffsetPosition, Krys::Move(offsetPosition));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OffsetAnchor, Krys::Move(offsetAnchor));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OffsetRotate, Krys::Move(offsetRotate));

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
        if (tokens.Peek().ValueId() == CSSValueId::None)
        {
          ++noneCount;
          ConsumeIdent(tokens);
          continue;
        }

        if (!position
            && (position = CSSPropertyParsing::ParseStylePropertyLonghand(
                  tokens, CSSPropertyId::ListStylePosition, state)))
        {
          continue;
        }

        if (!image
            && (image = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::ListStyleImage,
                                                                       state)))
        {
          continue;
        }

        if (!type
            && (type = CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::ListStyleType,
                                                                      state)))
        {
          continue;
        }

        return false;
      }

      if (noneCount > (static_cast<size_t>(!image + !type)))
      {
        return false;
      }

      if (noneCount == 2uz)
      {
        // Using implicit none for list-style-image is how we serialize "none" instead of "none none".
        image = nullptr;
        type = CSSPrimitiveValue::Create(CSSValueId::None);
      }
      else if (noneCount == 1uz)
      {
        // Use implicit none for list-style-image, but non-implicit for type.
        if (!type)
        {
          type = CSSPrimitiveValue::Create(CSSValueId::None);
        }
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ListStylePosition, Krys::Move(position));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ListStyleImage, Krys::Move(image));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ListStyleType, Krys::Move(type));

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeOverscrollBehaviorShorthand(CSSTokenRange &tokens,
                                                                  CSSPropertyParserState &state,
                                                                  const CSSPropertyShorthand &shorthand,
                                                                  CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);

      if (tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr overscrollBehaviorX = CSSPropertyParsing::ConsumeOverscrollBehaviorX(tokens);
      if (!overscrollBehaviorX)
      {
        return false;
      }

      RefPtr<CSSValue> overscrollBehaviorY;
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        overscrollBehaviorY = overscrollBehaviorX;
      }
      else
      {
        overscrollBehaviorY = CSSPropertyParsing::ConsumeOverscrollBehaviorY(tokens);
        tokens.DiscardWhitespace();
        if (!tokens.IsAtEnd())
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OverscrollBehaviorX,
                                            Krys::Move(overscrollBehaviorX));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::OverscrollBehaviorY,
                                            Krys::Move(overscrollBehaviorY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainerShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      RefPtr name =
        CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::ContainerName, state);
      if (!name)
      {
        return false;
      }

      bool sawSlash = false;
      auto ConsumeSlashType = [&]() -> RefPtr<CSSValue>
      {
        if (tokens.IsAtEnd())
        {
          return nullptr;
        }

        if (!ConsumeSlash(tokens))
        {
          return nullptr;
        }

        sawSlash = true;
        return CSSPropertyParsing::ParseStylePropertyLonghand(tokens, CSSPropertyId::ContainerType, state);
      };

      auto type = ConsumeSlashType();
      if (!tokens.IsAtEnd() || (sawSlash && !type))
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ContainerName, Krys::Move(name));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ContainerType, Krys::Move(type));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainIntrinsicSizeShorthand(CSSTokenRange &tokens,
                                                                    CSSPropertyParserState &state,
                                                                    const CSSPropertyShorthand &shorthand,
                                                                    CSSPropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);
      assert(IsExposed(CSSPropertyId::ContainIntrinsicSize, &state.Context.PropertySettings));

      if (tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr containIntrinsicWidth = CSSPropertyParsing::ConsumeContainIntrinsicWidth(tokens, state);
      if (!containIntrinsicWidth)
      {
        return false;
      }

      RefPtr<CSSValue> containIntrinsicHeight;
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        containIntrinsicHeight = containIntrinsicWidth;
      }
      else
      {
        containIntrinsicHeight = CSSPropertyParsing::ConsumeContainIntrinsicHeight(tokens, state);
        tokens.DiscardWhitespace();
        if (!tokens.IsAtEnd() || !containIntrinsicHeight)
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ContainIntrinsicWidth,
                                            Krys::Move(containIntrinsicWidth));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ContainIntrinsicHeight,
                                            Krys::Move(containIntrinsicHeight));

      return true;
    }

    // KRYS_NODISCARD static bool ConsumeAnimationRangeShorthand(CSSTokenRange &tokens,
    //                                                           CSSPropertyParserState &state,
    //                                                           const CSSPropertyShorthand &shorthand,
    //                                                           CSSPropertyParserResult &result) noexcept
    //{
    //   CSSValueListBuilder startList;
    //   CSSValueListBuilder endList;
    //   do
    //   {
    //     RefPtr start = ConsumeSingleAnimationRangeStart(tokens, state);
    //     if (!start)
    //     {
    //       return false;
    //     }

    // RefPtr<CSSValue> end;
    // tokens.DiscardWhitespace();
    // if (tokens.IsAtEnd() || tokens.Peek().Type() == CSSTokenType::Comma)
    // {
    //   // From the spec: If <'animation-range-end'> is omitted and <'animation-range-start'> includes a
    //   // component, then animation-range-end is set to that same and 100%. Otherwise, any omitted longhand
    //   // is set to its initial value.
    //   auto RangeEndValueForStartValue = [](const CSSValue &value)
    //   {
    //     auto IsRangeOffset = [](auto &value)
    //     {
    //       return value.IsLength() || value.IsPercentage() || value.IsCalculatedPercentageWithLength();
    //     };

    // if (auto *primitiveValue = DynamicDowncast<CSSPrimitiveValue>(value);
    //     primitiveValue && IsRangeOffset(*primitiveValue))
    // {
    //   return CSSPrimitiveValue::Create(CSSValueId::Normal);
    // }

    // return CSSPrimitiveValue::Create(value.ValueId());
    // };

    // if (RefPtr startPrimitiveValue = DynamicDowncast<CSSPrimitiveValue>(start))
    // {
    //   end = RangeEndValueForStartValue(*startPrimitiveValue);
    // }
    // else
    // {
    //   RefPtr startPair = Downcast<CSSValuePair>(start);
    //   end = RangeEndValueForStartValue(startPair->first());
    // }
    // }
    // else
    // {
    // end = ConsumeSingleAnimationRangeEnd(tokens, state);
    // tokens.DiscardWhitespace();
    // if (!end)
    // {
    //   return false;
    // }
    // }

    // startList.push_back(Krys::Move(start));
    // endList.push_back(Krys::Move(end));
    // } while (ConsumeComma(tokens));

    // if (!tokens.IsAtEnd())
    // {
    //   return false;
    // }

    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::AnimationRangeStart,
    //                                       CSSValueList::CreateCommaSeparated(Krys::Move(startList)));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::AnimationRangeEnd,
    //                                       CSSValueList::CreateCommaSeparated(Krys::Move(endList)));

    // return true;
    // }

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
        if (auto name = CSSPropertyParsing::ConsumeSingleScrollTimelineName(tokens))
        {
          namesList.push_back(Krys::Move(name));
        }
        else
        {
          return false;
        }

        // A scroll-timeline-axis is optional.
        if (tokens.Peek().Type() == CSSTokenType::Comma || tokens.IsAtEnd())
        {
          axesList.push_back(CSSPrimitiveValue::Create(CSSValueId::Block));
        }
        else if (auto axis = CSSPropertyParsing::ConsumeAxis(tokens))
        {
          axesList.push_back(Krys::Move(axis));
        }
        else
        {
          return false;
        }
      } while (ConsumeComma(tokens));

      if (namesList.empty())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ScrollTimelineName,
                                            CSSValueList::CreateCommaSeparated(Krys::Move(namesList)));
      if (!axesList.empty())
      {
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ScrollTimelineAxis,
                                              CSSValueList::CreateCommaSeparated(Krys::Move(axesList)));
      }

      return true;
    }

    // KRYS_NODISCARD static bool ConsumeViewTimelineShorthand(CSSTokenRange &tokens,
    //                                                         CSSPropertyParserState &state,
    //                                                         const CSSPropertyShorthand &shorthand,
    //                                                         CSSPropertyParserResult &result) noexcept
    //{
    //   CSSValueListBuilder namesList;
    //   CSSValueListBuilder axesList;
    //   CSSValueListBuilder insetsList;

    // auto DefaultAxis = [] -> Ref<CSSValue>
    // {
    //   return CSSPrimitiveValue::Create(CSSValueId::Block);
    // };
    // auto DefaultInsets = [] -> Ref<CSSValue>
    // {
    //   return CSSPrimitiveValue::Create(CSSValueId::Auto);
    // };

    // do
    // {
    //   // A valid view-timeline-name is required.
    //   if (auto name = CSSPropertyParsing::ConsumeSingleScrollTimelineName(tokens))
    //   {
    //     namesList.push_back(Krys::Move(name));
    //   }
    //   else
    //   {
    //     return false;
    //   }

    // // Both a view-timeline-axis and a view-timeline-inset are optional.
    // if (tokens.Peek().Type() != CSSTokenType::Comma && !tokens.IsAtEnd())
    // {
    //   RefPtr axis = CSSPropertyParsing::ConsumeAxis(tokens);
    //   RefPtr insets = ConsumeSingleViewTimelineInsetItem(tokens, state);

    // // Since the order of view-timeline-axis and view-timeline-inset is not guaranteed, let's try
    // // view-timeline-axis again.
    // if (!axis)
    // {
    //   axis = CSSPropertyParsing::ConsumeAxis(tokens);
    // }

    // if (!axis && !insets)
    // {
    //   return false;
    // }

    // axesList.push_back(axis ? Krys::Move(axis) : DefaultAxis());
    // insetsList.push_back(insets ? Krys::Move(insets) : DefaultInsets());
    // }
    // else
    // {
    // axesList.push_back(DefaultAxis());
    // insetsList.push_back(DefaultInsets());
    // }
    // } while (ConsumeComma(tokens));

    // if (namesList.empty())
    // {
    //   return false;
    // }

    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ViewTimelineName,
    //                                       CSSValueList::CreateCommaSeparated(Krys::Move(namesList)));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ViewTimelineAxis,
    //                                       CSSValueList::CreateCommaSeparated(Krys::Move(axesList)));
    // result.AddPropertyForCurrentShorthand(state, CSSPropertyId::ViewTimelineInset,
    //                                       CSSValueList::CreateCommaSeparated(Krys::Move(insetsList)));

    // return true;
    // }

    KRYS_NODISCARD static bool ConsumeLineClampShorthand(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                                         const CSSPropertyShorthand &shorthand,
                                                         CSSPropertyParserResult &result) noexcept
    {
      assert(state.Context.PropertySettings.cssLineClampEnabled);

      if (tokens.Peek().ValueId() == CSSValueId::None)
      {
        // Sets max-lines to none, continue to auto, and block-ellipsis to none.
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaxLines,
                                              CSSPrimitiveValue::Create(CSSValueId::None));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::Continue,
                                              CSSPrimitiveValue::Create(CSSValueId::Auto));
        result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockEllipsis,
                                              CSSPrimitiveValue::Create(CSSValueId::None));

        ConsumeIdent(tokens);

        return tokens.IsAtEnd();
      }

      RefPtr<CSSValue> maxLines;
      RefPtr<CSSValue> blockEllipsis;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!maxLines && (maxLines = CSSPropertyParsing::ConsumeMaxLines(tokens, state)))
        {
          continue;
        }

        if (!blockEllipsis && (blockEllipsis = CSSPropertyParsing::ConsumeBlockEllipsis(tokens)))
        {
          continue;
        }

        // There has to be at least one valid longhand.
        return false;
      }

      if (!blockEllipsis)
      {
        blockEllipsis = CSSPrimitiveValue::Create(CSSValueId::Auto);
      }

      if (!maxLines)
      {
        maxLines = CSSPrimitiveValue::Create(CSSValueId::None);
      }

      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::MaxLines, Krys::Move(maxLines));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::Continue,
                                            CSSPrimitiveValue::Create(CSSValueId::Discard));
      result.AddPropertyForCurrentShorthand(state, CSSPropertyId::BlockEllipsis, Krys::Move(blockEllipsis));
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
      {
        textBoxEdge = CSSPrimitiveValue::Create(CSSValueId::Auto);
      }

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