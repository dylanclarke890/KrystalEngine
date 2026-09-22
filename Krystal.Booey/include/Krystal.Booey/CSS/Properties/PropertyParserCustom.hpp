#pragma once

#include "Krystal.Booey/CSS/Properties/ParsedPropertyList.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRangeGuard.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Align.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Anchor.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AnglePercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Attr.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Box.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Content.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Display.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Easing.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/FlexDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/FrequencyDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Grid.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Inline.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Lists.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Masking.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaResolver.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Motion.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PositionTry.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ratio.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Scrollbars.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ScrollSnap.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/SVG.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Syntax.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/TextDecoration.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Timeline.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Transform.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Transitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/UI.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/UnicodeRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ViewTransition.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/WillChange.hpp"
#include "Krystal.Booey/CSS/Properties/FontVariantLigaturesParser.hpp"
#include "Krystal.Booey/CSS/Properties/FontVariantNumericParser.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserResult.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthandFunctions.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"
#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/CSS/Values/TransformListValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridNamedAreaMap.hpp"
#include "Krystal.Booey/CSS/Values/GridLineNamesValue.hpp"
#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueCreation.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Utils/ZippedRange.hpp"

namespace krys::boo::css
{
  using namespace PropertyParserHelpers;

  struct BorderShorthandComponents
  {
    RefPtr<Value> Width;
    RefPtr<Value> Style;
    RefPtr<Value> Color;
  };

  KRYS_NODISCARD inline Maybe<BorderShorthandComponents>
    ConsumeBorderShorthandComponents(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    BorderShorthandComponents components {};

    while (!components.Width || !components.Style || !components.Color)
    {
      if (!components.Width)
      {
        components.Width =
          PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::BorderLeftWidth, state);

        if (components.Width)
        {
          continue;
        }
      }

      if (!components.Style)
      {
        components.Style =
          PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::BorderLeftStyle, state);

        if (components.Style)
        {
          continue;
        }
      }

      if (!components.Color)
      {
        components.Color =
          PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::BorderLeftColor, state);

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

    if (!tokens.IsAtEnd())
    {
      return {};
    }

    return components;
  }

  KRYS_NODISCARD constexpr ValueId MapFromPageBreakBetween(ValueId value) noexcept
  {
    if (value == ValueId::Always)
    {
      return ValueId::Page;
    }

    if (value == ValueId::Auto || value == ValueId::Avoid || value == ValueId::Left
        || value == ValueId::Right)
    {
      return value;
    }

    return ValueId::Invalid;
  }

  KRYS_NODISCARD constexpr ValueId MapFromColumnBreakBetween(ValueId value) noexcept
  {
    if (value == ValueId::Always)
    {
      return ValueId::Column;
    }

    if (value == ValueId::Auto)
    {
      return value;
    }

    if (value == ValueId::Avoid)
    {
      return ValueId::AvoidColumn;
    }

    return ValueId::Invalid;
  }

  KRYS_NODISCARD constexpr ValueId MapFromColumnRegionOrPageBreakInside(ValueId value) noexcept
  {
    if (value == ValueId::Auto || value == ValueId::Avoid)
    {
      return value;
    }

    return ValueId::Invalid;
  }

  class PropertyParserCustom
  {
  public:
    KRYS_NODISCARD static bool ConsumeStandardSpaceSeparatedShorthand(TokenRange &tokens,
                                                                      PropertyParserState &state,
                                                                      const PropertyShorthand &shorthand,
                                                                      PropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() <= 6); // Existing shorthands have at most 6 longhands.

      Array<RefPtr<Value>, 6> longhands;
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

          longhands[i] = PropertyParsing::ParseStylePropertyLonghand(tokens, shorthandProperties[i], state);

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
        result.AddPropertyForCurrentShorthand(state, shorthandProperties[i], krys::move(longhands[i]));
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeCoalescingPairShorthand(TokenRange &tokens, PropertyParserState &state,
                                                              const PropertyShorthand &shorthand,
                                                              PropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() == 2uz);

      auto longhands = shorthand.Properties();
      auto start = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!start)
      {
        return false;
      }

      auto end = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      auto endImplicit = IsImplicit(!end);
      if (endImplicit)
      {
        end = start;
      }

      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(start));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(end), endImplicit);

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeCoalescingQuadShorthand(TokenRange &tokens, PropertyParserState &state,
                                                              const PropertyShorthand &shorthand,
                                                              PropertyParserResult &result) noexcept
    {
      assert(state.CurrentProperty == shorthand.Id());
      assert(shorthand.Size() == 4uz);

      auto longhands = shorthand.Properties();
      auto top = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!top)
      {
        return false;
      }

      RefPtr right = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      RefPtr<Value> bottom;
      RefPtr<Value> left;
      if (right)
      {
        bottom = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[2], state);
        if (bottom)
        {
          left = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[3], state);
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

      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(top));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(right), rightImplicit);
      result.AddPropertyForCurrentShorthand(state, longhands[2], krys::move(bottom), bottomImplicit);
      result.AddPropertyForCurrentShorthand(state, longhands[3], krys::move(left), leftImplicit);

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeBorderShorthand(TokenRange &tokens, PropertyParserState &state,
                                                      const PropertyShorthand &shorthand,
                                                      PropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderWidth,
                                                   krys::move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderStyle,
                                                   krys::move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderColor,
                                                   krys::move(components->Color), state.Important);

      for (auto longhand : BorderImageShorthand())
      {
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr);
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderInlineShorthand(TokenRange &tokens, PropertyParserState &state,
                                                            const PropertyShorthand &shorthand,
                                                            PropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderInlineWidth,
                                                   krys::move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderInlineStyle,
                                                   krys::move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderInlineColor,
                                                   krys::move(components->Color), state.Important);

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderImageShorthand(TokenRange &tokens, PropertyParserState &state,
                                                           const PropertyShorthand &shorthand,
                                                           PropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderImageComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderImageSource,
                                            krys::move(components->Source));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderImageSlice,
                                            krys::move(components->Slice));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderImageWidth,
                                            krys::move(components->Width));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderImageOutset,
                                            krys::move(components->Outset));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderImageRepeat,
                                            krys::move(components->Repeat));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderBlockShorthand(TokenRange &tokens, PropertyParserState &state,
                                                           const PropertyShorthand &shorthand,
                                                           PropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderShorthandComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderBlockWidth,
                                                   krys::move(components->Width), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderBlockStyle,
                                                   krys::move(components->Style), state.Important);
      result.AddPropertyForAllLonghandsOfShorthand(state, PropertyId::BorderBlockColor,
                                                   krys::move(components->Color), state.Important);
      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderRadiusShorthand(TokenRange &tokens, PropertyParserState &state,
                                                            const PropertyShorthand &shorthand,
                                                            PropertyParserResult &result) noexcept
    {
      auto borderRadius = ConsumeUnresolvedBorderRadius(tokens, state);
      if (!borderRadius.has_value())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderTopLeftRadius,
                                            CreateValue(state.ValuePool, borderRadius->TopLeft()));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderTopRightRadius,
                                            CreateValue(state.ValuePool, borderRadius->TopRight()));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderBottomRightRadius,
                                            CreateValue(state.ValuePool, borderRadius->BottomRight()));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BorderBottomLeftRadius,
                                            CreateValue(state.ValuePool, borderRadius->BottomLeft()));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBorderSpacingShorthand(TokenRange &tokens, PropertyParserState &state,
                                                             const PropertyShorthand &,
                                                             PropertyParserResult &result)
    {
      RefPtr horizontalSpacing =
        PrimitiveValueResolver<Length<NonNegative>>::ConsumeAndResolve(tokens, state);
      if (!horizontalSpacing)
      {
        return false;
      }

      RefPtr verticalSpacing = horizontalSpacing;
      if (!tokens.IsAtEnd())
      {
        verticalSpacing = PrimitiveValueResolver<Length<NonNegative>>::ConsumeAndResolve(tokens, state);
      }

      if (!verticalSpacing || !tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::WebkitBorderHorizontalSpacing,
                                            krys::move(horizontalSpacing));
      result.AddPropertyForCurrentShorthand(state, PropertyId::WebkitBorderVerticalSpacing,
                                            krys::move(verticalSpacing));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeAnimationShorthand(TokenRange &tokens, PropertyParserState &state,
                                                         const PropertyShorthand &shorthand,
                                                         PropertyParserResult &result) noexcept
    {
      auto ConsumeAnimationValueForShorthand = [&](PropertyId property) -> RefPtr<Value>
      {
        switch (property)
        {
          case PropertyId::AnimationDelay:
          {
            return PrimitiveValueResolver<Time<>>::ConsumeAndResolve(tokens, state);
          }
          case PropertyId::AnimationDirection:
          {
            return PropertyParsing::ConsumeSingleAnimationDirection(tokens);
          }
          case PropertyId::AnimationDuration:
          {
            return PropertyParsing::ConsumeSingleAnimationDuration(tokens, state);
          }
          case PropertyId::AnimationFillMode:
          {
            return PropertyParsing::ConsumeSingleAnimationFillMode(tokens);
          }
          case PropertyId::AnimationIterationCount:
          {
            return PropertyParsing::ConsumeSingleAnimationIterationCount(tokens, state);
          }
          case PropertyId::AnimationName:
          {
            return PropertyParsing::ConsumeSingleAnimationName(tokens, state);
          }
          case PropertyId::AnimationPlayState:
          {
            return PropertyParsing::ConsumeSingleAnimationPlayState(tokens);
          }
          case PropertyId::AnimationComposition:
          {
            return PropertyParsing::ConsumeSingleAnimationComposition(tokens);
          }
          case PropertyId::AnimationTimeline:
          case PropertyId::AnimationRangeStart:
          case PropertyId::AnimationRangeEnd:
          {
            return nullptr; // reset-only longhands
          }
          case PropertyId::AnimationTimingFunction:
          {
            // TODO: Implement ConsumeEasingFunction
            return nullptr;
            // return ConsumeEasingFunction(tokens, state);
          }
          case PropertyId::TransitionBehavior:
          {
            return PropertyParsing::ConsumeTransitionBehaviorValue(tokens);
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
      Array<ValueListBuilder, maxLonghandCount> longhands;

      auto IsResetOnlyLonghand = [](PropertyId longhand)
      {
        switch (longhand)
        {
          case PropertyId::AnimationTimeline:
          case PropertyId::AnimationRangeStart:
          case PropertyId::AnimationRangeEnd:
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
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma);

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i] && !IsResetOnlyLonghand(shorthandProperties[i]))
          {
            longhands[i].push_back(ShareRef(PrimitiveValue::ImplicitInitialValue()));
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
                                                ValueList::CreateCommaSeparated(krys::move(list)));
        }
      }

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeTransitionShorthand(TokenRange &tokens, PropertyParserState &state,
                                                          const PropertyShorthand &shorthand,
                                                          PropertyParserResult &result) noexcept
    {
      auto IsValidTransitionPropertyList = [](PropertyId property, const ValueListBuilder &valueList)
      {
        // If there is more than one <single-transition> in the shorthand, and any of the transitions
        // has none as the <single-transition-property>, then the declaration is invalid.
        if (property != PropertyId::TransitionProperty || valueList.size() < 2)
        {
          return true;
        }

        for (auto &value : valueList)
        {
          if (IsValueId(value, ValueId::None))
          {
            return false;
          }
        }

        return true;
      };

      auto ConsumeTransitionValueForShorthand = [&](PropertyId property) -> RefPtr<Value>
      {
        switch (property)
        {
          case PropertyId::TransitionDelay:
          {
            return PrimitiveValueResolver<Time<>>::ConsumeAndResolve(tokens, state);
          }
          case PropertyId::TransitionDuration:
          {
            return PrimitiveValueResolver<Time<NonNegative>>::ConsumeAndResolve(tokens, state);
          }
          case PropertyId::TransitionProperty:
          {
            return nullptr;
            // return ConsumeSingleTransitionPropertyOrNone(tokens, state);
          }
          case PropertyId::TransitionTimingFunction:
          {
            return nullptr;
            // return ConsumeEasingFunction(tokens, state);
          }
          case PropertyId::TransitionBehavior:
          {
            return PropertyParsing::ConsumeTransitionBehaviorValue(tokens);
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

      Array<ValueListBuilder, maxLonghandCount> longhands;
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
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma);

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          if (!parsedLonghand[i])
          {
            longhands[i].push_back(ShareRef(PrimitiveValue::ImplicitInitialValue()));
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
                                              ValueList::CreateCommaSeparated(krys::move(longhands[i])));
      }

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeBackgroundShorthand(TokenRange &tokens, PropertyParserState &state,
                                                          const PropertyShorthand &shorthand,
                                                          PropertyParserResult &result) noexcept
    {
      assert(shorthand.Id() == state.CurrentProperty);

      auto ConsumeBackgroundComponent = [&](PropertyId property) -> RefPtr<Value>
      {
        switch (property)
        {
          // background-*
          case PropertyId::BackgroundClip:
          {
            return PropertyParsing::ConsumeSingleBackgroundClip(tokens, state);
          }
          case PropertyId::BackgroundBlendMode:
          {
            return PropertyParsing::ConsumeSingleBackgroundBlendMode(tokens);
          }
          case PropertyId::BackgroundAttachment:
          {
            return PropertyParsing::ConsumeSingleBackgroundAttachment(tokens);
          }
          case PropertyId::BackgroundOrigin:
          {
            return PropertyParsing::ConsumeSingleBackgroundOrigin(tokens);
          }
          case PropertyId::BackgroundImage:
          {
            return PropertyParsing::ConsumeSingleBackgroundImage(tokens, state);
          }
          case PropertyId::BackgroundRepeat:
          {
            return PropertyParsing::ConsumeSingleBackgroundRepeat(tokens, state);
          }
          case PropertyId::BackgroundPositionX:
          {
            return PropertyParsing::ConsumeSingleBackgroundPositionX(tokens, state);
          }
          case PropertyId::BackgroundPositionY:
          {
            return PropertyParsing::ConsumeSingleBackgroundPositionY(tokens, state);
          }
          case PropertyId::BackgroundSize:
          {
            return ConsumeSingleBackgroundSize(tokens, state);
          }
          case PropertyId::BackgroundColor:
          {
            return ConsumeColor(tokens, state);
          }

          // mask-*
          case PropertyId::MaskComposite:
          {
            return PropertyParsing::ConsumeSingleMaskComposite(tokens);
          }
          case PropertyId::MaskOrigin:
          {
            return PropertyParsing::ConsumeSingleMaskOrigin(tokens);
          }
          case PropertyId::MaskClip:
          {
            return PropertyParsing::ConsumeSingleMaskClip(tokens);
          }
          case PropertyId::MaskImage:
          {
            return PropertyParsing::ConsumeSingleMaskImage(tokens, state);
          }
          case PropertyId::MaskMode:
          {
            return PropertyParsing::ConsumeSingleMaskMode(tokens);
          }
          case PropertyId::MaskRepeat:
          {
            return PropertyParsing::ConsumeSingleMaskRepeat(tokens, state);
          }
          case PropertyId::MaskSize:
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
      if (shorthand.Id() == PropertyId::Mask)
      {
        longhandCount -= MaskBorderShorthand().Size();
      }

      Array<ValueListBuilder, 10uz> longhands;
      assert(longhandCount <= 10uz);

      do
      {
        bool lastParsedWasPosition = false;
        bool clipIsBorderArea = false;

        Array<bool, 10uz> parsedLonghand = {};
        RefPtr<Value> originValue;
        do
        {
          bool foundProperty = false;
          for (size_t i = 0uz; i < longhandCount; ++i)
          {
            if (parsedLonghand[i])
              continue;

            RefPtr<Value> value;
            RefPtr<Value> valueY;
            PropertyId property = shorthandProperties[i];

            if (property == PropertyId::BackgroundPositionX)
            {
              // Note: This assumes y properties (for example background-position-y) follow the x properties
              // in the shorthand array.
              auto position = ConsumeBackgroundPositionUnresolved(tokens, state);
              if (!position)
              {
                continue;
              }

              auto [positionX, positionY] = Split(krys::move(*position));
              value = PositionXValue::Create(krys::move(positionX));
              valueY = PositionYValue::Create(krys::move(positionY));
            }
            else if (property == PropertyId::BackgroundSize)
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
            else if (property == PropertyId::MaskSize)
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
            else if (property == PropertyId::BackgroundPositionY)
            {
              continue;
            }
            else
            {
              value = ConsumeBackgroundComponent(property);
            }
            if (value)
            {
              if (property == PropertyId::BackgroundOrigin || property == PropertyId::MaskOrigin)
              {
                originValue = value;
              }
              else if (property == PropertyId::BackgroundClip)
              {
                clipIsBorderArea = value->ValueId() == ValueId::BorderArea;
              }

              parsedLonghand[i] = true;
              foundProperty = true;

              longhands[i].push_back(krys::move(value));
              lastParsedWasPosition = valueY != nullptr;
              if (valueY)
              {
                parsedLonghand[i + 1uz] = true;
                longhands[i + 1uz].push_back(krys::move(valueY));
              }
            }
          }

          if (!foundProperty)
          {
            return false;
          }
        } while (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma);

        for (size_t i = 0uz; i < longhandCount; ++i)
        {
          auto property = shorthandProperties[i];
          if (property == PropertyId::BackgroundColor && !tokens.IsAtEnd())
          {
            if (parsedLonghand[i])
            {
              return false; // Colors are only allowed in the last layer.
            }
            continue;
          }

          if ((property == PropertyId::BackgroundClip || property == PropertyId::MaskClip)
              && !parsedLonghand[i] && originValue)
          {
            longhands[i].push_back(krys::move(originValue));
            continue;
          }

          if (clipIsBorderArea && (property == PropertyId::BackgroundOrigin) && !parsedLonghand[i])
          {
            longhands[i].push_back(PrimitiveValue::Create(ValueId::BorderBox));
            continue;
          }

          if (!parsedLonghand[i])
          {
            longhands[i].push_back(ShareRef(PrimitiveValue::ImplicitInitialValue()));
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
          result.AddPropertyForCurrentShorthand(state, property, krys::move(longhands[i][0]));
        }
        else
        {
          result.AddPropertyForCurrentShorthand(state, property,
                                                ValueList::CreateCommaSeparated(krys::move(longhands[i])));
        }
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBackgroundPositionShorthand(TokenRange &tokens,
                                                                  PropertyParserState &state,
                                                                  const PropertyShorthand &shorthand,
                                                                  PropertyParserResult &result) noexcept
    {
      assert(shorthand.Id() == state.CurrentProperty);

      ValueListBuilder x;
      ValueListBuilder y;
      do
      {
        auto position = ConsumeBackgroundPositionUnresolved(tokens, state);
        if (!position)
        {
          return false;
        }

        auto [positionX, positionY] = Split(krys::move(*position));
        x.push_back(PositionXValue::Create(krys::move(positionX)));
        y.push_back(PositionYValue::Create(krys::move(positionY)));
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr<Value> resultX;
      RefPtr<Value> resultY;
      if (x.size() == 1uz)
      {
        resultX = krys::move(x[0]);
        resultY = krys::move(y[0]);
      }
      else
      {
        resultX = ValueList::CreateCommaSeparated(krys::move(x));
        resultY = ValueList::CreateCommaSeparated(krys::move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(resultX));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(resultY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeWebkitBackgroundSizeShorthand(TokenRange &tokens,
                                                                    PropertyParserState &state,
                                                                    const PropertyShorthand &shorthand,
                                                                    PropertyParserResult &result) noexcept
    {
      auto backgroundSize = ConsumeListSeparatedBy<',', OneOrMore, ListOptimization::SingleValue>(
        tokens, [](auto &tokens, auto &state) { return ConsumeSingleWebkitBackgroundSize(tokens, state); },
        state);

      if (!backgroundSize || !tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BackgroundSize, krys::move(backgroundSize));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeMaskShorthand(TokenRange &tokens, PropertyParserState &state,
                                                    const PropertyShorthand &shorthand,
                                                    PropertyParserResult &result) noexcept
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

    KRYS_NODISCARD static bool ConsumeMaskPositionShorthand(TokenRange &tokens, PropertyParserState &state,
                                                            const PropertyShorthand &shorthand,
                                                            PropertyParserResult &result) noexcept
    {
      ValueListBuilder x;
      ValueListBuilder y;
      do
      {
        auto position = ConsumePositionUnresolved(tokens, state);
        if (!position)
          return false;
        auto [positionX, positionY] = Split(krys::move(*position));
        x.push_back(PositionXValue::Create(krys::move(positionX)));
        y.push_back(PositionYValue::Create(krys::move(positionY)));
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr<Value> resultX;
      RefPtr<Value> resultY;
      if (x.size() == 1uz)
      {
        resultX = krys::move(x[0]);
        resultY = krys::move(y[0]);
      }
      else
      {
        resultX = ValueList::CreateCommaSeparated(krys::move(x));
        resultY = ValueList::CreateCommaSeparated(krys::move(y));
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(resultX));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(resultY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeOverflowShorthand(TokenRange &tokens, PropertyParserState &state,
                                                        const PropertyShorthand &shorthand,
                                                        PropertyParserResult &result) noexcept
    {
      ValueId xValueId = tokens.Consume().ValueId();
      tokens.DiscardWhitespace();

      if (!PropertyParsing::IsKeywordValidForStyleProperty(PropertyId::OverflowY, xValueId, state))
      {
        return false;
      }

      ValueId yValueId;
      if (tokens.IsAtEnd())
      {
        yValueId = xValueId;
      }
      else
      {
        yValueId = tokens.Consume().ValueId();
        tokens.DiscardWhitespace();
      }

      if (!PropertyParsing::IsKeywordValidForStyleProperty(PropertyId::OverflowY, yValueId, state))
      {
        return false;
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::OverflowX,
                                            PrimitiveValue::Create(xValueId));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OverflowY,
                                            PrimitiveValue::Create(yValueId));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeColumnsShorthand(TokenRange &tokens, PropertyParserState &state,
                                                       const PropertyShorthand &shorthand,
                                                       PropertyParserResult &result) noexcept
    {
      RefPtr<Value> columnWidth;
      RefPtr<Value> columnCount;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (tokens.Peek().ValueId() == ValueId::Auto)
        {
          // 'auto' is a valid value for any of the two longhands, and at this point
          // we don't know which one(s) it is meant for. We need to see if there are other values first.
          DiscardIdent(tokens);
        }
        else
        {
          if (!columnWidth
              && (columnWidth =
                    PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ColumnWidth, state)))
          {
            continue;
          }

          if (!columnCount
              && (columnCount =
                    PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ColumnCount, state)))
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

      result.AddPropertyForCurrentShorthand(state, PropertyId::ColumnWidth, krys::move(columnWidth));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ColumnCount, krys::move(columnCount));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridItemPositionShorthand(TokenRange &tokens,
                                                                PropertyParserState &state,
                                                                const PropertyShorthand &shorthand,
                                                                PropertyParserResult &result) noexcept
    {
      assert(shorthand.Id() == state.CurrentProperty);
      assert(shorthand.Size() == 2uz);

      RefPtr<Value> startValue = ConsumeGridLine(tokens, state);
      if (!startValue)
      {
        return false;
      }

      RefPtr<Value> endValue;
      if (ConsumeSlash(tokens))
      {
        endValue = ConsumeGridLine(tokens, state);
        if (!endValue)
        {
          return false;
        }
      }
      else
      {
        if (IsCustomIdentValue(*startValue))
        {
          endValue = startValue;
        }
        else
        {
          endValue = PrimitiveValue::Create(ValueId::Auto);
        }
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      auto longhands = shorthand.Properties();
      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(startValue));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(endValue));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridTemplateShorthand(TokenRange &tokens, PropertyParserState &state,
                                                            const PropertyShorthand &shorthand,
                                                            PropertyParserResult &result) noexcept
    {
      TokenRange rangeCopy = tokens;
      RefPtr<Value> rowsValue = ConsumeIdent<ValueId::None>(rangeCopy);

      // 1- 'none' case.
      if (rowsValue && tokens.IsAtEnd())
      {
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateRows,
                                              PrimitiveValue::Create(ValueId::None));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateColumns,
                                              PrimitiveValue::Create(ValueId::None));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateAreas,
                                              PrimitiveValue::Create(ValueId::None));

        return true;
      }

      // 2- <grid-template-rows> / <grid-template-columns>
      if (!rowsValue)
      {
        rowsValue = ConsumeGridTrackList(tokens, state, GridTemplate);
      }

      if (rowsValue)
      {
        if (!ConsumeSlash(tokens))
        {
          return false;
        }

        RefPtr columnsValue = ConsumeGridTemplatesRowsOrColumns(tokens, state);
        if (!columnsValue || !tokens.IsAtEnd())
        {
          return false;
        }

        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateRows, krys::move(rowsValue));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateColumns,
                                              krys::move(columnsValue));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateAreas,
                                              PrimitiveValue::Create(ValueId::None));

        return true;
      }

      // // 3- [ <line-names>? <string> <track-size>? <line-names>? ]+ [ / <track-list> ]?

      // tokens = rangeCopy;

      GridNamedAreaMap gridAreaMap;
      ValueListBuilder templateRows;

      // Persists between loop iterations so we can use the same value for
      // consecutive <line-names> values
      RefPtr<GridLineNamesValue> lineNames;

      do
      {
        // Handle leading <custom-ident>*.
        auto previousLineNames = std::exchange(lineNames, ConsumeGridLineNames(tokens, state));
        if (lineNames)
        {
          if (!previousLineNames)
          {
            templateRows.push_back(krys::move(lineNames));
          }
          else
          {
            SmallList<CSSOMString> combinedLineNames;
            combinedLineNames.append(previousLineNames->Names().begin(), previousLineNames->Names().end());
            combinedLineNames.append(lineNames->Names().begin(), lineNames->Names().end());
            templateRows.back() = GridLineNamesValue::Create(combinedLineNames);
          }
        }

        // Handle a template-area's row.
        auto row = ConsumeUnresolvedGridTemplateAreasRow(tokens, state);
        if (!row || !AddRow(gridAreaMap, *row))
        {
          return false;
        }

        // Handle template-rows's track-size.
        if (RefPtr value = ConsumeGridTrackSize(tokens, state))
        {
          templateRows.push_back(krys::move(value));
        }
        else
        {
          templateRows.push_back(PrimitiveValue::Create(ValueId::Auto));
        }

        // This will handle the trailing/leading <custom-ident>* in the grammar.
        lineNames = ConsumeGridLineNames(tokens, state);
        if (lineNames)
        {
          templateRows.push_back(krys::move(lineNames));
        }
      } while (!tokens.IsAtEnd()
               && !(tokens.Peek().Type() == TokenType::Delim && tokens.Peek().IdentCodePoints() == u8"/"));

      RefPtr<Value> columnsValue;
      if (!tokens.IsAtEnd())
      {
        if (!ConsumeSlash(tokens))
        {
          return false;
        }

        columnsValue = ConsumeGridTrackList(tokens, state, GridTemplateNoRepeat);
        if (!columnsValue || !tokens.IsAtEnd())
        {
          return false;
        }
      }
      else
      {
        columnsValue = PrimitiveValue::Create(ValueId::None);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateRows,
                                            ValueList::CreateSpaceSeparated(krys::move(templateRows)));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateColumns, krys::move(columnsValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateAreas,
                                            GridTemplateAreasValue::Create({krys::move(gridAreaMap)}));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridShorthand(TokenRange &tokens, PropertyParserState &state,
                                                    const PropertyShorthand &shorthand,
                                                    PropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 6);

      auto ConsumeImplicitGridAutoFlow = [](TokenRange &tokens, ValueId flowDirection) -> RefPtr<Value>
      {
        // [ auto-flow && dense? ]
        bool autoFlow = ConsumeIdentRaw<ValueId::AutoFlow>(tokens).has_value();
        bool dense = ConsumeIdentRaw<ValueId::Dense>(tokens).has_value();
        if (!autoFlow && (!dense || !ConsumeIdentRaw<ValueId::AutoFlow>(tokens)))
        {
          return nullptr;
        }

        if (!dense)
        {
          return ValueList::CreateSpaceSeparated(PrimitiveValue::Create(flowDirection));
        }

        if (flowDirection == ValueId::Row)
        {
          return ValueList::CreateSpaceSeparated(PrimitiveValue::Create(ValueId::Dense));
        }

        return ValueList::CreateSpaceSeparated(PrimitiveValue::Create(flowDirection),
                                                  PrimitiveValue::Create(ValueId::Dense));
      };

      TokenRange rangeCopy = tokens;

      // 1- <grid-template>
      if (ConsumeGridTemplateShorthand(tokens, state, GridTemplateShorthand(), result))
      {
        // It can only be specified the explicit or the implicit grid properties in a single grid
        // declaration .
        // The sub-properties not specified are set to their initial value, as normal for shorthands.
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoFlow,
                                              PrimitiveValue::Create(ValueId::Row));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoColumns,
                                              PrimitiveValue::Create(ValueId::Auto));
        result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoRows,
                                              PrimitiveValue::Create(ValueId::Auto));

        return true;
      }

      tokens = rangeCopy;

      RefPtr<Value> autoColumnsValue;
      RefPtr<Value> autoRowsValue;
      RefPtr<Value> templateRows;
      RefPtr<Value> templateColumns;
      RefPtr<Value> gridAutoFlow;

      if (tokens.Peek().ValueId() == ValueId::AutoFlow || tokens.Peek().ValueId() == ValueId::Dense)
      {
        // 2- [ auto-flow && dense? ] <grid-auto-rows>? / <grid-template-columns>
        gridAutoFlow = ConsumeImplicitGridAutoFlow(tokens, ValueId::Row);
        if (!gridAutoFlow || tokens.IsAtEnd())
          return false;
        if (ConsumeSlash(tokens))
          autoRowsValue = PrimitiveValue::Create(ValueId::Auto);
        else
        {
          autoRowsValue = ConsumeGridTrackList(tokens, state, GridAuto);
          if (!autoRowsValue)
          {
            return false;
          }

          if (!ConsumeSlash(tokens))
          {
            return false;
          }
        }

        if (tokens.IsAtEnd())
        {
          return false;
        }

        templateColumns = ConsumeGridTemplatesRowsOrColumns(tokens, state);
        if (!templateColumns)
        {
          return false;
        }

        templateRows = PrimitiveValue::Create(ValueId::None);
        autoColumnsValue = PrimitiveValue::Create(ValueId::Auto);
      }
      else
      {
        // 3- <grid-template-rows> / [ auto-flow && dense? ] <grid-auto-columns>?
        templateRows = ConsumeGridTemplatesRowsOrColumns(tokens, state);
        if (!templateRows)
        {
          return false;
        }

        if (!ConsumeSlash(tokens) || tokens.IsAtEnd())
        {
          return false;
        }

        gridAutoFlow = ConsumeImplicitGridAutoFlow(tokens, ValueId::Column);
        if (!gridAutoFlow)
        {
          return false;
        }

        if (tokens.IsAtEnd())
        {
          autoColumnsValue = PrimitiveValue::Create(ValueId::Auto);
        }
        else
        {
          autoColumnsValue = ConsumeGridTrackList(tokens, state, GridAuto);
          if (!autoColumnsValue)
          {
            return false;
          }
        }

        templateColumns = PrimitiveValue::Create(ValueId::None);
        autoRowsValue = PrimitiveValue::Create(ValueId::Auto);
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      // It can only be specified the explicit or the implicit grid properties in a single grid declaration.
      // The sub-properties not specified are set to their initial value, as normal for shorthands.
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateColumns,
                                            krys::move(templateColumns));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateRows, krys::move(templateRows));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridTemplateAreas,
                                            PrimitiveValue::Create(ValueId::None));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoFlow, krys::move(gridAutoFlow));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoColumns, krys::move(autoColumnsValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridAutoRows, krys::move(autoRowsValue));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeGridAreaShorthand(TokenRange &tokens, PropertyParserState &state,
                                                        const PropertyShorthand &shorthand,
                                                        PropertyParserResult &result) noexcept
    {
      RefPtr rowStartValue = ConsumeGridLine(tokens, state);
      if (!rowStartValue)
      {
        return false;
      }

      RefPtr<Value> columnStartValue;
      RefPtr<Value> rowEndValue;
      RefPtr<Value> columnEndValue;

      if (ConsumeSlash(tokens))
      {
        columnStartValue = ConsumeGridLine(tokens, state);
        if (!columnStartValue)
        {
          return false;
        }

        if (ConsumeSlash(tokens))
        {
          rowEndValue = ConsumeGridLine(tokens, state);
          if (!rowEndValue)
          {
            return false;
          }

          if (ConsumeSlash(tokens))
          {
            columnEndValue = ConsumeGridLine(tokens, state);
            if (!columnEndValue)
            {
              return false;
            }
          }
        }
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      bool rowStartIsCustomIdent = IsCustomIdentValue(*rowStartValue);
      if (!columnStartValue)
      {
        if (rowStartIsCustomIdent)
        {
          columnStartValue = rowStartValue;
        }
        else
        {
          columnStartValue = PrimitiveValue::Create(ValueId::Auto);
        }
      }

      if (!rowEndValue)
      {
        if (rowStartIsCustomIdent)
        {
          rowEndValue = rowStartValue;
        }
        else
        {
          rowEndValue = PrimitiveValue::Create(ValueId::Auto);
        }
      }

      if (!columnEndValue)
      {
        if (IsCustomIdentValue(*columnStartValue))
        {
          columnEndValue = rowStartValue;
        }
        else
        {
          columnEndValue = PrimitiveValue::Create(ValueId::Auto);
        }
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::GridRowStart, krys::move(rowStartValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridColumnStart, krys::move(columnStartValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridRowEnd, krys::move(rowEndValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::GridColumnEnd, krys::move(columnEndValue));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeAlignShorthand(TokenRange &tokens, PropertyParserState &state,
                                                     const PropertyShorthand &shorthand,
                                                     PropertyParserResult &result) noexcept
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

      RefPtr prop1 = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[0], state);
      if (!prop1)
      {
        return false;
      }

      // If there are no more tokens, that prop2 should use re-use the original tokens. This is the equivalent
      // of copying and validating prop1.
      if (tokens.IsAtEnd())
      {
        tokens = rangeCopy;
      }

      RefPtr prop2 = PropertyParsing::ParseStylePropertyLonghand(tokens, longhands[1], state);
      if (!prop2 || !tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, longhands[0], krys::move(prop1));
      result.AddPropertyForCurrentShorthand(state, longhands[1], krys::move(prop2));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeBlockStepShorthand(TokenRange &tokens, PropertyParserState &state,
                                                         const PropertyShorthand &shorthand,
                                                         PropertyParserResult &result) noexcept
    {
      // https://drafts.csswg.org/css-rhythm/#block-step
      RefPtr<Value> size;
      RefPtr<Value> insert;
      RefPtr<Value> align;
      RefPtr<Value> round;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 4uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!size && (size = PropertyParsing::ConsumeBlockStepSize(tokens, state)))
        {
          continue;
        }

        if (!insert && (insert = PropertyParsing::ConsumeBlockStepInsert(tokens)))
        {
          continue;
        }

        if (!align && (align = PropertyParsing::ConsumeBlockStepAlign(tokens)))
        {
          continue;
        }

        if (!round && (round = PropertyParsing::ConsumeBlockStepRound(tokens)))
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
        size = PrimitiveValue::Create(ValueId::None);
      }

      if (!insert)
      {
        insert = PrimitiveValue::Create(ValueId::MarginBox);
      }

      if (!align)
      {
        align = PrimitiveValue::Create(ValueId::Auto);
      }

      if (!round)
      {
        round = PrimitiveValue::Create(ValueId::Up);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BlockStepSize, krys::move(size));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BlockStepInsert, krys::move(insert));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BlockStepAlign, krys::move(align));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BlockStepRound, krys::move(round));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontShorthand(TokenRange &tokens, PropertyParserState &state,
                                                    const PropertyShorthand &shorthand,
                                                    PropertyParserResult &result) noexcept
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
      // result.AddPropertyForAllLonghandsOfCurrentShorthand(state, PrimitiveValue::Create(systemFont),
      //                                                     IsImplicit(true));

      // return true;
      // }

      TokenRangeGuard guard {tokens};

      Array<RefPtr<Value>, 7> values;
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
        if (ConsumeIdent<ValueId::Normal>(tokens))
        {
          continue;
        }

        if (!fontStyle
            && (fontStyle =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontStyle, state)))
        {
          continue;
        }

        if (!fontVariantCaps && (fontVariantCaps = ConsumeIdent<ValueId::SmallCaps>(tokens)))
        {
          continue;
        }

        if (!fontWeight
            && (fontWeight =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontWeight, state)))
        {
          continue;
        }

        if (!fontWidth && (fontWidth = PropertyParsing::ConsumeFontWidthAbsolute(tokens)))
        {
          continue;
        }

        break;
      }

      if (tokens.IsAtEnd())
      {
        return false;
      }

      fontSize = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontSize, state);
      if (!fontSize || tokens.IsAtEnd())
      {
        return false;
      }

      if (ConsumeSlash(tokens))
      {
        if (!ConsumeIdent<ValueId::Normal>(tokens))
        {
          lineHeight = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::LineHeight, state);
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

      fontFamily = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontFamily, state);
      if (!fontFamily || !tokens.IsAtEnd())
      {
        return false;
      }

      guard.Commit();

      auto shorthandProperties = shorthand.Properties();
      for (auto [value, longhand] : zippedRange(values, shorthandProperties.first(values.size())))
      {
        result.AddPropertyForCurrentShorthand(state, longhand, krys::move(value), IsImplicit(true));
      }

      for (auto longhand : shorthandProperties.subspan(values.size()))
      {
        result.AddPropertyForCurrentShorthand(state, longhand, nullptr, IsImplicit(true));
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontVariantShorthand(TokenRange &tokens, PropertyParserState &state,
                                                           const PropertyShorthand &shorthand,
                                                           PropertyParserResult &result) noexcept
    {
      if (IdentMatches<ValueId::Normal, ValueId::None>(tokens.Peek().ValueId()))
      {
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantLigatures, ConsumeIdent(tokens));
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantCaps, nullptr);
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantAlternates, nullptr);
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantNumeric, nullptr);
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantEastAsian, nullptr);
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantPosition, nullptr);
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantEmoji, nullptr);

        return tokens.IsAtEnd();
      }

      RefPtr<Value> capsValue;
      RefPtr<Value> alternatesValue;
      RefPtr<Value> positionValue;
      RefPtr<Value> eastAsianValue;
      RefPtr<Value> emojiValue;

      FontVariantLigaturesParser ligaturesParser;
      FontVariantNumericParser numericParser;

      auto implicitLigatures = IsImplicit(true);
      auto implicitNumeric = IsImplicit(true);
      do
      {
        if (tokens.Peek().ValueId() == ValueId::Normal)
        {
          return false;
        }

        if (!capsValue
            && (capsValue =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontVariantCaps, state)))
        {
          continue;
        }

        if (!positionValue
            && (positionValue = PropertyParsing::ParseStylePropertyLonghand(
                  tokens, PropertyId::FontVariantPosition, state)))
        {
          continue;
        }

        if (!alternatesValue
            && (alternatesValue = PropertyParsing::ParseStylePropertyLonghand(
                  tokens, PropertyId::FontVariantAlternates, state)))
        {
          continue;
        }

        auto ligaturesParseResult = ligaturesParser.ConsumeLigature(tokens);
        auto numericParseResult = numericParser.ConsumeNumeric(tokens);

        if (ligaturesParseResult == FontVariantLigaturesParser::ParseResult::ConsumedValue)
        {
          implicitLigatures = IsImplicit(false);
          continue;
        }
        if (numericParseResult == FontVariantNumericParser::ParseResult::ConsumedValue)
        {
          implicitNumeric = IsImplicit(false);
          continue;
        }

        if (ligaturesParseResult == FontVariantLigaturesParser::ParseResult::DisallowedValue
            || numericParseResult == FontVariantNumericParser::ParseResult::DisallowedValue)
        {
          return false;
        }

        if (!eastAsianValue
            && (eastAsianValue = PropertyParsing::ParseStylePropertyLonghand(
                  tokens, PropertyId::FontVariantEastAsian, state)))
        {
          continue;
        }

        if (state.Context.PropertySettings.cssFontVariantEmojiEnabled && !emojiValue
            && (emojiValue =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::FontVariantEmoji, state)))
        {
          continue;
        }

        // Saw some value that didn't match anything else.
        return false;
      } while (!tokens.IsAtEnd());

      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantLigatures,
                                            ligaturesParser.Finalize(), implicitLigatures);
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantCaps, krys::move(capsValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantAlternates,
                                            krys::move(alternatesValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantNumeric, numericParser.Finalize(),
                                            implicitNumeric);
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantEastAsian,
                                            krys::move(eastAsianValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantPosition,
                                            krys::move(positionValue));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FontVariantEmoji, krys::move(emojiValue));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFontSynthesisShorthand(TokenRange &tokens, PropertyParserState &state,
                                                             const PropertyShorthand &shorthand,
                                                             PropertyParserResult &result) noexcept
    {
      // none | [ weight || style || small-caps ]
      if (tokens.Peek().ValueId() == ValueId::None)
      {
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontSynthesisSmallCaps,
                                              ConsumeIdent(tokens));
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontSynthesisStyle,
                                              PrimitiveValue::Create(ValueId::None));
        result.AddPropertyForCurrentShorthand(state, PropertyId::FontSynthesisWeight,
                                              PrimitiveValue::Create(ValueId::None));
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
        RefPtr ident = ConsumeIdent<ValueId::Weight, ValueId::Style, ValueId::SmallCaps>(tokens);
        if (!ident)
        {
          return false;
        }

        switch (ident->ValueId())
        {
          case ValueId::Weight:
          {
            if (!CheckAndMarkExistence(&foundWeight))
            {
              return false;
            }

            break;
          }
          case ValueId::Style:
          {
            if (!CheckAndMarkExistence(&foundStyle))
            {
              return false;
            }

            break;
          }
          case ValueId::SmallCaps:
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
        state, PropertyId::FontSynthesisWeight,
        PrimitiveValue::Create(foundWeight ? ValueId::Auto : ValueId::None));
      result.AddPropertyForCurrentShorthand(
        state, PropertyId::FontSynthesisStyle,
        PrimitiveValue::Create(foundStyle ? ValueId::Auto : ValueId::None));
      result.AddPropertyForCurrentShorthand(
        state, PropertyId::FontSynthesisSmallCaps,
        PrimitiveValue::Create(foundSmallCaps ? ValueId::Auto : ValueId::None));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTextDecorationSkipShorthand(TokenRange &tokens,
                                                                  PropertyParserState &state,
                                                                  const PropertyShorthand &shorthand,
                                                                  PropertyParserResult &result) noexcept
    {
      if (auto skip = ConsumeIdentRaw<ValueId::None, ValueId::Auto, ValueId::Ink>(tokens))
      {
        switch (*skip)
        {
          case ValueId::None:
          {
            result.AddPropertyForCurrentShorthand(state, PropertyId::TextDecorationSkipInk,
                                                  PrimitiveValue::Create(ValueId::None));
            return tokens.IsAtEnd();
          }
          case ValueId::Auto:
          case ValueId::Ink:
          {
            result.AddPropertyForCurrentShorthand(state, PropertyId::TextDecorationSkipInk,
                                                  PrimitiveValue::Create(ValueId::Auto));
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

    KRYS_NODISCARD static bool ConsumeMaskBorderShorthand(TokenRange &tokens, PropertyParserState &state,
                                                          const PropertyShorthand &shorthand,
                                                          PropertyParserResult &result) noexcept
    {
      auto components = ConsumeBorderImageComponents(tokens, state);
      if (!components)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::MaskBorderSource,
                                            krys::move(components->Source));
      result.AddPropertyForCurrentShorthand(state, PropertyId::MaskBorderSlice,
                                            krys::move(components->Slice));
      result.AddPropertyForCurrentShorthand(state, PropertyId::MaskBorderWidth,
                                            krys::move(components->Width));
      result.AddPropertyForCurrentShorthand(state, PropertyId::MaskBorderOutset,
                                            krys::move(components->Outset));
      result.AddPropertyForCurrentShorthand(state, PropertyId::MaskBorderRepeat,
                                            krys::move(components->Repeat));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeFlexShorthand(TokenRange &tokens, PropertyParserState &state,
                                                    const PropertyShorthand &shorthand,
                                                    PropertyParserResult &result) noexcept
    {
      // <'flex'>        = none | [ <'flex-grow'> <'flex-shrink'>? || <'flex-basis'> ]
      // <'flex-grow'>   = <number [0,∞]>
      //     NOTE: When omitted from shorthand, it is set to 1.
      // <'flex-shrink'> = <number [0,∞]>
      //     NOTE: When omitted from shorthand, it is set to 1.
      // <'flex-basis'>  = content | <'width'>
      //    NOTE: When omitted from shorthand, it is set to 0.
      // https://drafts.csswg.org/css-flexbox/#propdef-flex

      auto IsFlexBasisIdent = [](ValueId id)
      {
        switch (id)
        {
          case ValueId::Auto:
          case ValueId::Content:
          case ValueId::Intrinsic:
          case ValueId::MinIntrinsic:
          case ValueId::MinContent:
          case ValueId::MaxContent:
          case ValueId::FitContent:
          {
            return true;
          }
          default:
          {
            return false;
          }
        }
      };

      RefPtr<PrimitiveValue> flexGrow;
      RefPtr<PrimitiveValue> flexShrink;
      RefPtr<PrimitiveValue> flexBasis;

      if (tokens.Peek().ValueId() == ValueId::None)
      {
        flexGrow = PrimitiveValue::Create(0);
        flexShrink = PrimitiveValue::Create(0);
        flexBasis = PrimitiveValue::Create(ValueId::Auto);

        tokens.Discard();
        tokens.DiscardWhitespace();
      }
      else
      {
        size_t index = 0uz;
        while (!tokens.IsAtEnd() && index++ < 3uz)
        {
          if (auto number = PrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state))
          {
            if (!flexGrow)
            {
              flexGrow = krys::move(number);
            }
            else if (!flexShrink)
            {
              flexShrink = krys::move(number);
            }
            else if (number->IsZero() == true) // flex only allows a basis of 0 (sans units) if flex-grow and
                                               // flex-shrink values have already been set.
            {
              flexBasis = PrimitiveValue::Create(0, UnitType::px);
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
                PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
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
          flexGrow = PrimitiveValue::Create(1);
        }

        if (!flexShrink)
        {
          flexShrink = PrimitiveValue::Create(1);
        }

        // FIXME: Using % here is a hack to work around intrinsic sizing implementation being
        // a mess (e.g., turned off for nested column flexboxes, failing to relayout properly even
        // if turned back on for nested columns, etc.). We have layout test coverage of both
        // scenarios.
        if (!flexBasis)
        {
          flexBasis = PrimitiveValue::Create(0, UnitType::Percentage);
        }
      }

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::FlexGrow, krys::move(flexGrow));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FlexShrink, krys::move(flexShrink));
      result.AddPropertyForCurrentShorthand(state, PropertyId::FlexBasis, krys::move(flexBasis));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakAfterShorthand(TokenRange &tokens, PropertyParserState &state,
                                                              const PropertyShorthand &shorthand,
                                                              PropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromPageBreakBetween(*keyword);
      if (value == ValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BreakAfter, PrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakBeforeShorthand(TokenRange &tokens, PropertyParserState &state,
                                                               const PropertyShorthand &shorthand,
                                                               PropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromPageBreakBetween(*keyword);
      if (value == ValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BreakBefore, PrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePageBreakInsideShorthand(TokenRange &tokens, PropertyParserState &state,
                                                               const PropertyShorthand &shorthand,
                                                               PropertyParserResult &result) noexcept
    {
      auto keyword = ConsumeIdentRaw(tokens);
      if (!keyword || !tokens.IsAtEnd())
      {
        return false;
      }

      auto value = MapFromColumnRegionOrPageBreakInside(*keyword);
      if (value == ValueId::Invalid)
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::BreakInside, PrimitiveValue::Create(value));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTransformOriginShorthand(TokenRange &tokens, PropertyParserState &state,
                                                               const PropertyShorthand &shorthand,
                                                               PropertyParserResult &result) noexcept
    {
      if (auto position = ConsumeOneOrTwoComponentPositionUnresolved(tokens, state))
      {
        tokens.DiscardWhitespace();

        bool atEnd = tokens.IsAtEnd();
        auto resultZ = PrimitiveValueResolver<Length<>>::ConsumeAndResolve(tokens, state);
        if ((!resultZ && !atEnd) || !tokens.IsAtEnd())
        {
          return false;
        }

        auto [positionX, positionY] = Split(krys::move(*position));
        result.AddPropertyForCurrentShorthand(state, PropertyId::TransformOriginX,
                                              PositionXValue::Create(krys::move(positionX)));
        result.AddPropertyForCurrentShorthand(state, PropertyId::TransformOriginY,
                                              PositionYValue::Create(krys::move(positionY)));
        result.AddPropertyForCurrentShorthand(state, PropertyId::TransformOriginZ, resultZ);

        return true;
      }

      return false;
    }

    KRYS_NODISCARD static bool ConsumePerspectiveOriginShorthand(TokenRange &tokens,
                                                                 PropertyParserState &state,
                                                                 const PropertyShorthand &shorthand,
                                                                 PropertyParserResult &result) noexcept
    {
      if (auto position = ConsumePositionUnresolved(tokens, state))
      {
        auto [positionX, positionY] = Split(krys::move(*position));
        result.AddPropertyForCurrentShorthand(state, PropertyId::PerspectiveOriginX,
                                              PositionXValue::Create(krys::move(positionX)));
        result.AddPropertyForCurrentShorthand(state, PropertyId::PerspectiveOriginY,
                                              PositionYValue::Create(krys::move(positionY)));
        return true;
      }

      return false;
    }

    KRYS_NODISCARD static bool ConsumeOffsetShorthand(TokenRange &tokens, PropertyParserState &state,
                                                      const PropertyShorthand &shorthand,
                                                      PropertyParserResult &result) noexcept
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
        PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetPosition, state);

      // Parse out offset-path.
      auto offsetPath = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetPath, state);

      // Either one of offset-position and offset-path must be present.
      if (!offsetPosition && !offsetPath)
        return false;

      // Only parse offset-distance and offset-rotate if offset-path is specified.
      RefPtr<Value> offsetDistance;
      RefPtr<Value> offsetRotate;
      if (offsetPath)
      {
        // Try to parse offset-distance first. If successful, parse the following offset-rotate.
        // Otherwise, parse in the reverse order.
        if ((offsetDistance =
               PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetDistance, state)))
          offsetRotate = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetRotate, state);
        else
        {
          offsetRotate = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetRotate, state);
          offsetDistance =
            PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetDistance, state);
        }
      }

      // Parse out offset-anchor. Only parse if the prefix slash is present.
      RefPtr<Value> offsetAnchor;
      if (ConsumeSlash(tokens))
      {
        // offset-anchor must follow the slash.
        if (!(offsetAnchor =
                PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::OffsetAnchor, state)))
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::OffsetPath, krys::move(offsetPath));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OffsetDistance, krys::move(offsetDistance));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OffsetPosition, krys::move(offsetPosition));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OffsetAnchor, krys::move(offsetAnchor));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OffsetRotate, krys::move(offsetRotate));

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeListStyleShorthand(TokenRange &tokens, PropertyParserState &state,
                                                         const PropertyShorthand &shorthand,
                                                         PropertyParserResult &result) noexcept
    {
      RefPtr<Value> position;
      RefPtr<Value> image;
      RefPtr<Value> type;
      unsigned noneCount = 0;

      while (!tokens.IsAtEnd())
      {
        if (tokens.Peek().ValueId() == ValueId::None)
        {
          ++noneCount;
          DiscardIdent(tokens);
          continue;
        }

        if (!position
            && (position =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ListStylePosition, state)))
        {
          continue;
        }

        if (!image
            && (image =
                  PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ListStyleImage, state)))
        {
          continue;
        }

        if (!type
            && (type = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ListStyleType, state)))
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
        type = PrimitiveValue::Create(ValueId::None);
      }
      else if (noneCount == 1uz)
      {
        // Use implicit none for list-style-image, but non-implicit for type.
        if (!type)
        {
          type = PrimitiveValue::Create(ValueId::None);
        }
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::ListStylePosition, krys::move(position));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ListStyleImage, krys::move(image));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ListStyleType, krys::move(type));

      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeOverscrollBehaviorShorthand(TokenRange &tokens,
                                                                  PropertyParserState &state,
                                                                  const PropertyShorthand &shorthand,
                                                                  PropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);

      if (tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr overscrollBehaviorX = PropertyParsing::ConsumeOverscrollBehaviorX(tokens);
      if (!overscrollBehaviorX)
      {
        return false;
      }

      RefPtr<Value> overscrollBehaviorY;
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        overscrollBehaviorY = overscrollBehaviorX;
      }
      else
      {
        overscrollBehaviorY = PropertyParsing::ConsumeOverscrollBehaviorY(tokens);
        tokens.DiscardWhitespace();
        if (!tokens.IsAtEnd())
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::OverscrollBehaviorX,
                                            krys::move(overscrollBehaviorX));
      result.AddPropertyForCurrentShorthand(state, PropertyId::OverscrollBehaviorY,
                                            krys::move(overscrollBehaviorY));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainerShorthand(TokenRange &tokens, PropertyParserState &state,
                                                         const PropertyShorthand &shorthand,
                                                         PropertyParserResult &result) noexcept
    {
      RefPtr name = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ContainerName, state);
      if (!name)
      {
        return false;
      }

      bool sawSlash = false;
      auto ConsumeSlashType = [&]() -> RefPtr<Value>
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
        return PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::ContainerType, state);
      };

      auto type = ConsumeSlashType();
      if (!tokens.IsAtEnd() || (sawSlash && !type))
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::ContainerName, krys::move(name));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ContainerType, krys::move(type));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeContainIntrinsicSizeShorthand(TokenRange &tokens,
                                                                    PropertyParserState &state,
                                                                    const PropertyShorthand &shorthand,
                                                                    PropertyParserResult &result) noexcept
    {
      assert(shorthand.Size() == 2);
      assert(IsExposed(PropertyId::ContainIntrinsicSize, &state.Context.PropertySettings));

      if (tokens.IsAtEnd())
      {
        return false;
      }

      RefPtr containIntrinsicWidth = PropertyParsing::ConsumeContainIntrinsicWidth(tokens, state);
      if (!containIntrinsicWidth)
      {
        return false;
      }

      RefPtr<Value> containIntrinsicHeight;
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        containIntrinsicHeight = containIntrinsicWidth;
      }
      else
      {
        containIntrinsicHeight = PropertyParsing::ConsumeContainIntrinsicHeight(tokens, state);
        tokens.DiscardWhitespace();
        if (!tokens.IsAtEnd() || !containIntrinsicHeight)
        {
          return false;
        }
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::ContainIntrinsicWidth,
                                            krys::move(containIntrinsicWidth));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ContainIntrinsicHeight,
                                            krys::move(containIntrinsicHeight));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeAnimationRangeShorthand(TokenRange &tokens, PropertyParserState &state,
                                                              const PropertyShorthand &shorthand,
                                                              PropertyParserResult &result) noexcept
    {
      ValueListBuilder startList;
      ValueListBuilder endList;

      do
      {
        RefPtr<Value> start = ConsumeSingleAnimationRangeStart(tokens, state);
        if (!start)
        {
          return false;
        }
        tokens.DiscardWhitespace();

        RefPtr<Value> end;
        if (tokens.IsAtEnd() || tokens.Peek().Type() == TokenType::Comma)
        {
          // From the spec: If <'animation-range-end'> is omitted and <'animation-range-start'> includes a
          // component, then animation-range-end is set to that same and 100%. Otherwise, any omitted longhand
          // is set to its initial value.
          auto RangeEndValueForStartValue = [](const Value &value)
          {
            auto IsRangeOffset = [](auto &value)
            {
              return value.IsLength() || value.IsPercentage() || value.IsCalculatedPercentageWithLength();
            };

            if (auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value);
                primitiveValue && IsRangeOffset(*primitiveValue))
            {
              return PrimitiveValue::Create(ValueId::Normal);
            }

            return PrimitiveValue::Create(value.ValueId());
          };

          if (auto *startPrimitiveValue = DynamicDowncast<PrimitiveValue>(start.get()))
          {
            end = RangeEndValueForStartValue(*startPrimitiveValue);
          }
          else
          {
            auto *startPair = Downcast<ValuePair>(start.get());
            end = RangeEndValueForStartValue(startPair->First());
          }
        }
        else
        {
          end = ConsumeSingleAnimationRangeEnd(tokens, state);
          tokens.DiscardWhitespace();
          if (!end)
          {
            return false;
          }
        }

        startList.push_back(krys::move(start));
        endList.push_back(krys::move(end));
      } while (ConsumeComma(tokens));

      if (!tokens.IsAtEnd())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::AnimationRangeStart,
                                            ValueList::CreateCommaSeparated(krys::move(startList)));
      result.AddPropertyForCurrentShorthand(state, PropertyId::AnimationRangeEnd,
                                            ValueList::CreateCommaSeparated(krys::move(endList)));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeScrollTimelineShorthand(TokenRange &tokens, PropertyParserState &state,
                                                              const PropertyShorthand &shorthand,
                                                              PropertyParserResult &result) noexcept
    {
      ValueListBuilder namesList;
      ValueListBuilder axesList;

      do
      {
        // A valid scroll-timeline-name is required.
        if (auto name = PropertyParsing::ConsumeSingleScrollTimelineName(tokens))
        {
          namesList.push_back(krys::move(name));
        }
        else
        {
          return false;
        }

        // A scroll-timeline-axis is optional.
        if (tokens.Peek().Type() == TokenType::Comma || tokens.IsAtEnd())
        {
          axesList.push_back(PrimitiveValue::Create(ValueId::Block));
        }
        else if (auto axis = PropertyParsing::ConsumeAxis(tokens))
        {
          axesList.push_back(krys::move(axis));
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

      result.AddPropertyForCurrentShorthand(state, PropertyId::ScrollTimelineName,
                                            ValueList::CreateCommaSeparated(krys::move(namesList)));
      if (!axesList.empty())
      {
        result.AddPropertyForCurrentShorthand(state, PropertyId::ScrollTimelineAxis,
                                              ValueList::CreateCommaSeparated(krys::move(axesList)));
      }

      return true;
    }

    KRYS_NODISCARD static bool ConsumeViewTimelineShorthand(TokenRange &tokens, PropertyParserState &state,
                                                            const PropertyShorthand &shorthand,
                                                            PropertyParserResult &result) noexcept
    {
      ValueListBuilder namesList;
      ValueListBuilder axesList;
      ValueListBuilder insetsList;

      auto DefaultAxis = [] -> Ref<Value>
      {
        return PrimitiveValue::Create(ValueId::Block);
      };

      auto DefaultInsets = [] -> Ref<Value>
      {
        return PrimitiveValue::Create(ValueId::Auto);
      };

      do
      {
        // A valid view-timeline-name is required.
        if (auto name = PropertyParsing::ConsumeSingleScrollTimelineName(tokens))
        {
          namesList.push_back(krys::move(name));
        }
        else
        {
          return false;
        }

        // Both a view-timeline-axis and a view-timeline-inset are optional.
        if (tokens.Peek().Type() != TokenType::Comma && !tokens.IsAtEnd())
        {
          RefPtr axis = PropertyParsing::ConsumeAxis(tokens);
          RefPtr insets = ConsumeSingleViewTimelineInsetItem(tokens, state);

          // Since the order of view-timeline-axis and view-timeline-inset is not guaranteed, let's try
          // view-timeline-axis again.
          if (!axis)
          {
            axis = PropertyParsing::ConsumeAxis(tokens);
          }

          if (!axis && !insets)
          {
            return false;
          }

          if (axis)
          {
            axesList.push_back(krys::move(axis));
          }
          else
          {
            axesList.push_back(DefaultAxis());
          }

          if (insets)
          {
            axesList.push_back(krys::move(insets));
          }
          else
          {
            axesList.push_back(DefaultInsets());
          }
        }
        else
        {
          axesList.push_back(DefaultAxis());
          insetsList.push_back(DefaultInsets());
        }
      } while (ConsumeComma(tokens));

      if (namesList.empty())
      {
        return false;
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::ViewTimelineName,
                                            ValueList::CreateCommaSeparated(krys::move(namesList)));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ViewTimelineAxis,
                                            ValueList::CreateCommaSeparated(krys::move(axesList)));
      result.AddPropertyForCurrentShorthand(state, PropertyId::ViewTimelineInset,
                                            ValueList::CreateCommaSeparated(krys::move(insetsList)));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeLineClampShorthand(TokenRange &tokens, PropertyParserState &state,
                                                         const PropertyShorthand &shorthand,
                                                         PropertyParserResult &result) noexcept
    {
      assert(state.Context.PropertySettings.cssLineClampEnabled);

      if (tokens.Peek().ValueId() == ValueId::None)
      {
        // Sets max-lines to none, continue to auto, and block-ellipsis to none.
        result.AddPropertyForCurrentShorthand(state, PropertyId::MaxLines,
                                              PrimitiveValue::Create(ValueId::None));
        result.AddPropertyForCurrentShorthand(state, PropertyId::Continue,
                                              PrimitiveValue::Create(ValueId::Auto));
        result.AddPropertyForCurrentShorthand(state, PropertyId::BlockEllipsis,
                                              PrimitiveValue::Create(ValueId::None));

        DiscardIdent(tokens);

        return tokens.IsAtEnd();
      }

      RefPtr<Value> maxLines;
      RefPtr<Value> blockEllipsis;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!maxLines && (maxLines = PropertyParsing::ConsumeMaxLines(tokens, state)))
        {
          continue;
        }

        if (!blockEllipsis && (blockEllipsis = PropertyParsing::ConsumeBlockEllipsis(tokens)))
        {
          continue;
        }

        // There has to be at least one valid longhand.
        return false;
      }

      if (!blockEllipsis)
      {
        blockEllipsis = PrimitiveValue::Create(ValueId::Auto);
      }

      if (!maxLines)
      {
        maxLines = PrimitiveValue::Create(ValueId::None);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::MaxLines, krys::move(maxLines));
      result.AddPropertyForCurrentShorthand(state, PropertyId::Continue,
                                            PrimitiveValue::Create(ValueId::Discard));
      result.AddPropertyForCurrentShorthand(state, PropertyId::BlockEllipsis, krys::move(blockEllipsis));
      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeTextBoxShorthand(TokenRange &tokens, PropertyParserState &state,
                                                       const PropertyShorthand &shorthand,
                                                       PropertyParserResult &result) noexcept
    {
      if (tokens.Peek().ValueId() == ValueId::Normal)
      {
        // if the single keyword normal is specified, it sets text-box-trim to none and text-box-edge to auto.
        result.AddPropertyForCurrentShorthand(state, PropertyId::TextBoxTrim,
                                              PrimitiveValue::Create(ValueId::None));
        result.AddPropertyForCurrentShorthand(state, PropertyId::TextBoxEdge,
                                              PrimitiveValue::Create(ValueId::Auto));

        DiscardIdent(tokens);
        return tokens.IsAtEnd();
      }

      RefPtr<Value> textBoxTrim;
      RefPtr<Value> textBoxEdge;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!textBoxTrim && (textBoxTrim = PropertyParsing::ConsumeTextBoxTrim(tokens)))
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
        textBoxEdge = PrimitiveValue::Create(ValueId::Auto);
      }

      // Omitting the text-box-trim value sets it to both (not the initial value)
      if (!textBoxTrim)
      {
        textBoxTrim = PrimitiveValue::Create(ValueId::TrimBoth);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::TextBoxTrim, krys::move(textBoxTrim));
      result.AddPropertyForCurrentShorthand(state, PropertyId::TextBoxEdge, krys::move(textBoxEdge));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeTextWrapShorthand(TokenRange &tokens, PropertyParserState &state,
                                                        const PropertyShorthand &shorthand,
                                                        PropertyParserResult &result) noexcept
    {
      RefPtr<Value> mode;
      RefPtr<Value> style;

      for (size_t propertiesParsed = 0uz; propertiesParsed < 2uz && !tokens.IsAtEnd(); ++propertiesParsed)
      {
        if (!mode && (mode = PropertyParsing::ConsumeTextWrapMode(tokens)))
        {
          continue;
        }

        if (!style && (style = PropertyParsing::ConsumeTextWrapStyle(tokens, state)))
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
        mode = PrimitiveValue::Create(ValueId::Wrap);
      }

      if (!style)
      {
        style = PrimitiveValue::Create(ValueId::Auto);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::TextWrapMode, krys::move(mode));
      result.AddPropertyForCurrentShorthand(state, PropertyId::TextWrapStyle, krys::move(style));

      return true;
    }

    KRYS_NODISCARD static bool ConsumeWhiteSpaceShorthand(TokenRange &tokens, PropertyParserState &state,
                                                          const PropertyShorthand &shorthand,
                                                          PropertyParserResult &result) noexcept
    {
      RefPtr<Value> whiteSpaceCollapse;
      RefPtr<Value> textWrapMode;

      // Single value syntax.
      auto singleValueKeyword =
        ConsumeIdentRaw<ValueId::Normal, ValueId::Pre, ValueId::PreLine, ValueId::PreWrap>(tokens);

      if (singleValueKeyword)
      {
        switch (*singleValueKeyword)
        {
          case ValueId::Normal:
          {
            whiteSpaceCollapse = PrimitiveValue::Create(ValueId::Collapse);
            textWrapMode = PrimitiveValue::Create(ValueId::Wrap);
            break;
          }
          case ValueId::Pre:
          {
            whiteSpaceCollapse = PrimitiveValue::Create(ValueId::Preserve);
            textWrapMode = PrimitiveValue::Create(ValueId::Nowrap);
            break;
          }
          case ValueId::PreLine:
          {
            whiteSpaceCollapse = PrimitiveValue::Create(ValueId::PreserveBreaks);
            textWrapMode = PrimitiveValue::Create(ValueId::Wrap);
            break;
          }
          case ValueId::PreWrap:
          {
            whiteSpaceCollapse = PrimitiveValue::Create(ValueId::Preserve);
            textWrapMode = PrimitiveValue::Create(ValueId::Wrap);
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
              && (whiteSpaceCollapse = PropertyParsing::ConsumeWhiteSpaceCollapse(tokens)))
          {
            continue;
          }

          if (!textWrapMode && (textWrapMode = PropertyParsing::ConsumeTextWrapMode(tokens)))
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
        whiteSpaceCollapse = PrimitiveValue::Create(ValueId::Collapse);
      }

      if (!textWrapMode)
      {
        textWrapMode = PrimitiveValue::Create(ValueId::Wrap);
      }

      result.AddPropertyForCurrentShorthand(state, PropertyId::WhiteSpaceCollapse,
                                            krys::move(whiteSpaceCollapse));
      result.AddPropertyForCurrentShorthand(state, PropertyId::TextWrapMode, krys::move(textWrapMode));

      return true;
    }

    KRYS_NODISCARD static bool ConsumePositionTryShorthand(TokenRange &tokens, PropertyParserState &state,
                                                           const PropertyShorthand &shorthand,
                                                           PropertyParserResult &result) noexcept
    {
      auto order = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::PositionTryOrder, state);
      auto fallbacks =
        PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::PositionTryFallbacks, state);
      if (!fallbacks)
        return false;

      result.AddPropertyForCurrentShorthand(state, PropertyId::PositionTryOrder, krys::move(order));
      result.AddPropertyForCurrentShorthand(state, PropertyId::PositionTryFallbacks, krys::move(fallbacks));
      return tokens.IsAtEnd();
    }

    KRYS_NODISCARD static bool ConsumeMarkerShorthand(TokenRange &tokens, PropertyParserState &state,
                                                      const PropertyShorthand &shorthand,
                                                      PropertyParserResult &result) noexcept
    {
      auto marker = PropertyParsing::ParseStylePropertyLonghand(tokens, PropertyId::MarkerStart, state);
      if (!marker || !tokens.IsAtEnd())
      {
        return false;
      }

      Ref<Value> markerRef = krys::move(marker);

      result.AddPropertyForCurrentShorthand(state, PropertyId::MarkerStart, markerRef);
      result.AddPropertyForCurrentShorthand(state, PropertyId::MarkerMid, markerRef);
      result.AddPropertyForCurrentShorthand(state, PropertyId::MarkerEnd, krys::move(markerRef));

      return true;
    }
  };
}