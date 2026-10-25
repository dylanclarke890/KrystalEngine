#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimation.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/CSS/Parser/KeywordLookup.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Animations.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Easing.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSNumericFactory.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/Graphics/GeometryUtilities.hpp"
#include "Krystal.Booey/Graphics/GraphicsLayerAnimation.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperationsSharedPrimitivesPrefix.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Extractor.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/PendingResources.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include "Krystal.Booey/Style/PseudoElementRequest.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Style/TransformResolver.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRangeName.hpp"
#include "Krystal.Booey/Style/Values/Easing/EasingFunction.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Transform.hpp"
#include "Krystal.Booey/Style/Values/WillChange/WillChange.hpp"
#include <algorithm>
#include <charconv>
#include <cmath>
#include <limits>

namespace krys::boo
{
  namespace
  {
    void InvalidateElement(const Maybe<style::Styleable> &styleable) noexcept
    {
      if (!styleable)
      {
        return;
      }

      Ref element = styleable->ProtectedElement();
      if (!element->NodeDocument().InStyleRecalc())
      {
        element->InvalidateStyleForAnimation();
      }
    }

    KRYS_NODISCARD bool IsLinearTimingFunctionWithPoints(const TimingFunction *timingFunction) noexcept
    {
      auto *linearTimingFunction = DynamicDowncast<LinearTimingFunction>(timingFunction);
      return linearTimingFunction && !linearTimingFunction->Points().empty();
    }

    KRYS_NODISCARD css::PropertyId
      IDLAttributeNameToAnimationPropertyName(const dom::DOMString &idlAttributeName) noexcept
    {
      // https://drafts.csswg.org/web-animations-1/#idl-attribute-name-to-animation-property-name
      // 1. If attribute conforms to the <custom-property-name> production, return attribute.

      // 2. If attribute is the string "cssFloat", then return an animation property representing the CSS
      // float property.
      if (idlAttributeName == u8"cssFloat")
      {
        return css::PropertyId::Float;
      }

      // 3. If attribute is the string "cssOffset", then return an animation property representing the CSS
      // offset property.
      if (idlAttributeName == u8"cssOffset")
      {
        return css::PropertyId::Offset;
      }

      // If the attribute is the string "fontStretch" return the CSS font-width property that it aliases.
      if (idlAttributeName == u8"fontStretch")
      {
        return css::PropertyId::FontWidth;
      }

      // 4. Otherwise, return the result of applying the IDL attribute to CSS property algorithm [CSSOM] to
      // attribute.
      auto cssPropertyId = css::CSSStyleProperties::GetPropertyIdFromJavaScriptPropertyName(
        css::CSSOMStringAtom(idlAttributeName));

      if (cssPropertyId == css::PropertyId::Invalid && css::IsCustomPropertyName(idlAttributeName))
      {
        return css::PropertyId::Custom;
      }

      // We need to check that converting the property back to IDL form yields the same result such that a
      // property passed in non-IDL form is rejected, for instance "font-size".
      if (idlAttributeName != KeyframeEffect::PropertyIdToIDLAttributeName(cssPropertyId))
      {
        return css::PropertyId::Invalid;
      }

      return cssPropertyId;
    }

    KRYS_NODISCARD bool IsTimelineRangeOffsetValid(const TimelineRangeOffset &timelineRangeOffset) noexcept
    {
      if (style::ConvertRangeStringToSingleTimelineRangeName(timelineRangeOffset.RangeName)
          == style::SingleAnimationRangeName::Normal)
      {
        return false;
      }
      auto *offsetUnitValue = DynamicDowncast<css::CSSUnitValue>(timelineRangeOffset.Offset.get());
      return offsetUnitValue && offsetUnitValue->UnitEnum() == css::UnitType::Percentage;
    }

    /// @brief The whole of @p string as a number, as script converts a string to a double.
    KRYS_NODISCARD Maybe<double> StringToDouble(const dom::DOMString &string) noexcept
    {
      const auto *begin = reinterpret_cast<const char *>(string.data());
      const auto *end = begin + string.size();
      double value = 0;
      auto [pointer, error] = std::from_chars(begin, end, value);
      if (error != std::errc {} || pointer != end || begin == end)
      {
        return null;
      }
      return value;
    }

    KRYS_NODISCARD Maybe<Variant<double, TimelineRangeOffset>>
      DoubleOrTimelineRangeOffsetFromString(const dom::DOMString &offsetString,
                                            const dom::Document &document) noexcept
    {
      if (auto doubleValue = StringToDouble(offsetString))
      {
        return Variant<double, TimelineRangeOffset> {*doubleValue};
      }

      css::ParserContext parserContext(document);
      auto offsets = css::PropertyParserHelpers::ParseKeyframeKeyList(offsetString, parserContext);
      if (offsets.size() != 1)
      {
        return null;
      }

      auto [rangeValueId, value] = offsets[0];
      auto rangeName = style::ConvertValueIdToSingleAnimationRangeName(rangeValueId);
      if (rangeName == style::SingleAnimationRangeName::Normal)
      {
        return Variant<double, TimelineRangeOffset> {value};
      }

      return Variant<double, TimelineRangeOffset> {
        TimelineRangeOffset {style::ConvertSingleAnimationRangeNameToRangeString(rangeName),
                             css::CSSNumericFactory::Percent(value * 100)}};
    }

    KRYS_NODISCARD Maybe<KeyframeEffect::KeyframeOffset>
      ValidateKeyframeOffset(const KeyframeEffect::KeyframeOffset &offset,
                             const dom::Document &document) noexcept
    {
      if (const auto *doubleValue = std::get_if<double>(&offset))
      {
        return KeyframeEffect::KeyframeOffset {*doubleValue};
      }

      if (const auto *timelineRangeOffset = std::get_if<TimelineRangeOffset>(&offset))
      {
        if (!IsTimelineRangeOffsetValid(*timelineRangeOffset))
        {
          return null;
        }
        return KeyframeEffect::KeyframeOffset {*timelineRangeOffset};
      }

      if (const auto *stringOffset = std::get_if<dom::DOMString>(&offset))
      {
        auto parsedValue = DoubleOrTimelineRangeOffsetFromString(*stringOffset, document);
        if (!parsedValue)
        {
          return null;
        }
        if (const auto *doubleOffset = std::get_if<double>(&*parsedValue))
        {
          return KeyframeEffect::KeyframeOffset {*doubleOffset};
        }
        return KeyframeEffect::KeyframeOffset {std::get<TimelineRangeOffset>(*parsedValue)};
      }

      krys_debug_assert(std::holds_alternative<std::nullptr_t>(offset));
      return KeyframeEffect::KeyframeOffset {nullptr};
    }

    KRYS_NODISCARD double ComputedOffset(style::SingleAnimationRangeName rangeName, double offset) noexcept
    {
      if (rangeName == style::SingleAnimationRangeName::Normal
          || rangeName == style::SingleAnimationRangeName::Omitted)
      {
        return offset;
      }

      // FIXME: The offset within the active view timeline's named range, and within the animation's
      // attachment range, once there are view timelines; until then, as without a view timeline.
      return std::numeric_limits<double>::quiet_NaN();
    }

    void ComputeMissingKeyframeOffsets(List<KeyframeEffect::ParsedKeyframe> &keyframes) noexcept
    {
      // https://drafts.csswg.org/web-animations-1/#compute-missing-keyframe-offsets

      if (keyframes.empty())
      {
        return;
      }

      List<KeyframeEffect::ParsedKeyframe *> keyframesWithDoubleOrNullOffset;

      // 1. For each keyframe, in keyframes, let the computed keyframe offset of the keyframe be equal to its
      // keyframe offset value. In our implementation, we only set non-null values to avoid making
      // computedOffset Maybe<double>. Instead, we'll know that a keyframe hasn't had a computed offset by
      // checking if it has a null offset and a 0 computedOffset, since the first keyframe will already have a
      // 0 computedOffset.
      for (auto &keyframe : keyframes)
      {
        auto &offset = keyframe.Offset;
        if (const auto *timelineRangeOffset = std::get_if<TimelineRangeOffset>(&offset))
        {
          auto rangeName = style::ConvertRangeStringToSingleTimelineRangeName(timelineRangeOffset->RangeName);
          auto *offsetUnitValue = DynamicDowncast<css::CSSUnitValue>(timelineRangeOffset->Offset.get());
          krys_debug_assert(offsetUnitValue && offsetUnitValue->UnitEnum() == css::UnitType::Percentage);
          keyframe.ComputedOffset = ComputedOffset(rangeName, offsetUnitValue->Value() / 100);
        }
        else
        {
          keyframesWithDoubleOrNullOffset.push_back(&keyframe);
          if (const auto *doubleValue = std::get_if<double>(&offset))
          {
            keyframe.ComputedOffset = *doubleValue;
          }
          else
          {
            keyframe.ComputedOffset = std::numeric_limits<double>::quiet_NaN();
          }
        }
      }

      if (keyframesWithDoubleOrNullOffset.empty())
      {
        return;
      }

      // 2. If keyframes contains more than one keyframe and the computed keyframe offset of the first
      // keyframe in keyframes is null, set the computed keyframe offset of the first keyframe to 0.
      if (keyframesWithDoubleOrNullOffset.size() > 1
          && std::isnan(keyframesWithDoubleOrNullOffset[0]->ComputedOffset))
      {
        keyframesWithDoubleOrNullOffset[0]->ComputedOffset = 0;
      }

      // 3. If the computed keyframe offset of the last keyframe in keyframes is null, set its computed
      // keyframe offset to 1.
      if (std::isnan(keyframesWithDoubleOrNullOffset.back()->ComputedOffset))
      {
        keyframesWithDoubleOrNullOffset.back()->ComputedOffset = 1;
      }

      // 4. For each pair of keyframes A and B where:
      //    - A appears before B in keyframes, and
      //    - A and B have a computed keyframe offset that is not null, and
      //    - all keyframes between A and B have a null computed keyframe offset,
      //    calculate the computed keyframe offset of each keyframe between A and B as follows:
      //    1. Let offsetk be the computed keyframe offset of a keyframe k.
      //    2. Let n be the number of keyframes between and including A and B minus 1.
      //    3. Let index refer to the position of keyframe in the sequence of keyframes between A and B such
      //    that
      //       the first keyframe after A has an index of 1.
      //    4. Set the computed keyframe offset of keyframe to offsetA + (offsetB − offsetA) × index / n.

      size_t indexOfLastKeyframeWithNonNullOffset = 0;
      for (size_t i = 1; i < keyframesWithDoubleOrNullOffset.size(); ++i)
      {
        auto &keyframe = *keyframesWithDoubleOrNullOffset[i];
        // Keyframes with a null offset that don't yet have a non-zero computed offset are keyframes with an
        // offset that needs to be computed.
        if (std::isnan(keyframe.ComputedOffset))
        {
          continue;
        }
        if (indexOfLastKeyframeWithNonNullOffset != i - 1)
        {
          double lastNonNullOffset =
            keyframesWithDoubleOrNullOffset[indexOfLastKeyframeWithNonNullOffset]->ComputedOffset;
          double offsetDelta = keyframe.ComputedOffset - lastNonNullOffset;
          double offsetIncrement =
            offsetDelta / static_cast<double>(i - indexOfLastKeyframeWithNonNullOffset);
          size_t indexOfFirstKeyframeWithNullOffset = indexOfLastKeyframeWithNonNullOffset + 1;
          for (size_t j = indexOfFirstKeyframeWithNullOffset; j < i; ++j)
          {
            keyframesWithDoubleOrNullOffset[j]->ComputedOffset =
              lastNonNullOffset
              + static_cast<double>(j - indexOfLastKeyframeWithNonNullOffset) * offsetIncrement;
          }
        }
        indexOfLastKeyframeWithNonNullOffset = i;
      }
    }

    /// @brief Steps 2 to 6 of processing a keyframe-like object: the properties of @p properties that can be
    /// animated, sorted, with their values.
    /// @see https://drafts.csswg.org/web-animations-1/#process-a-keyframe-like-object
    KRYS_NODISCARD List<KeyframeEffect::PropertyAndValues> ProcessKeyframeLikeObjectProperties(
      dom::Document &document, const List<Pair<dom::DOMString, List<dom::DOMString>>> &properties) noexcept
    {
      // 2. Build up a list of animatable properties as follows:
      //
      //    1. Let animatable properties be a list of property names (including shorthand properties that have
      //       longhand sub-properties that are animatable) that can be animated by the implementation.
      //    2. Convert each property name in animatable properties to the equivalent IDL attribute by applying
      //       the animation property name to IDL attribute name algorithm.

      // 3. Let input properties be the result of calling the EnumerableOwnNames operation with keyframe input
      //    as the object.
      // A property given more than once keeps its last values, as an object's would.
      List<const Pair<dom::DOMString, List<dom::DOMString>> *> inputProperties;
      for (const auto &property : properties)
      {
        std::erase_if(inputProperties,
                      [&](const auto *inputProperty) { return inputProperty->first == property.first; });
        inputProperties.push_back(&property);
      }

      auto isDirectionAwareShorthand = [](css::PropertyId property)
      {
        for (auto longhand : css::ShorthandForProperty(property))
        {
          if (css::Property::IsDirectionAwareProperty(longhand))
          {
            return true;
          }
        }
        return false;
      };

      css::ParserContext parserContext(document);

      // 4. Make up a new list animation properties that consists of all of the properties that are in both
      //    input properties and animatable properties, or which are in input properties and conform to the
      //    <custom-property-name> production.
      using InputProperty = const Pair<dom::DOMString, List<dom::DOMString>> *;
      List<InputProperty> logicalShorthands;
      List<InputProperty> physicalShorthands;
      List<InputProperty> logicalLonghands;
      List<InputProperty> physicalLonghands;
      for (const auto *inputProperty : inputProperties)
      {
        auto cssProperty = IDLAttributeNameToAnimationPropertyName(inputProperty->first);
        if (cssProperty != css::PropertyId::Custom
            && !css::IsExposed(cssProperty, &parserContext.PropertySettings))
        {
          cssProperty = css::PropertyId::Invalid;
        }
        auto resolvedCSSProperty = css::Property::ResolveDirectionAwareProperty(cssProperty, WritingMode());
        AnimatableCSSProperty animatableProperty = resolvedCSSProperty;
        if (cssProperty == css::PropertyId::Custom)
        {
          animatableProperty = css::CSSOMStringAtom(inputProperty->first);
        }
        if (style::interpolation::CanInterpolate(animatableProperty))
        {
          if (cssProperty != css::PropertyId::Custom && isDirectionAwareShorthand(cssProperty))
          {
            logicalShorthands.push_back(inputProperty);
          }
          else if (cssProperty != css::PropertyId::Custom && css::IsShorthand(cssProperty))
          {
            physicalShorthands.push_back(inputProperty);
          }
          else if (resolvedCSSProperty != cssProperty)
          {
            logicalLonghands.push_back(inputProperty);
          }
          else
          {
            physicalLonghands.push_back(inputProperty);
          }
        }
      }

      // 5. Sort animation properties in ascending order by the Unicode codepoints that define each property
      //    name.
      auto sortPropertiesInAscendingOrder = [](List<InputProperty> &list)
      {
        std::ranges::sort(list, [](InputProperty a, InputProperty b) { return a->first < b->first; });
      };
      sortPropertiesInAscendingOrder(logicalShorthands);
      sortPropertiesInAscendingOrder(physicalShorthands);
      sortPropertiesInAscendingOrder(logicalLonghands);
      sortPropertiesInAscendingOrder(physicalLonghands);

      List<InputProperty> animationProperties;
      animationProperties.insert(animationProperties.end(), logicalShorthands.begin(),
                                 logicalShorthands.end());
      animationProperties.insert(animationProperties.end(), physicalShorthands.begin(),
                                 physicalShorthands.end());
      animationProperties.insert(animationProperties.end(), logicalLonghands.begin(), logicalLonghands.end());
      animationProperties.insert(animationProperties.end(), physicalLonghands.begin(),
                                 physicalLonghands.end());

      // 6. For each property name in animation properties,
      List<KeyframeEffect::PropertyAndValues> propertiesAndValues;
      for (const auto *animationProperty : animationProperties)
      {
        // 1. to 3. Let property values be the value given for the property name.

        // 4. Calculate the normalized property name as the result of applying the IDL attribute name to
        //    animation property name algorithm to property name.
        const auto &propertyName = animationProperty->first;
        auto cssPropertyId = IDLAttributeNameToAnimationPropertyName(propertyName);

        // 5. Add a property to to keyframe output with normalized property name as the property name, and
        //    property values as the property value.
        if (cssPropertyId == css::PropertyId::Custom)
        {
          propertiesAndValues.push_back(KeyframeEffect::PropertyAndValues {
            cssPropertyId, css::CSSOMStringAtom(propertyName), animationProperty->second});
        }
        else
        {
          propertiesAndValues.push_back(KeyframeEffect::PropertyAndValues {
            cssPropertyId, css::CSSOMStringAtom::Null(), animationProperty->second});
        }
      }

      // 7. Return keyframe output.
      return propertiesAndValues;
    }

    void SetKeyframeProperty(KeyframeEffect::ParsedKeyframe &keyframe,
                             const KeyframeEffect::PropertyAndValues &property, const dom::DOMString &value,
                             const css::ParserContext &parserContext) noexcept
    {
      if (property.Property == css::PropertyId::Custom)
      {
        if (keyframe.Style->SetCustomProperty(css::CSSOMString(property.CustomProperty.View()), value,
                                              parserContext))
        {
          keyframe.CustomStyleStrings.insert_or_assign(property.CustomProperty, value);
        }
      }
      else if (keyframe.Style->SetProperty(property.Property, value, parserContext))
      {
        keyframe.StyleStrings.insert_or_assign(property.Property, value);
      }
    }

    KRYS_NODISCARD dom::ExceptionOr<void>
      ProcessIterableKeyframes(dom::Document &document, List<KeyframeEffect::InputKeyframe> &&keyframesInput,
                               List<KeyframeEffect::ParsedKeyframe> &parsedKeyframes) noexcept
    {
      css::ParserContext parserContext(document);

      for (auto &inputKeyframe : keyframesInput)
      {
        // 7. Append to processed keyframes the result of running the procedure to process a keyframe-like
        // object passing nextItem as the keyframe input and with the allow lists flag set to false.
        KeyframeEffect::ParsedKeyframe keyframeOutput;

        if (const auto *doubleValue = std::get_if<double>(&inputKeyframe.Offset))
        {
          keyframeOutput.Offset = *doubleValue;
        }
        else if (const auto *timelineRangeOffset = std::get_if<TimelineRangeOffset>(&inputKeyframe.Offset))
        {
          if (!IsTimelineRangeOffsetValid(*timelineRangeOffset))
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          keyframeOutput.Offset = *timelineRangeOffset;
        }
        else if (const auto *stringOffset = std::get_if<dom::DOMString>(&inputKeyframe.Offset))
        {
          auto parsedValue = DoubleOrTimelineRangeOffsetFromString(*stringOffset, document);
          if (!parsedValue)
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          if (const auto *doubleOffset = std::get_if<double>(&*parsedValue))
          {
            keyframeOutput.Offset = *doubleOffset;
          }
          else
          {
            keyframeOutput.Offset = std::get<TimelineRangeOffset>(*parsedValue);
          }
        }

        keyframeOutput.Easing = krys::move(inputKeyframe.Easing);
        keyframeOutput.Composite = inputKeyframe.Composite;

        List<Pair<dom::DOMString, List<dom::DOMString>>> properties;
        properties.reserve(inputKeyframe.Properties.size());
        for (auto &[name, value] : inputKeyframe.Properties)
        {
          properties.push_back({krys::move(name), List<dom::DOMString> {krys::move(value)}});
        }

        for (const auto &propertyAndValue : ProcessKeyframeLikeObjectProperties(document, properties))
        {
          // With the "allow lists" flag set to false, there is only ever a single value for a given property.
          krys_debug_assert(propertyAndValue.Values.size() == 1);
          SetKeyframeProperty(keyframeOutput, propertyAndValue, propertyAndValue.Values[0], parserContext);
        }

        parsedKeyframes.push_back(krys::move(keyframeOutput));
      }

      return {};
    }

    KRYS_NODISCARD dom::ExceptionOr<void> ProcessPropertyIndexedKeyframes(
      dom::Document &document, KeyframeEffect::InputPropertyIndexedKeyframes &&keyframesInput,
      List<KeyframeEffect::ParsedKeyframe> &parsedKeyframes, List<dom::DOMString> &unusedEasings) noexcept
    {
      // 1. Let property-indexed keyframe be the result of running the procedure to process a keyframe-like
      // object passing object as the keyframe input.
      auto &baseProperties = static_cast<KeyframeEffect::BasePropertyIndexedKeyframe &>(keyframesInput);

      // Convert string offsets to TimelineRangeOffset in case we were provided with a list of offsets.
      if (auto *offsets = std::get_if<List<KeyframeEffect::KeyframeOffset>>(&baseProperties.Offset))
      {
        for (auto &offset : *offsets)
        {
          const auto *stringOffset = std::get_if<dom::DOMString>(&offset);
          if (!stringOffset)
          {
            continue;
          }
          auto parsedValue = DoubleOrTimelineRangeOffsetFromString(*stringOffset, document);
          if (!parsedValue)
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          if (const auto *doubleOffset = std::get_if<double>(&*parsedValue))
          {
            offset = *doubleOffset;
          }
          else
          {
            offset = std::get<TimelineRangeOffset>(*parsedValue);
          }
        }
      }

      auto propertiesAndValues = ProcessKeyframeLikeObjectProperties(document, keyframesInput.Properties);

      css::ParserContext parserContext(document);

      // 2. For each member, m, in property-indexed keyframe, perform the following steps:
      for (const auto &m : propertiesAndValues)
      {
        // 1. Let property name be the key for m.
        // 2. If property name is “composite”, or “easing”, or “offset”, skip the remaining steps in this loop
        //    and continue from the next member in property-indexed keyframe after m.
        //    We skip this test since we split those properties and the actual CSS properties that we're
        //    currently iterating over.
        // 3. Let property values be the value for m.
        // 4. Let property keyframes be an empty sequence of keyframes.
        List<KeyframeEffect::ParsedKeyframe> propertyKeyframes;
        // 5. For each value, v, in property values perform the following steps:
        for (const auto &v : m.Values)
        {
          // 1. Let k be a new keyframe with a null keyframe offset.
          KeyframeEffect::ParsedKeyframe k;
          // 2. Add the property-value pair, property name → v, to k.
          SetKeyframeProperty(k, m, v, parserContext);
          // 3. Append k to property keyframes.
          propertyKeyframes.push_back(krys::move(k));
        }
        // 6. Apply the procedure to compute missing keyframe offsets to property keyframes.
        ComputeMissingKeyframeOffsets(propertyKeyframes);

        // 7. Add keyframes in property keyframes to processed keyframes.
        parsedKeyframes.insert(parsedKeyframes.end(), std::make_move_iterator(propertyKeyframes.begin()),
                               std::make_move_iterator(propertyKeyframes.end()));
      }

      // 3. Sort processed keyframes by the computed keyframe offset of each keyframe in increasing order.
      std::ranges::stable_sort(parsedKeyframes,
                               [](const auto &lhs, const auto &rhs)
                               {
                                 if (!std::isnan(lhs.ComputedOffset) && !std::isnan(rhs.ComputedOffset))
                                 {
                                   return lhs.ComputedOffset < rhs.ComputedOffset;
                                 }
                                 // This will sort null values prior to other values.
                                 return !std::isnan(lhs.ComputedOffset) && std::isnan(rhs.ComputedOffset);
                               });

      // 4. Merge adjacent keyframes in processed keyframes when they have equal computed keyframe offsets.
      size_t i = 1;
      while (i < parsedKeyframes.size())
      {
        auto &keyframe = parsedKeyframes[i];
        auto &previousKeyframe = parsedKeyframes[i - 1];
        // If the offsets of this keyframe and the previous keyframe are different, this means that the two
        // keyframes should not be merged and we can move on to the next keyframe.
        if (keyframe.ComputedOffset != previousKeyframe.ComputedOffset)
        {
          i++;
          continue;
        }
        // Otherwise, both this keyframe and the previous keyframe should be merged. Unprocessed keyframes in
        // parsedKeyframes at this stage have at most a single property in cssPropertiesAndValues, so just set
        // this on the previous keyframe. In case an invalid or null value was originally provided, then the
        // property was not set and the property count is 0, in which case there is nothing to merge.
        if (!keyframe.StyleStrings.empty())
        {
          (void)previousKeyframe.Style->MergeAndOverrideOnConflict(*keyframe.Style);
          for (const auto &[property, value] : keyframe.StyleStrings)
          {
            previousKeyframe.StyleStrings.insert_or_assign(property, value);
          }
        }
        if (!keyframe.CustomStyleStrings.empty())
        {
          (void)previousKeyframe.Style->MergeAndOverrideOnConflict(*keyframe.Style);
          for (const auto &[customProperty, value] : keyframe.CustomStyleStrings)
          {
            previousKeyframe.CustomStyleStrings.insert_or_assign(customProperty, value);
          }
        }
        // Since we've processed this keyframe, we can remove it and keep i the same so that we process the
        // next keyframe in the next loop iteration.
        parsedKeyframes.erase(parsedKeyframes.begin() + static_cast<ptrdiff_t>(i));
      }

      // 5. Let offsets be a sequence of nullable double values assigned based on the type of the “offset”
      // member
      //    of the property-indexed keyframe as follows:
      //    - sequence<double?>, the value of “offset” as-is.
      //    - double?, a sequence of length one with the value of “offset” as its single item, i.e. « offset
      //    »,
      List<KeyframeEffect::KeyframeOffset> offsets;
      const auto &sourceOffsets = baseProperties.Offset;
      if (const auto *listOfKeyframeOffsets =
            std::get_if<List<KeyframeEffect::KeyframeOffset>>(&sourceOffsets))
      {
        for (const auto &keyframeOffset : *listOfKeyframeOffsets)
        {
          auto validatedOffset = ValidateKeyframeOffset(keyframeOffset, document);
          if (!validatedOffset)
          {
            return dom::Exception {dom::ExceptionCode::TypeError};
          }
          offsets.push_back(*validatedOffset);
        }
      }
      else if (const auto *doubleValue = std::get_if<double>(&sourceOffsets))
      {
        offsets.push_back(*doubleValue);
      }
      else if (const auto *timelineRangeOffset = std::get_if<TimelineRangeOffset>(&sourceOffsets))
      {
        if (!IsTimelineRangeOffsetValid(*timelineRangeOffset))
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
        offsets.push_back(*timelineRangeOffset);
      }
      else if (const auto *stringOffset = std::get_if<dom::DOMString>(&sourceOffsets))
      {
        auto parsedValue = DoubleOrTimelineRangeOffsetFromString(*stringOffset, document);
        if (!parsedValue)
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }
        if (const auto *doubleOffset = std::get_if<double>(&*parsedValue))
        {
          offsets.push_back(*doubleOffset);
        }
        else
        {
          offsets.push_back(std::get<TimelineRangeOffset>(*parsedValue));
        }
      }
      else
      {
        offsets.push_back(nullptr);
      }

      // 6. Assign each value in offsets to the keyframe offset of the keyframe with corresponding position in
      //    property keyframes until the end of either sequence is reached.
      for (size_t index = 0; index < offsets.size() && index < parsedKeyframes.size(); ++index)
      {
        parsedKeyframes[index].Offset = offsets[index];
      }

      // 7. Let easings be a sequence of DOMString values assigned based on the type of the “easing” member of
      //    the property-indexed keyframe as follows:
      //    - sequence<DOMString>, the value of “easing” as-is.
      //    - DOMString, a sequence of length one with the value of “easing” as its single item, i.e. « easing
      //    »,
      List<dom::DOMString> easings;
      if (const auto *easingList = std::get_if<List<dom::DOMString>>(&baseProperties.Easing))
      {
        easings = *easingList;
      }
      else if (const auto *easing = std::get_if<dom::DOMString>(&baseProperties.Easing))
      {
        easings.push_back(*easing);
      }

      // 8. If easings is an empty sequence, let it be a sequence of length one containing the single value
      //    “linear”, i.e. « "linear" ».
      if (easings.empty())
      {
        easings.push_back(u8"linear");
      }

      // 9. If easings has fewer items than property keyframes, repeat the elements in easings successively
      //    starting from the beginning of the list until easings has as many items as property keyframes.
      if (easings.size() < parsedKeyframes.size())
      {
        size_t initialNumberOfEasings = easings.size();
        for (i = initialNumberOfEasings; i < parsedKeyframes.size(); ++i)
        {
          easings.push_back(easings[i % initialNumberOfEasings]);
        }
      }

      // 10. If easings has more items than property keyframes, store the excess items as unused easings.
      while (easings.size() > parsedKeyframes.size())
      {
        unusedEasings.push_back(krys::move(easings.back()));
        easings.pop_back();
      }

      // 11. Assign each value in easings to a property named “easing” on the keyframe with the corresponding
      //     position in property keyframes until the end of property keyframes is reached.
      for (size_t index = 0; index < parsedKeyframes.size(); ++index)
      {
        parsedKeyframes[index].Easing = easings[index];
      }

      // 12. If the “composite” member of the property-indexed keyframe is not an empty sequence:
      List<CompositeOperationOrAuto> compositeModes;
      if (const auto *compositeList = std::get_if<List<CompositeOperationOrAuto>>(&baseProperties.Composite))
      {
        compositeModes = *compositeList;
      }
      else if (const auto *composite = std::get_if<CompositeOperationOrAuto>(&baseProperties.Composite))
      {
        compositeModes.push_back(*composite);
      }
      if (!compositeModes.empty())
      {
        // 1. Let composite modes be a sequence of CompositeOperationOrAuto values assigned from the
        // “composite”
        //    member of property-indexed keyframe. If that member is a single CompositeOperationOrAuto value
        //    operation, let composite modes be a sequence of length one, with the value of the “composite” as
        //    its single item.
        // 2. As with easings, if composite modes has fewer items than processed keyframes, repeat the
        // elements
        //    in composite modes successively starting from the beginning of the list until composite modes
        //    has as many items as processed keyframes.
        if (compositeModes.size() < parsedKeyframes.size())
        {
          size_t initialNumberOfCompositeModes = compositeModes.size();
          for (i = initialNumberOfCompositeModes; i < parsedKeyframes.size(); ++i)
          {
            compositeModes.push_back(compositeModes[i % initialNumberOfCompositeModes]);
          }
        }
        // 3. Assign each value in composite modes that is not auto to the keyframe-specific composite
        // operation
        //    on the keyframe with the corresponding position in processed keyframes until the end of
        //    processed keyframes is reached.
        for (size_t index = 0; index < compositeModes.size() && index < parsedKeyframes.size(); ++index)
        {
          if (compositeModes[index] != CompositeOperationOrAuto::Auto)
          {
            parsedKeyframes[index].Composite = compositeModes[index];
          }
        }
      }

      return {};
    }

    KRYS_NODISCARD TimelineRangeOffset
      TimelineRangeOffsetFromSpecifiedOffset(const BlendingKeyframe::KeyframeOffset &specifiedOffset) noexcept
    {
      auto name = style::ConvertSingleAnimationRangeNameToRangeString(specifiedOffset.Name);
      return TimelineRangeOffset {name, css::CSSNumericFactory::Percent(specifiedOffset.Value * 100)};
    }

    KRYS_NODISCARD BlendingKeyframe::KeyframeOffset
      SpecifiedOffsetForParsedKeyframe(const KeyframeEffect::ParsedKeyframe &keyframe) noexcept
    {
      if (const auto *timelineRangeOffset = std::get_if<TimelineRangeOffset>(&keyframe.Offset))
      {
        auto rangeName = style::ConvertRangeStringToSingleTimelineRangeName(timelineRangeOffset->RangeName);
        auto *offsetUnitValue = DynamicDowncast<css::CSSUnitValue>(timelineRangeOffset->Offset.get());
        krys_debug_assert(offsetUnitValue && offsetUnitValue->UnitEnum() == css::UnitType::Percentage);
        return {rangeName, offsetUnitValue->Value() / 100};
      }

      krys_debug_assert(!std::isnan(keyframe.ComputedOffset));
      return keyframe.ComputedOffset;
    }
  }

  KeyframeEffect::~KeyframeEffect() noexcept
  {
    if (_inTargetEffectStack)
    {
      if (auto target = TargetStyleable())
      {
        if (auto *keyframeEffectStack = target->GetKeyframeEffectStack())
        {
          keyframeEffectStack->RemoveEffect(*this);
        }
      }
    }

    krys_debug_assert(!_inTargetEffectStack);
  }

  KeyframeEffect::ParsedKeyframe::ParsedKeyframe() noexcept : Style(css::MutableStyleProperties::Create())
  {
  }

  KeyframeEffect::ParsedKeyframe::~ParsedKeyframe() noexcept = default;
  KeyframeEffect::ParsedKeyframe::ParsedKeyframe(ParsedKeyframe &&) noexcept = default;
  KeyframeEffect::ParsedKeyframe &
    KeyframeEffect::ParsedKeyframe::operator=(ParsedKeyframe &&) noexcept = default;

  dom::DOMString KeyframeEffect::PropertyIdToIDLAttributeName(css::PropertyId property) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#animation-property-name-to-idl-attribute-name
    // 1. If property follows the <custom-property-name> production, return property.

    // 2. If property refers to the CSS float property, return the string "cssFloat".
    if (property == css::PropertyId::Float)
    {
      return u8"cssFloat";
    }

    // 3. If property refers to the CSS offset property, return the string "cssOffset".
    if (property == css::PropertyId::Offset)
    {
      return u8"cssOffset";
    }

    // 4. Otherwise, return the result of applying the CSS property to IDL attribute algorithm [CSSOM] to
    // property.
    return css::NameForIDL(property);
  }

  dom::ExceptionOr<Ref<KeyframeEffect>>
    KeyframeEffect::Create(dom::Document &document, dom::Element *target, KeyframesInput &&keyframes,
                           Maybe<Variant<double, KeyframeEffectOptions>> &&options) noexcept
  {
    Ref keyframeEffect = AdoptRef(*new KeyframeEffect(target, {}));
    keyframeEffect->_document = CreateWeakPtr(&document);

    if (options)
    {
      OptionalEffectTiming timing;
      auto &optionsValue = *options;
      if (const auto *duration = std::get_if<double>(&optionsValue))
      {
        timing.Duration = Variant<double, dom::DOMString> {*duration};
      }
      else
      {
        auto &keyframeEffectOptions = std::get<KeyframeEffectOptions>(optionsValue);

        auto setPseudoElementResult = keyframeEffect->SetPseudoElement(keyframeEffectOptions.PseudoElement);
        if (setPseudoElementResult.HasException())
        {
          return setPseudoElementResult.ReleaseException();
        }

        auto convertedDuration = keyframeEffectOptions.DurationAsDoubleOrString();
        if (!convertedDuration)
        {
          return dom::Exception {dom::ExceptionCode::TypeError};
        }

        timing = {
          .Duration = *convertedDuration,
          .Iterations = keyframeEffectOptions.Iterations,
          .Delay = keyframeEffectOptions.Delay,
          .EndDelay = keyframeEffectOptions.EndDelay,
          .IterationStart = keyframeEffectOptions.IterationStart,
          .Easing = keyframeEffectOptions.Easing,
          .Fill = keyframeEffectOptions.Fill,
          .Direction = keyframeEffectOptions.Direction,
        };

        keyframeEffect->SetComposite(keyframeEffectOptions.Composite);
        keyframeEffect->SetIterationComposite(keyframeEffectOptions.IterationComposite);
      }
      auto updateTimingResult = keyframeEffect->UpdateTiming(document, timing);
      if (updateTimingResult.HasException())
      {
        return updateTimingResult.ReleaseException();
      }
    }

    auto processKeyframesResult = keyframeEffect->ProcessKeyframes(document, krys::move(keyframes));
    if (processKeyframesResult.HasException())
    {
      return processKeyframesResult.ReleaseException();
    }

    return keyframeEffect;
  }

  Ref<KeyframeEffect> KeyframeEffect::Create(Ref<KeyframeEffect> &&source) noexcept
  {
    Ref keyframeEffect = AdoptRef(*new KeyframeEffect(nullptr, {}));
    keyframeEffect->CopyPropertiesFromSource(krys::move(source));
    return keyframeEffect;
  }

  Ref<KeyframeEffect>
    KeyframeEffect::Create(const dom::Element &target,
                           const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    return AdoptRef(*new KeyframeEffect(const_cast<dom::Element *>(&target), pseudoElementIdentifier));
  }

  KeyframeEffect::KeyframeEffect(
    dom::Element *target, const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
      : _target(ShareRefPtr(target)), _pseudoElementIdentifier(pseudoElementIdentifier)
  {
    if (_target)
    {
      _document = CreateWeakPtr(&_target->NodeDocument());
    }
  }

  void KeyframeEffect::CopyPropertiesFromSource(Ref<KeyframeEffect> &&source) noexcept
  {
    _target = source->_target;
    _pseudoElementIdentifier = source->_pseudoElementIdentifier;
    _document = source->_document;
    _compositeOperation = source->_compositeOperation;
    _iterationCompositeOperation = source->_iterationCompositeOperation;

    List<ParsedKeyframe> parsedKeyframes;
    for (const auto &sourceParsedKeyframe : source->_parsedKeyframes)
    {
      ParsedKeyframe parsedKeyframe;
      parsedKeyframe.Easing = sourceParsedKeyframe.Easing;
      parsedKeyframe.Offset = sourceParsedKeyframe.Offset;
      parsedKeyframe.Composite = sourceParsedKeyframe.Composite;
      parsedKeyframe.StyleStrings = sourceParsedKeyframe.StyleStrings;
      parsedKeyframe.CustomStyleStrings = sourceParsedKeyframe.CustomStyleStrings;
      parsedKeyframe.ComputedOffset = sourceParsedKeyframe.ComputedOffset;
      parsedKeyframe.TimingFunction = sourceParsedKeyframe.TimingFunction;
      parsedKeyframe.Style = sourceParsedKeyframe.Style->MutableCopy();
      parsedKeyframes.push_back(krys::move(parsedKeyframe));
    }
    _parsedKeyframes = krys::move(parsedKeyframes);

    SetFill(source->Fill());
    SetDelay(source->SpecifiedDelay());
    SetEndDelay(source->SpecifiedEndDelay());
    SetDirection(source->Direction());
    (void)SetIterations(source->Iterations());
    SetTimingFunction(ShareRefPtr(source->TimingFunction()));
    (void)SetIterationStart(source->IterationStart());
    SetIterationDuration(source->SpecifiedIterationDuration());

    BlendingKeyframes blendingKeyframes(_blendingKeyframes.Identifier());
    blendingKeyframes.CopyKeyframes(source->_blendingKeyframes);
    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  auto KeyframeEffect::GetKeyframes() noexcept -> List<ComputedKeyframe>
  {
    // https://drafts.csswg.org/web-animations-1/#dom-keyframeeffectreadonly-getkeyframes

    if (RefPtr styleOriginatedAnimation = ShareRefPtr(DynamicDowncast<StyleOriginatedAnimation>(Animation())))
    {
      styleOriginatedAnimation->FlushPendingStyleChanges();
    }

    // FIXME: Updating the computed offsets of keyframes in named timeline ranges
    // (`updateComputedKeyframeOffsetsIfNeeded`), once there are view timelines.

    List<ComputedKeyframe> computedKeyframes;

    if (!_parsedKeyframes.empty() || _animationType == WebAnimationType::WebAnimation
        || !_blendingKeyframes.ContainsAnimatableCSSProperty())
    {
      for (size_t i = 0; i < _parsedKeyframes.size(); ++i)
      {
        const auto &parsedKeyframe = _parsedKeyframes[i];
        ComputedKeyframe computedKeyframe {static_cast<const ComputedKeyframe &>(parsedKeyframe)};
        for (auto &[cssPropertyId, stringValue] : computedKeyframe.StyleStrings)
        {
          if (cssPropertyId == css::PropertyId::Custom)
          {
            continue;
          }
          if (auto cssValue = parsedKeyframe.Style->GetPropertyCSSValue(cssPropertyId))
          {
            stringValue = cssValue->CssText(css::DefaultSerialisationContext());
          }
        }
        computedKeyframe.Easing = TimingFunctionForKeyframeAtIndex(i)->CssText();
        computedKeyframes.push_back(krys::move(computedKeyframe));
      }
      return computedKeyframes;
    }

    RefPtr target = _target;
    const auto *lastStyleChangeEventStyle = TargetStyleable()->LastStyleChangeEventStyle();
    const auto &elementStyle = lastStyleChangeEventStyle ? *lastStyleChangeEventStyle : CurrentStyle();

    style::Extractor computedStyleExtractor {target.get(), false, _pseudoElementIdentifier};

    BlendingKeyframes computedBlendingKeyframes(_blendingKeyframes.Identifier());
    computedBlendingKeyframes.CopyKeyframes(_blendingKeyframes);

    // FIXME: Or when there's an active view timeline, once there are view timelines.
    if (computedBlendingKeyframes.HasKeyframeNotUsingRangeOffset())
    {
      computedBlendingKeyframes.FillImplicitKeyframes(*this, elementStyle);
    }

    auto keyframeRules = [&]() -> List<Ref<css::KeyframeRule>>
    {
      RefPtr cssAnimation = ShareRefPtr(DynamicDowncast<CSSAnimation>(Animation()));
      if (!cssAnimation)
      {
        return {};
      }

      if (!_target || !_target->IsConnected())
      {
        return {};
      }

      const auto &backingStyleAnimation = cssAnimation->BackingStyleAnimation();
      auto *styleScope =
        style::Scope::ForOrdinal(*_target, backingStyleAnimation.GetName().TryKeyframesName()->ScopeOrdinal);
      if (!styleScope)
      {
        return {};
      }

      return styleScope->GetResolver().KeyframeRulesForName(
        computedBlendingKeyframes.KeyframesName(), backingStyleAnimation.GetTimingFunction().Value.get());
    }();

    auto matchingStyleRuleKeyframe = [&](const BlendingKeyframe &keyframe) -> css::KeyframeRule *
    {
      auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation());
      if (!cssAnimation)
      {
        return nullptr;
      }

      const auto &backingStyleAnimation = cssAnimation->BackingStyleAnimation();
      auto defaultCompositeOperation = backingStyleAnimation.GetCompositeOperation();
      RefPtr<boo::TimingFunction> defaultTimingFunction = backingStyleAnimation.GetTimingFunction().Value;

      auto compositeOperation = keyframe.CompositeOperation().value_or(defaultCompositeOperation);
      RefPtr<boo::TimingFunction> timingFunction = ShareRefPtr(keyframe.GetTimingFunction());
      if (!timingFunction)
      {
        timingFunction = defaultTimingFunction;
      }

      auto compositeOperationForStyleRuleKeyframe = [&](const Ref<css::KeyframeRule> &styleRuleKeyframe)
      {
        if (auto compositeOperationCSSValue =
              styleRuleKeyframe->Properties().GetPropertyCSSValue(css::PropertyId::AnimationComposition))
        {
          if (auto keyframeCompositeOperation = ToCompositeOperation(*compositeOperationCSSValue))
          {
            return *keyframeCompositeOperation;
          }
        }
        return defaultCompositeOperation;
      };

      auto timingFunctionForStyleRuleKeyframe =
        [&](const Ref<css::KeyframeRule> &styleRuleKeyframe) -> RefPtr<const boo::TimingFunction>
      {
        if (auto timingFunctionCSSValue =
              styleRuleKeyframe->Properties().GetPropertyCSSValue(css::PropertyId::AnimationTimingFunction))
        {
          if (auto keyframeTimingFunction = style::CreateTimingFunctionDeprecated(*timingFunctionCSSValue))
          {
            return keyframeTimingFunction;
          }
        }
        if (defaultTimingFunction)
        {
          return defaultTimingFunction;
        }
        return ShareRefPtr<const boo::TimingFunction>(&CubicBezierTimingFunction::DefaultTimingFunction());
      };

      const auto &specifiedOffset = keyframe.SpecifiedOffset();
      css::KeyframeRule::Key key {style::ConvertSingleAnimationRangeNameToValueId(specifiedOffset.Name),
                                  specifiedOffset.Value};

      for (const auto &keyframeRule : keyframeRules)
      {
        if (compositeOperationForStyleRuleKeyframe(keyframeRule) != compositeOperation)
        {
          continue;
        }
        if (timingFunctionForStyleRuleKeyframe(keyframeRule).get() != timingFunction.get())
        {
          continue;
        }
        for (const auto &keyframeRuleKey : keyframeRule->Keys())
        {
          if (keyframeRuleKey == key)
          {
            return keyframeRule.get();
          }
        }
      }
      return nullptr;
    };

    auto styleProperties = css::MutableStyleProperties::Create();
    if (_animationType == WebAnimationType::CSSAnimation && _target->IsConnected())
    {
      Maybe<style::PseudoElementRequest> pseudoElementRequest;
      if (_pseudoElementIdentifier)
      {
        pseudoElementRequest = style::PseudoElementRequest(*_pseudoElementIdentifier);
      }
      auto matchingRules = _target->StyleResolver().PseudoStyleRulesForElement(
        target.get(), pseudoElementRequest, style::Resolver::AllCSSRules);
      for (const auto &matchedRule : matchingRules)
      {
        (void)styleProperties->MergeAndOverrideOnConflict(matchedRule->Properties());
      }
      if (auto *styledElement = DynamicDowncast<dom::StyledElement>(*_target);
          styledElement && !_pseudoElementIdentifier)
      {
        if (const auto *inlineProperties = styledElement->InlineStyle())
        {
          (void)styleProperties->MergeAndOverrideOnConflict(*inlineProperties);
        }
      }
    }

    List<ComputedKeyframe> computedKeyframesWithTimelineRangeOffset;
    List<ComputedKeyframe> computedKeyframesWithDoubleOffset;

    for (const auto &keyframe : computedBlendingKeyframes)
    {
      const auto &style = *keyframe.Style();
      auto *keyframeRule = matchingStyleRuleKeyframe(keyframe);

      ComputedKeyframe computedKeyframe;
      if (keyframe.UsesRangeOffset())
      {
        computedKeyframe.Offset = TimelineRangeOffsetFromSpecifiedOffset(keyframe.SpecifiedOffset());
      }
      else
      {
        computedKeyframe.Offset = keyframe.SpecifiedOffset().Value;
      }
      computedKeyframe.ComputedOffset = keyframe.Offset();
      // For CSS transitions, all keyframes should return "linear" since the effect's global timing function
      // applies.
      computedKeyframe.Easing = Animation() && Is<CSSTransition>(*Animation())
                                  ? dom::DOMString(u8"linear")
                                  : TimingFunctionForBlendingKeyframe(keyframe)->CssText();

      if (auto compositeOperation = keyframe.CompositeOperation())
      {
        computedKeyframe.Composite = ToCompositeOperationOrAuto(*compositeOperation);
      }

      auto addPropertyToKeyframe = [&](css::PropertyId cssPropertyId)
      {
        dom::DOMString styleString;
        if (keyframeRule)
        {
          if (auto cssValue = keyframeRule->Properties().GetPropertyCSSValue(cssPropertyId))
          {
            if (!cssValue->HasVariableReferences())
            {
              styleString = keyframeRule->Properties().GetPropertyValue(cssPropertyId);
            }
          }
        }
        if (styleString.empty())
        {
          if (auto cssValue = styleProperties->GetPropertyCSSValue(cssPropertyId))
          {
            if (!cssValue->HasVariableReferences())
            {
              styleString = styleProperties->GetPropertyValue(cssPropertyId);
            }
          }
        }
        if (styleString.empty())
        {
          styleString = computedStyleExtractor.PropertyValueSerialisationInStyle(
            style, cssPropertyId, css::DefaultSerialisationContext(), css::ValuePool::MainThreadPool(),
            nullptr, style::ExtractorState::PropertyValueType::Computed);
        }
        computedKeyframe.StyleStrings.insert_or_assign(cssPropertyId, styleString);
      };

      auto addCustomPropertyToKeyframe = [&](const css::CSSOMStringAtom &customProperty)
      {
        dom::DOMString styleString;
        if (keyframeRule)
        {
          if (auto cssValue = keyframeRule->Properties().GetCustomPropertyCSSValue(customProperty.View()))
          {
            if (!cssValue->HasVariableReferences())
            {
              styleString = keyframeRule->Properties().GetCustomPropertyValue(customProperty.View());
            }
          }
        }
        if (styleString.empty())
        {
          if (auto cssValue = styleProperties->GetCustomPropertyCSSValue(customProperty.View()))
          {
            if (!cssValue->HasVariableReferences())
            {
              styleString = styleProperties->GetCustomPropertyValue(customProperty.View());
            }
          }
        }
        if (styleString.empty())
        {
          styleString = computedStyleExtractor.CustomPropertyValueSerialisationInStyle(
            style, customProperty, css::DefaultSerialisationContext());
        }
        computedKeyframe.CustomStyleStrings.insert_or_assign(customProperty, styleString);
      };

      for (const auto &property : keyframe.Properties())
      {
        krys::SwitchOn(
          property, [&](css::PropertyId cssProperty) { addPropertyToKeyframe(cssProperty); },
          [&](const css::CSSOMStringAtom &customProperty)
          {
            if (_animationType != WebAnimationType::CSSAnimation)
            {
              addCustomPropertyToKeyframe(customProperty);
            }
          });
      }

      // FIXME: this is so that we mimic the Chrome behavior since this isn't spec'd out, but it makes little
      // sense to me. Items ought to be sorted by computed offset just like BlendingKeyframes would organize
      // its keyframes. https://github.com/w3c/csswg-drafts/issues/11467
      if (std::holds_alternative<double>(computedKeyframe.Offset))
      {
        computedKeyframesWithDoubleOffset.push_back(krys::move(computedKeyframe));
      }
      else
      {
        computedKeyframesWithTimelineRangeOffset.push_back(krys::move(computedKeyframe));
      }
    }

    computedKeyframes.insert(computedKeyframes.end(),
                             std::make_move_iterator(computedKeyframesWithDoubleOffset.begin()),
                             std::make_move_iterator(computedKeyframesWithDoubleOffset.end()));
    computedKeyframes.insert(computedKeyframes.end(),
                             std::make_move_iterator(computedKeyframesWithTimelineRangeOffset.begin()),
                             std::make_move_iterator(computedKeyframesWithTimelineRangeOffset.end()));

    return computedKeyframes;
  }

  dom::ExceptionOr<void> KeyframeEffect::SetBindingsKeyframes(dom::Document &document,
                                                              KeyframesInput &&keyframesInput) noexcept
  {
    auto retVal = SetKeyframes(document, krys::move(keyframesInput));
    if (!retVal.HasException())
    {
      if (RefPtr cssAnimation = ShareRefPtr(DynamicDowncast<CSSAnimation>(Animation())))
      {
        cssAnimation->EffectKeyframesWereSetUsingBindings();
      }
    }
    return retVal;
  }

  dom::ExceptionOr<void> KeyframeEffect::SetKeyframes(dom::Document &document,
                                                      KeyframesInput &&keyframesInput) noexcept
  {
    auto processKeyframesResult = ProcessKeyframes(document, krys::move(keyframesInput));
    if (!processKeyframesResult.HasException() && Animation())
    {
      Animation()->EffectTimingDidChange();

      // Need a full style invalidation since the new keyframes may interact differently with the base style.
      if (auto target = TargetStyleable())
      {
        target->Element.InvalidateStyleInternal();
      }
    }

    return processKeyframesResult;
  }

  dom::ExceptionOr<void> KeyframeEffect::ProcessKeyframes(dom::Document &document,
                                                          KeyframesInput &&keyframesInput) noexcept
  {
    Ref protectedDocument = ShareRef(document);

    // 1. If object is null, return an empty sequence of keyframes.
    if (std::holds_alternative<std::nullptr_t>(keyframesInput))
    {
      return {};
    }

    // 2. Let processed keyframes be an empty sequence of keyframes.
    List<ParsedKeyframe> parsedKeyframes;

    // 3. to 5. Perform the steps corresponding to the first matching condition from below,
    List<dom::DOMString> unusedEasings;
    if (auto *keyframeList = std::get_if<List<InputKeyframe>>(&keyframesInput))
    {
      auto retVal = ProcessIterableKeyframes(document, krys::move(*keyframeList), parsedKeyframes);
      if (retVal.HasException())
      {
        return retVal.ReleaseException();
      }
    }
    else
    {
      auto retVal = ProcessPropertyIndexedKeyframes(
        document, krys::move(std::get<InputPropertyIndexedKeyframes>(keyframesInput)), parsedKeyframes,
        unusedEasings);
      if (retVal.HasException())
      {
        return retVal.ReleaseException();
      }
    }

    // 6. If processed keyframes is not loosely sorted by offset, throw a TypeError and abort these steps.
    // 7. If there exist any keyframe in processed keyframes whose keyframe offset is non-null and less than
    //    zero or greater than one, throw a TypeError and abort these steps.
    double lastNonNullOffset = -1;
    for (const auto &keyframe : parsedKeyframes)
    {
      const auto *doubleOffset = std::get_if<double>(&keyframe.Offset);
      if (!doubleOffset)
      {
        continue;
      }
      auto offset = *doubleOffset;
      if (offset < lastNonNullOffset || offset < 0 || offset > 1)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      lastNonNullOffset = offset;
    }

    // We take a slight detour from the spec text and compute the missing keyframe offsets right away since
    // they can be computed up-front.
    ComputeMissingKeyframeOffsets(parsedKeyframes);

    css::ParserContext parserContext(document);

    // 8. For each frame in processed keyframes, perform the following steps:
    for (auto &keyframe : parsedKeyframes)
    {
      // Let the timing function of frame be the result of parsing the “easing” property on frame using the
      // CSS syntax defined for the easing property of the AnimationEffectTiming interface. If parsing the
      // “easing” property fails, throw a TypeError and abort this procedure.

      // FIXME: Determine the how calc() and relative units should be resolved and switch to the
      // non-deprecated parsing function.
      auto timingFunctionResult =
        css::PropertyParserHelpers::ParseEasingFunctionDeprecated(keyframe.Easing, parserContext);
      if (!timingFunctionResult)
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
      keyframe.TimingFunction = krys::move(timingFunctionResult);
    }

    // 9. Parse each of the values in unused easings using the CSS syntax defined for easing property of the
    //    AnimationEffectTiming interface, and if any of the values fail to parse, throw a TypeError and abort
    //    this procedure.
    for (const auto &easing : unusedEasings)
    {
      // FIXME: Determine the how calc() and relative units should be resolved and switch to the
      // non-deprecated parsing function.
      if (!css::PropertyParserHelpers::ParseEasingFunctionDeprecated(easing, parserContext))
      {
        return dom::Exception {dom::ExceptionCode::TypeError};
      }
    }

    _parsedKeyframes = krys::move(parsedKeyframes);

    ClearBlendingKeyframes();

    Invalidate();

    return {};
  }

  void KeyframeEffect::UpdateBlendingKeyframes(RenderStyle &elementStyle,
                                               const style::ResolutionContext &resolutionContext) noexcept
  {
    // FIXME: Updating the computed offsets of keyframes in named timeline ranges
    // (`updateComputedKeyframeOffsetsIfNeeded`), once there are view timelines.

    if (!_blendingKeyframes.IsEmpty() || !_target)
    {
      return;
    }

    BlendingKeyframes blendingKeyframes(_blendingKeyframes.Identifier());
    auto &styleResolver = _target->StyleResolver();

    for (const auto &keyframe : _parsedKeyframes)
    {
      BlendingKeyframe blendingKeyframe(SpecifiedOffsetForParsedKeyframe(keyframe), nullptr);
      blendingKeyframe.SetTimingFunction(keyframe.TimingFunction->Clone());

      switch (keyframe.Composite)
      {
        case CompositeOperationOrAuto::Replace:
          blendingKeyframe.SetCompositeOperation(boo::CompositeOperation::Replace);
          break;
        case CompositeOperationOrAuto::Add:
          blendingKeyframe.SetCompositeOperation(boo::CompositeOperation::Add);
          break;
        case CompositeOperationOrAuto::Accumulate:
          blendingKeyframe.SetCompositeOperation(boo::CompositeOperation::Accumulate);
          break;
        case CompositeOperationOrAuto::Auto: break;
      }

      auto keyframeRule = css::KeyframeRule::Create(keyframe.Style->ImmutableCopyIfNeeded());
      blendingKeyframe.SetStyle(styleResolver.StyleForKeyframe(*_target, elementStyle, resolutionContext,
                                                               *keyframeRule, blendingKeyframe));
      blendingKeyframes.Insert(krys::move(blendingKeyframe));
      blendingKeyframes.UpdatePropertiesMetadata(keyframeRule->Properties());
    }

    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  const AnimatableCSSPropertySet &KeyframeEffect::AnimatedProperties() noexcept
  {
    if (!_blendingKeyframes.IsEmpty())
    {
      return _blendingKeyframes.Properties();
    }

    if (_animatedProperties.empty())
    {
      for (const auto &keyframe : _parsedKeyframes)
      {
        for (const auto &[keyframeCustomProperty, value] : keyframe.CustomStyleStrings)
        {
          _animatedProperties.insert(keyframeCustomProperty);
        }
        for (const auto &[keyframeProperty, value] : keyframe.StyleStrings)
        {
          _animatedProperties.insert(keyframeProperty);
        }
      }
    }

    return _animatedProperties;
  }

  bool KeyframeEffect::AnimatesProperty(const AnimatableCSSProperty &property) const noexcept
  {
    if (!_blendingKeyframes.IsEmpty())
    {
      return _blendingKeyframes.ContainsProperty(property);
    }

    return krys::SwitchOn(
      property,
      [&](css::PropertyId cssProperty)
      {
        return std::ranges::any_of(_parsedKeyframes, [&](const auto &keyframe)
                                   { return keyframe.StyleStrings.contains(cssProperty); });
      },
      [&](const css::CSSOMStringAtom &customProperty)
      {
        return std::ranges::any_of(_parsedKeyframes, [&](const auto &keyframe)
                                   { return keyframe.CustomStyleStrings.contains(customProperty); });
      });
  }

  void KeyframeEffect::ClearBlendingKeyframes() noexcept
  {
    _animationType = WebAnimationType::WebAnimation;
    _blendingKeyframes.Clear();
  }

  bool KeyframeEffect::ForceLayoutIfNeeded() noexcept
  {
    if (!_needsForcedLayout || !_target)
    {
      return false;
    }

    auto *renderer = Renderer();
    if (!renderer || !renderer->Parent())
    {
      return false;
    }

    krys_debug_assert(Document());
    auto *frameView = Document()->View();
    if (!frameView)
    {
      return false;
    }

    frameView->LayoutContext().ScheduleLayout();
    frameView->LayoutContext().Layout();
    return true;
  }

  void KeyframeEffect::SetBlendingKeyframes(BlendingKeyframes &&blendingKeyframes) noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);

    _blendingKeyframes = krys::move(blendingKeyframes);
    _animatedProperties.clear();

    ComputedNeedsForcedLayout();
    ComputeStackingContextImpact();
    ComputeAcceleratedPropertiesState();
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
    ComputeHasImplicitKeyframeForAcceleratedProperty();
    ComputeHasKeyframeComposingAcceleratedProperty();
    ComputeHasAcceleratedPropertyOverriddenByCascadeProperty();
    ComputeHasReferenceFilter();
    ComputeHasSizeDependentTransform();
    AnalyzeAcceleratedProperties();

    CheckForMatchingTransformFunctionLists();

    UpdateAcceleratedAnimationIfNecessary();
  }

  void KeyframeEffect::AnalyzeAcceleratedProperties() noexcept
  {
    _acceleratedProperties.clear();
    _acceleratedPropertiesWithImplicitKeyframe.clear();

    krys_debug_assert(Document());
    const auto &settings = Document()->GetSettings();
    for (const auto &property : _blendingKeyframes.Properties())
    {
      if (!style::interpolation::IsAccelerated(property, settings))
      {
        continue;
      }
      _acceleratedProperties.insert(property);
      if (_blendingKeyframes.HasImplicitKeyframeForProperty(property))
      {
        _acceleratedPropertiesWithImplicitKeyframe.insert(property);
      }
    }
  }

  void KeyframeEffect::CheckForMatchingTransformFunctionLists() noexcept
  {
    if (_blendingKeyframes.Size() < 2 || !_blendingKeyframes.ContainsProperty(css::PropertyId::Transform))
    {
      _transformFunctionListsMatchPrefix = 0;
      return;
    }

    TransformOperationsSharedPrimitivesPrefix<style::TransformFunctionType> prefix;
    for (const auto &keyframe : _blendingKeyframes)
    {
      prefix.Update(keyframe.Style()->GetTransform());
    }

    _transformFunctionListsMatchPrefix = static_cast<uint32>(prefix.Primitives().size());
  }

  Maybe<uint32> KeyframeEffect::TransformFunctionListPrefix() const noexcept
  {
    // The accelerated animation code can only use direct function interpolation when all keyframes share the
    // same prefix of shared transform function primitives, whereas software animations simply calls
    // blend(...) which can do direct interpolation based on the function list of any two particular
    // keyframes. The prefix serves as a way to make sure that the results of blend(...) can be made to return
    // the same results as rendered by the accelerated animation code.
    if (PreventsAcceleration())
    {
      return null;
    }
    return _transformFunctionListsMatchPrefix;
  }

  void KeyframeEffect::ComputedNeedsForcedLayout() noexcept
  {
    _needsForcedLayout = [&]()
    {
      if (Animation() && Is<CSSTransition>(*Animation()))
      {
        return false;
      }
      return _blendingKeyframes.HasWidthDependentTransform()
             || _blendingKeyframes.HasHeightDependentTransform();
    }();
  }

  void KeyframeEffect::ComputeStyleOriginatedAnimationBlendingKeyframes(
    const RenderStyle *oldStyle, const RenderStyle &newStyle,
    const style::ResolutionContext &resolutionContext) noexcept
  {
    krys_debug_assert(Animation() && Is<StyleOriginatedAnimation>(*Animation()));

    if (Animation() && Is<CSSAnimation>(*Animation()))
    {
      ComputeCSSAnimationBlendingKeyframes(newStyle, resolutionContext);
    }
    else if (Animation() && Is<CSSTransition>(*Animation()))
    {
      krys_debug_assert(oldStyle);
      ComputeCSSTransitionBlendingKeyframes(*oldStyle, newStyle);
    }
  }

  void KeyframeEffect::ComputeCSSAnimationBlendingKeyframes(
    const RenderStyle &unanimatedStyle, const style::ResolutionContext &resolutionContext) noexcept
  {
    krys_debug_assert(Document());

    const auto &backingStyleAnimation = DynamicDowncast<CSSAnimation>(Animation())->BackingStyleAnimation();

    // NOTE: A CSSAnimation is always constructed with a backing style::Animation that has a valid, non-none,
    // name.
    auto backingStyleAnimationName = backingStyleAnimation.GetName().TryKeyframesName();
    BlendingKeyframes blendingKeyframes(KeyframesIdentifier {backingStyleAnimationName->Name});
    if (_target)
    {
      (void)style::Scope::ResolveTreeScopedReference(
        *_target, *backingStyleAnimationName,
        [&](const style::Scope &scope, const css::CSSOMStringAtom &)
        {
          krys_debug_assert(scope.ResolverIfExists());
          return scope.ResolverIfExists()->KeyframeStylesForAnimation(
            *_target, unanimatedStyle, resolutionContext, blendingKeyframes,
            backingStyleAnimation.GetTimingFunction().Value.get());
        });

      // Ensure resource loads for all the frames.
      for (const auto &keyframe : blendingKeyframes)
      {
        if (auto *style = const_cast<RenderStyle *>(keyframe.Style()))
        {
          style::LoadPendingResources(*style, *Document(), _target.get());
        }
      }
    }

    _animationType = WebAnimationType::CSSAnimation;
    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  void KeyframeEffect::RecomputeKeyframesAtNextOpportunity() noexcept
  {
    ClearBlendingKeyframes();
    Invalidate();
  }

  void KeyframeEffect::ComputeCSSTransitionBlendingKeyframes(const RenderStyle &oldStyle,
                                                             const RenderStyle &newStyle) noexcept
  {
    krys_debug_assert(Document());

    if (_blendingKeyframes.Size())
    {
      return;
    }

    auto property = DynamicDowncast<CSSTransition>(Animation())->Property();

    auto toStyle = RenderStyle::ClonePtr(newStyle);
    if (_target)
    {
      style::LoadPendingResources(*toStyle, *Document(), _target.get());
    }

    BlendingKeyframes blendingKeyframes(_blendingKeyframes.Identifier());

    BlendingKeyframe fromBlendingKeyframe(0, RenderStyle::ClonePtr(oldStyle));
    fromBlendingKeyframe.AddProperty(property);
    blendingKeyframes.Insert(krys::move(fromBlendingKeyframe));

    BlendingKeyframe toBlendingKeyframe(1, krys::move(toStyle));
    toBlendingKeyframe.AddProperty(property);
    blendingKeyframes.Insert(krys::move(toBlendingKeyframe));

    _animationType = WebAnimationType::CSSTransition;
    SetBlendingKeyframes(krys::move(blendingKeyframes));
  }

  void KeyframeEffect::ComputeStackingContextImpact() noexcept
  {
    _triggersStackingContext = false;
    for (const auto &property : _blendingKeyframes.Properties())
    {
      if (const auto *propertyId = std::get_if<css::PropertyId>(&property);
          propertyId && style::WillChangeAnimatableFeature::PropertyCreatesStackingContext(*propertyId))
      {
        _triggersStackingContext = true;
        break;
      }
    }
  }

  void KeyframeEffect::UpdateIsAssociatedWithProgressBasedTimeline() noexcept
  {
    auto wasAssociatedWithProgressBasedTimeline = _isAssociatedWithProgressBasedTimeline;

    _isAssociatedWithProgressBasedTimeline = [&]
    {
      if (auto *animation = Animation())
      {
        if (auto *timeline = animation->Timeline())
        {
          return timeline->IsProgressBased();
        }
      }
      return false;
    }();

    if (wasAssociatedWithProgressBasedTimeline != _isAssociatedWithProgressBasedTimeline)
    {
      UpdateAcceleratedAnimationIfNecessary();
    }
  }

  void KeyframeEffect::AnimationTimelineDidChange() noexcept
  {
    AnimationEffect::AnimationTimelineDidChange();

    UpdateIsAssociatedWithProgressBasedTimeline();

    UpdateEffectStackMembership();
  }

  void KeyframeEffect::AnimationRelevancyDidChange() noexcept
  {
    UpdateEffectStackMembership();
  }

  void KeyframeEffect::UpdateEffectStackMembership() noexcept
  {
    auto target = TargetStyleable();
    if (!target)
    {
      return;
    }

    bool isRelevant = Animation() && Animation()->IsRelevant();
    if (isRelevant && !_inTargetEffectStack)
    {
      (void)target->EnsureKeyframeEffectStack().AddEffect(*this);
    }
    else if (!isRelevant && _inTargetEffectStack)
    {
      target->EnsureKeyframeEffectStack().RemoveEffect(*this);
    }
  }

  void KeyframeEffect::SetAnimation(WebAnimation *animation) noexcept
  {
    bool animationChanged = animation != Animation();
    AnimationEffect::SetAnimation(animation);
    if (!animationChanged)
    {
      return;
    }

    if (_animationType == WebAnimationType::CSSAnimation)
    {
      ClearBlendingKeyframes();
    }
    UpdateEffectStackMembership();

    UpdateIsAssociatedWithProgressBasedTimeline();
  }

  Maybe<style::Styleable> KeyframeEffect::TargetStyleable() const noexcept
  {
    if (_target)
    {
      return style::Styleable(*_target, _pseudoElementIdentifier);
    }
    return null;
  }

  bool KeyframeEffect::TargetsPseudoElement() const noexcept
  {
    return _target && _pseudoElementIdentifier;
  }

  void KeyframeEffect::SetTarget(RefPtr<dom::Element> &&newTarget) noexcept
  {
    if (_target == newTarget)
    {
      return;
    }

    auto previousTargetStyleable = TargetStyleable();
    RefPtr<dom::Element> protector;
    if (previousTargetStyleable)
    {
      protector = previousTargetStyleable->ProtectedElement();
    }

    _target = krys::move(newTarget);
    DidChangeTargetStyleable(previousTargetStyleable);
  }

  css::CSSOMString KeyframeEffect::PseudoElement() const noexcept
  {
    // https://drafts.csswg.org/web-animations/#dom-keyframeeffect-pseudoelement
    // The target pseudo-selector. null if this effect has no effect target or if the effect target is an
    // element (i.e. not a pseudo-element). When the effect target is a pseudo-element, this specifies the
    // pseudo-element selector (e.g. ::before).
    if (TargetsPseudoElement())
    {
      return PseudoElementIdentifierAsString(_pseudoElementIdentifier);
    }
    return {};
  }

  dom::ExceptionOr<void> KeyframeEffect::SetPseudoElement(const css::CSSOMString &pseudoElement) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-keyframeeffect-pseudoelement
    auto [parsed, pseudoElementIdentifier] = PseudoElementIdentifierFromString(pseudoElement, Document());
    if (!parsed)
    {
      return dom::Exception {dom::ExceptionCode::SyntaxError, u8"Parsing pseudo-element selector failed"};
    }

    if (_pseudoElementIdentifier == pseudoElementIdentifier)
    {
      return {};
    }

    auto previousTargetStyleable = TargetStyleable();
    _pseudoElementIdentifier = pseudoElementIdentifier;
    DidChangeTargetStyleable(previousTargetStyleable);

    return {};
  }

  void
    KeyframeEffect::DidChangeTargetStyleable(const Maybe<style::Styleable> &previousTargetStyleable) noexcept
  {
    auto newTargetStyleable = TargetStyleable();

    if (RefPtr effectAnimation = ShareRefPtr(Animation()))
    {
      effectAnimation->EffectTargetDidChange(previousTargetStyleable, newTargetStyleable);
    }

    ClearBlendingKeyframes();

    // We need to invalidate the effect now that the target has changed to ensure the effect's styles are
    // applied to the new target right away.
    Invalidate();

    // Likewise, we need to invalidate styles on the previous target so that any animated styles are removed
    // immediately.
    InvalidateElement(previousTargetStyleable);

    if (previousTargetStyleable)
    {
      previousTargetStyleable->EnsureKeyframeEffectStack().RemoveEffect(*this);
    }
    if (newTargetStyleable)
    {
      (void)newTargetStyleable->EnsureKeyframeEffectStack().AddEffect(*this);
    }
  }

  EnumSet<AnimationImpact>
    KeyframeEffect::Apply(RenderStyle &targetStyle, const style::ResolutionContext &resolutionContext,
                          EndpointInclusiveActiveInterval endpointInclusiveActiveInterval) noexcept
  {
    EnumSet<AnimationImpact> impact;
    if (!_target)
    {
      return impact;
    }

    UpdateBlendingKeyframes(targetStyle, resolutionContext);

    auto computedTiming = GetComputedTiming(UseCachedCurrentTime::Yes, endpointInclusiveActiveInterval);
    if (_phaseAtLastApplication != computedTiming.Phase)
    {
      _phaseAtLastApplication = computedTiming.Phase;
      impact.Add(AnimationImpact::RequiresRecomposite);
    }

    if (!computedTiming.Progress)
    {
      return impact;
    }

    krys_debug_assert(computedTiming.CurrentIteration);
    SetAnimatedPropertiesInStyle(targetStyle, computedTiming);
    return impact;
  }

  bool KeyframeEffect::IsRunningAccelerated() const noexcept
  {
    return _runningAccelerated == RunningAccelerated::Yes;
  }

  bool KeyframeEffect::IsCurrentlyAffectingProperty(css::PropertyId property,
                                                    Accelerated accelerated) const noexcept
  {
    if (accelerated == Accelerated::Yes && !IsRunningAccelerated() && !IsAboutToRunAccelerated())
    {
      return false;
    }

    if (!_blendingKeyframes.Properties().contains(property))
    {
      return false;
    }

    if (_pseudoElementIdentifier && _pseudoElementIdentifier->Type == PseudoElementType::Marker
        && !style::IsValidMarkerStyleProperty(property))
    {
      return false;
    }

    return _phaseAtLastApplication == AnimationEffectPhase::Active;
  }

  bool KeyframeEffect::IsRunningAcceleratedAnimationForProperty(css::PropertyId property) const noexcept
  {
    if (!IsRunningAccelerated())
    {
      return false;
    }

    krys_debug_assert(Document());
    return style::interpolation::IsAccelerated(property, Document()->GetSettings())
           && _blendingKeyframes.Properties().contains(property);
  }

  bool KeyframeEffect::IsRunningAcceleratedTransformRelatedAnimation() const noexcept
  {
    return IsRunningAccelerated()
           && AnimatablePropertiesContainTransformRelatedProperty(_blendingKeyframes.Properties());
  }

  void KeyframeEffect::Invalidate() noexcept
  {
    InvalidateElement(TargetStyleable());
  }

  void KeyframeEffect::ComputeAcceleratedPropertiesState() noexcept
  {
    bool hasSomeAcceleratedProperties = false;
    bool hasSomeUnacceleratedProperties = false;

    if (auto *document = Document())
    {
      const auto &settings = document->GetSettings();
      for (const auto &property : _blendingKeyframes.Properties())
      {
        // If any animated property can be accelerated, then the animation should run accelerated.
        if (style::interpolation::IsAccelerated(property, settings))
        {
          hasSomeAcceleratedProperties = true;
        }
        else
        {
          hasSomeUnacceleratedProperties = true;
        }
        if (hasSomeAcceleratedProperties && hasSomeUnacceleratedProperties)
        {
          break;
        }
      }
    }

    if (!hasSomeAcceleratedProperties)
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::None;
    }
    else if (hasSomeUnacceleratedProperties)
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::Some;
    }
    else
    {
      _acceleratedPropertiesState = AcceleratedPropertiesState::All;
    }
  }

  void KeyframeEffect::ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints() noexcept
  {
    _someKeyframesUseStepsTimingFunction = false;
    _someKeyframesUseLinearTimingFunctionWithPoints = false;

    // If we're dealing with a CSS Animation and it specifies a default steps() or linear() timing function,
    // we need to check that any of the specified keyframes either does not have an explicit timing function
    // or specifies an explicit steps() or linear() timing function.
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
    {
      auto *defaultTimingFunction = cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get();
      auto defaultTimingFunctionIsSteps = Is<StepsTimingFunction>(*defaultTimingFunction);
      auto defaultTimingFunctionIsLinearWithPoints = IsLinearTimingFunctionWithPoints(defaultTimingFunction);
      if (defaultTimingFunctionIsSteps || defaultTimingFunctionIsLinearWithPoints)
      {
        for (const auto &keyframe : _blendingKeyframes)
        {
          auto *timingFunction = keyframe.GetTimingFunction();
          if (defaultTimingFunctionIsSteps && !_someKeyframesUseStepsTimingFunction)
          {
            _someKeyframesUseStepsTimingFunction =
              !timingFunction || Is<StepsTimingFunction>(*timingFunction);
          }
          else if (defaultTimingFunctionIsLinearWithPoints
                   && !_someKeyframesUseLinearTimingFunctionWithPoints)
          {
            _someKeyframesUseLinearTimingFunctionWithPoints =
              !timingFunction || IsLinearTimingFunctionWithPoints(timingFunction);
          }
          if (defaultTimingFunctionIsSteps == _someKeyframesUseStepsTimingFunction
              && defaultTimingFunctionIsLinearWithPoints == _someKeyframesUseLinearTimingFunctionWithPoints)
          {
            break;
          }
        }
        return;
      }
    }

    // For any other type of animation, we just need to check whether any of the keyframes specify an explicit
    // steps() or linear() timing function.
    for (const auto &keyframe : _blendingKeyframes)
    {
      auto *timingFunction = keyframe.GetTimingFunction();
      if (!_someKeyframesUseStepsTimingFunction && timingFunction && Is<StepsTimingFunction>(*timingFunction))
      {
        _someKeyframesUseStepsTimingFunction = true;
      }
      if (!_someKeyframesUseLinearTimingFunctionWithPoints
          && IsLinearTimingFunctionWithPoints(timingFunction))
      {
        _someKeyframesUseLinearTimingFunctionWithPoints = true;
      }
      if (_someKeyframesUseStepsTimingFunction && _someKeyframesUseLinearTimingFunctionWithPoints)
      {
        return;
      }
    }
  }

  void KeyframeEffect::GetAnimatedStyle(UniquePtr<RenderStyle> &animatedStyle) noexcept
  {
    if (!Renderer() || !Animation())
    {
      return;
    }

    auto computedTiming = GetComputedTiming();
    if (!computedTiming.Progress)
    {
      return;
    }

    if (!animatedStyle)
    {
      if (auto *style = TargetStyleable()->LastStyleChangeEventStyle())
      {
        animatedStyle = RenderStyle::ClonePtr(*style);
      }
      else
      {
        animatedStyle = RenderStyle::ClonePtr(Renderer()->Style());
      }
    }

    krys_debug_assert(computedTiming.CurrentIteration);
    SetAnimatedPropertiesInStyle(*animatedStyle, computedTiming);
  }

  void KeyframeEffect::SetAnimatedPropertiesInStyle(RenderStyle &targetStyle,
                                                    const ComputedEffectTiming &computedTiming) const noexcept
  {
    krys_debug_assert(computedTiming.Progress);
    krys_debug_assert(computedTiming.CurrentIteration);

    auto iterationProgress = *computedTiming.Progress;
    auto currentIteration = *computedTiming.CurrentIteration;
    auto before = computedTiming.Before;

    const auto &properties = _blendingKeyframes.Properties();

    // In the case of CSS Transitions we already know that there are only two keyframes, one where offset=0
    // and one where offset=1, and only a single CSS property so we can simply blend based on the style
    // available on those keyframes with the provided iteration progress which already accounts for the
    // transition's timing function.
    if (_animationType == WebAnimationType::CSSTransition)
    {
      krys_debug_assert(properties.size() == 1);
      style::interpolation::Interpolate(*properties.begin(), targetStyle, *_blendingKeyframes[0].Style(),
                                        *_blendingKeyframes[1].Style(), iterationProgress,
                                        _compositeOperation, *this);
      return;
    }

    // 4.4.3. The effect value of a keyframe effect
    // https://drafts.csswg.org/web-animations-1/#the-effect-value-of-a-keyframe-animation-effect
    //
    // The effect value of a single property referenced by a keyframe effect as one of its target properties,
    // for a given iteration progress, current iteration and underlying value is calculated as follows.

    if (_blendingKeyframes.IsEmpty())
    {
      return;
    }

    BlendingKeyframe propertySpecificKeyframeWithZeroOffset(0, RenderStyle::ClonePtr(targetStyle));
    BlendingKeyframe propertySpecificKeyframeWithOneOffset(1, RenderStyle::ClonePtr(targetStyle));

    for (const auto &property : properties)
    {
      auto interval =
        InterpolationKeyframes(property, iterationProgress, propertySpecificKeyframeWithZeroOffset,
                               propertySpecificKeyframeWithOneOffset);
      if (interval.Endpoints.empty())
      {
        continue;
      }

      auto *startBlendingKeyframe = DynamicDowncast<BlendingKeyframe>(interval.Endpoints.front());
      auto *endBlendingKeyframe = DynamicDowncast<BlendingKeyframe>(interval.Endpoints.back());
      if (!startBlendingKeyframe || !endBlendingKeyframe)
      {
        krys_debug_assert(false);
        continue;
      }

      auto startKeyframeStyle = RenderStyle::Clone(*startBlendingKeyframe->Style());
      auto endKeyframeStyle = RenderStyle::Clone(*endBlendingKeyframe->Style());

      KeyframeInterpolation::CompositionCallback composeProperty =
        [&](const KeyframeInterpolation::Keyframe &keyframe, boo::CompositeOperation compositeOperation)
      {
        auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe);
        if (!blendingKeyframe)
        {
          krys_debug_assert(false);
          return;
        }

        if (blendingKeyframe->Offset() == startBlendingKeyframe->Offset())
        {
          style::interpolation::Interpolate(property, startKeyframeStyle, targetStyle,
                                            *blendingKeyframe->Style(), 1, compositeOperation, *this);
        }
        else
        {
          style::interpolation::Interpolate(property, endKeyframeStyle, targetStyle,
                                            *blendingKeyframe->Style(), 1, compositeOperation, *this);
        }
      };

      KeyframeInterpolation::AccumulationCallback accumulateProperty =
        [&](const KeyframeInterpolation::Keyframe &keyframe)
      {
        auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe);
        if (!blendingKeyframe)
        {
          krys_debug_assert(false);
          return;
        }

        if (blendingKeyframe->Offset() == startBlendingKeyframe->Offset())
        {
          style::interpolation::Interpolate(property, startKeyframeStyle, *endBlendingKeyframe->Style(),
                                            startKeyframeStyle, 1, boo::CompositeOperation::Accumulate,
                                            *this);
        }
        else
        {
          style::interpolation::Interpolate(property, endKeyframeStyle, *endBlendingKeyframe->Style(),
                                            endKeyframeStyle, 1, boo::CompositeOperation::Accumulate, *this);
        }
      };

      KeyframeInterpolation::InterpolationCallback interpolateProperty =
        [&](double intervalProgress, double currentIteration,
            boo::IterationCompositeOperation iterationCompositeOperation)
      {
        style::interpolation::Interpolate(property, targetStyle, startKeyframeStyle, endKeyframeStyle,
                                          intervalProgress, boo::CompositeOperation::Replace,
                                          iterationCompositeOperation, currentIteration, *this);
      };

      KeyframeInterpolation::RequiresInterpolationForAccumulativeIterationCallback
        requiresInterpolationForAccumulativeIterationCallback = [&]()
      {
        return style::interpolation::RequiresInterpolationForAccumulativeIteration(
          property, startKeyframeStyle, endKeyframeStyle, *this);
      };

      InterpolateKeyframes(property, interval, iterationProgress, currentIteration, IterationDuration(),
                           before, composeProperty, accumulateProperty, interpolateProperty,
                           requiresInterpolationForAccumulativeIterationCallback);
    }

    // In case one of the animated properties has its value set to "inherit" in one of the keyframes, let's
    // mark the resulting animated style as having an explicitly inherited property such that a future style
    // update accounts for this in a future call to TreeResolver::DetermineResolutionType().
    if (_blendingKeyframes.HasExplicitlyInheritedKeyframeProperty())
    {
      targetStyle.SetHasExplicitlyInheritedProperties();
    }
  }

  const TimingFunction *
    KeyframeEffect::TimingFunctionForBlendingKeyframe(const BlendingKeyframe &keyframe) const noexcept
  {
    if (auto *styleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(Animation()))
    {
      // If we're dealing with a CSS Animation, the timing function is specified either on the keyframe
      // itself.
      if (styleOriginatedAnimation->IsCSSAnimation())
      {
        if (auto *timingFunction = keyframe.GetTimingFunction())
        {
          return timingFunction;
        }
      }

      // Failing that, or for a CSS Transition, the timing function is inherited from the backing Animation
      // object.
      return styleOriginatedAnimation->BackingAnimationTimingFunction();
    }

    return keyframe.GetTimingFunction();
  }

  const TimingFunction *KeyframeEffect::TimingFunctionForKeyframeAtIndex(size_t index) const noexcept
  {
    if (!_parsedKeyframes.empty())
    {
      if (index >= _parsedKeyframes.size())
      {
        return nullptr;
      }
      return _parsedKeyframes[index].TimingFunction.get();
    }

    if (index >= _blendingKeyframes.Size())
    {
      return nullptr;
    }

    return TimingFunctionForBlendingKeyframe(_blendingKeyframes[index]);
  }

  bool KeyframeEffect::CanBeAccelerated() const noexcept
  {
    if (!Animation() || !Animation()->Timeline())
    {
      return false;
    }

    if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
    {
      return false;
    }

    if (_hasAcceleratedPropertyOverriddenByCascadeProperty)
    {
      return false;
    }

    if (_hasReferenceFilter)
    {
      return false;
    }

    if (_animatesSizeAndSizeDependentTransform)
    {
      return false;
    }

    if (_blendingKeyframes.HasDiscreteTransformInterval())
    {
      return false;
    }

    // FIXME: Quirks that prevent acceleration (shouldPreventKeyframeEffectAcceleration), once there are
    // quirks.

    if (_isAssociatedWithProgressBasedTimeline)
    {
      return false;
    }

    if (_someKeyframesUseStepsTimingFunction
        || (TimingFunction() && Is<StepsTimingFunction>(*TimingFunction())))
    {
      return false;
    }

    if (_someKeyframesUseLinearTimingFunctionWithPoints || IsLinearTimingFunctionWithPoints(TimingFunction()))
    {
      return false;
    }

    if (_compositeOperation != boo::CompositeOperation::Replace)
    {
      return false;
    }

    if (_hasKeyframeComposingAcceleratedProperty)
    {
      return false;
    }

    return true;
  }

  bool KeyframeEffect::AnimatesMotionPath() const noexcept
  {
    return AnimatesProperty(css::PropertyId::OffsetAnchor)
           || AnimatesProperty(css::PropertyId::OffsetDistance)
           || AnimatesProperty(css::PropertyId::OffsetPath)
           || AnimatesProperty(css::PropertyId::OffsetPosition)
           || AnimatesProperty(css::PropertyId::OffsetRotate);
  }

  bool KeyframeEffect::PreventsAcceleration() const noexcept
  {
    // We cannot run accelerated transform animations if a motion path is applied
    // to an element, either through the underlying style, or through a keyframe.
    if (auto target = TargetStyleable())
    {
      if (const auto *lastStyleChangeEventStyle = target->LastStyleChangeEventStyle())
      {
        if (lastStyleChangeEventStyle->HasOffsetPath())
        {
          return true;
        }
      }
    }

    if (AnimatesMotionPath())
    {
      return true;
    }

    if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
    {
      return false;
    }

    return !CanBeAccelerated() || _runningAccelerated == RunningAccelerated::Failed;
  }

  void KeyframeEffect::UpdateAcceleratedActions() noexcept
  {
    auto *renderer = Renderer();
    if (!renderer || !renderer->IsComposited())
    {
      return;
    }

    if (!CanBeAccelerated())
    {
      return;
    }

    auto computedTiming = GetComputedTiming();

    // If we're not already running accelerated, the only thing we're interested in is whether we need to
    // start the animation which we need to do once we're in the active phase. Otherwise, there's no change in
    // accelerated state to consider.
    bool isActive = computedTiming.Phase == AnimationEffectPhase::Active;
    if (_runningAccelerated == RunningAccelerated::NotStarted)
    {
      if (isActive && Animation()->GetPlayState() == WebAnimation::PlayState::Running)
      {
        AddPendingAcceleratedAction(AcceleratedAction::Play);
      }
      return;
    }

    // If we're no longer active, we need to remove the accelerated animation.
    if (!isActive)
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
      return;
    }

    auto playState = Animation()->GetPlayState();
    // The only thing left to consider is whether we need to pause or resume the animation following a change
    // of play-state.
    if (playState == WebAnimation::PlayState::Paused)
    {
      if (_lastRecordedAcceleratedAction != AcceleratedAction::Pause)
      {
        if (_lastRecordedAcceleratedAction == AcceleratedAction::Stop)
        {
          AddPendingAcceleratedAction(AcceleratedAction::Play);
        }
        AddPendingAcceleratedAction(AcceleratedAction::Pause);
      }
    }
    else if (playState == WebAnimation::PlayState::Running && isActive)
    {
      if (_lastRecordedAcceleratedAction != AcceleratedAction::Play)
      {
        AddPendingAcceleratedAction(AcceleratedAction::Play);
      }
    }
  }

  void KeyframeEffect::AddPendingAcceleratedAction(AcceleratedAction action) noexcept
  {
    if (!Animation())
    {
      return;
    }

    if (_runningAccelerated == RunningAccelerated::Prevented
        || _runningAccelerated == RunningAccelerated::Failed)
    {
      return;
    }

    if (action == _lastRecordedAcceleratedAction)
    {
      return;
    }

    if (action == AcceleratedAction::Stop)
    {
      _pendingAcceleratedActions.clear();
    }
    _pendingAcceleratedActions.push_back(action);
    if (action != AcceleratedAction::UpdateProperties && action != AcceleratedAction::TransformChange)
    {
      _lastRecordedAcceleratedAction = action;
    }
    Animation()->AcceleratedStateDidChange();
  }

  void KeyframeEffect::AnimationDidTick() noexcept
  {
    Invalidate();
    UpdateAcceleratedActions();
  }

  void KeyframeEffect::AnimationDidChangeTimingProperties() noexcept
  {
    ComputeSomeKeyframesUseStepsOrLinearTimingFunctionWithPoints();
    UpdateAcceleratedAnimationIfNecessary();
    Invalidate();
  }

  void KeyframeEffect::UpdateAcceleratedAnimationIfNecessary() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      if (CanBeAccelerated())
      {
        AddPendingAcceleratedAction(AcceleratedAction::UpdateProperties);
      }
      else
      {
        AbilityToBeAcceleratedDidChange();
        AddPendingAcceleratedAction(AcceleratedAction::Stop);
      }
    }
    else if (CanBeAccelerated())
    {
      _runningAccelerated = RunningAccelerated::NotStarted;
    }
  }

  void KeyframeEffect::AnimationPlaybackRateDidChange() noexcept
  {
    AnimationEffect::AnimationPlaybackRateDidChange();

    UpdateAcceleratedAnimationIfNecessary();
  }

  void KeyframeEffect::TransformRelatedPropertyDidChange() noexcept
  {
    krys_debug_assert(IsRunningAcceleratedTransformRelatedAnimation());
    auto hasTransformRelatedPropertyWithImplicitKeyframe =
      AnimatablePropertiesContainTransformRelatedProperty(_acceleratedPropertiesWithImplicitKeyframe);
    AddPendingAcceleratedAction(hasTransformRelatedPropertyWithImplicitKeyframe
                                  ? AcceleratedAction::UpdateProperties
                                  : AcceleratedAction::TransformChange);
  }

  Maybe<KeyframeEffect::RecomputationReason>
    KeyframeEffect::RecomputeKeyframesIfNecessary(const RenderStyle *previousUnanimatedStyle,
                                                  const RenderStyle &unanimatedStyle,
                                                  const style::ResolutionContext &resolutionContext) noexcept
  {
    if (_animationType == WebAnimationType::CSSTransition)
    {
      return null;
    }

    auto fontSizeChanged = [&]()
    {
      return previousUnanimatedStyle
             && previousUnanimatedStyle->ComputedFontSize() != unanimatedStyle.ComputedFontSize();
    };

    auto fontWeightChanged = [&]()
    {
      return _blendingKeyframes.UsesRelativeFontWeight() && previousUnanimatedStyle
             && previousUnanimatedStyle->GetFontWeight() != unanimatedStyle.GetFontWeight();
    };

    auto cssVariableChanged = [&]()
    {
      if (previousUnanimatedStyle && _blendingKeyframes.HasCSSVariableReferences())
      {
        if (!previousUnanimatedStyle->CustomPropertiesEqual(unanimatedStyle))
        {
          return true;
        }
      }
      return false;
    };

    auto hasPropertyExplicitlySetToInherit = [&]()
    {
      return !_blendingKeyframes.PropertiesSetToInherit().empty();
    };

    auto propertySetToCurrentColorChanged = [&]()
    {
      // If the "color" property itself is set to "currentcolor" on a keyframe, we always recompute keyframes.
      if (_blendingKeyframes.HasColorSetToCurrentColor())
      {
        return true;
      }
      // For all other color-related properties set to "currentcolor" on a keyframe, it's sufficient to check
      // whether the value "color" resolves to has changed since the last style resolution.
      return _blendingKeyframes.HasPropertySetToCurrentColor() && previousUnanimatedStyle
             && previousUnanimatedStyle->GetColor() != unanimatedStyle.GetColor();
    };

    auto logicalPropertyChanged = [&]()
    {
      if (!previousUnanimatedStyle)
      {
        return false;
      }
      if (previousUnanimatedStyle->GetWritingMode() == unanimatedStyle.GetWritingMode())
      {
        return false;
      }
      if (!_blendingKeyframes.IsEmpty())
      {
        return _blendingKeyframes.ContainsDirectionAwareProperty();
      }

      for (const auto &keyframe : _parsedKeyframes)
      {
        for (const auto &[property, value] : keyframe.StyleStrings)
        {
          if (css::Property::IsDirectionAwareProperty(property))
          {
            return true;
          }
        }
      }

      return false;
    }();

    auto usesAnchorFunctions = _blendingKeyframes.UsesAnchorFunctions();

    if (logicalPropertyChanged || fontSizeChanged() || fontWeightChanged() || cssVariableChanged()
        || hasPropertyExplicitlySetToInherit() || propertySetToCurrentColorChanged() || usesAnchorFunctions)
    {
      switch (_animationType)
      {
        case WebAnimationType::CSSTransition: krys_debug_assert(false); break;
        case WebAnimationType::CSSAnimation:
          ComputeCSSAnimationBlendingKeyframes(unanimatedStyle, resolutionContext);
          break;
        case WebAnimationType::WebAnimation: ClearBlendingKeyframes(); break;
      }
      return logicalPropertyChanged ? RecomputationReason::LogicalPropertyChange : RecomputationReason::Other;
    }

    return null;
  }

  void KeyframeEffect::WasAddedToEffectStack() noexcept
  {
    _inTargetEffectStack = true;
    Invalidate();
  }

  void KeyframeEffect::AnimationWasCanceled() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
    }
  }

  void KeyframeEffect::WasRemovedFromEffectStack() noexcept
  {
    _inTargetEffectStack = false;

    if (!CanBeAccelerated())
    {
      return;
    }

    // If the effect was running accelerated, we need to mark it for removal straight away
    // since it will not be invalidated by a future call to
    // KeyframeEffectStack::ApplyPendingAcceleratedActions().
    krys_debug_assert(Animation());
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      Ref animation = ShareRef(*Animation());
      bool isFinishingNaturally = animation->HasPendingFinishNotification()
                                  || animation->GetPlayState() == WebAnimation::PlayState::Finished;

      _pendingAcceleratedActions.clear();
      _pendingAcceleratedActions.push_back(AcceleratedAction::Stop);

      if (isFinishingNaturally)
      {
        // Don't immediately stop animations that are finishing naturally - delay cleanup via microtask
        // to allow the finished promise callback to observe the final animation state (e.g., layer tree).
        // Only immediately stop animations removed mid-flight.
        html::EventLoop::Current().QueueMicrotask([protectedThis = Ref<KeyframeEffect>(ShareRef(*this))]
                                                  { protectedThis->ApplyPendingAcceleratedActions(); });
      }
      else
      {
        ApplyPendingAcceleratedActions();
      }
    }
  }

  void KeyframeEffect::WillChangeRenderer() noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(AcceleratedAction::Stop);
    }
  }

  void KeyframeEffect::AnimationSuspensionStateDidChange(bool animationIsSuspended) noexcept
  {
    if (IsRunningAccelerated() || IsAboutToRunAccelerated())
    {
      AddPendingAcceleratedAction(animationIsSuspended ? AcceleratedAction::Pause : AcceleratedAction::Play);
    }
  }

  void KeyframeEffect::ApplyPendingAcceleratedActionsOrUpdateTimingProperties() noexcept
  {
    if (_pendingAcceleratedActions.empty())
    {
      if (!CanBeAccelerated() || GetComputedTiming().Phase != AnimationEffectPhase::Active)
      {
        return;
      }
      _pendingAcceleratedActions.push_back(AcceleratedAction::UpdateProperties);
      _lastRecordedAcceleratedAction = AcceleratedAction::Play;
      ApplyPendingAcceleratedActions();
      _pendingAcceleratedActions.clear();
    }
    else
    {
      ApplyPendingAcceleratedActions();
    }
  }

  void KeyframeEffect::ApplyPendingAcceleratedActions() noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);

    // Once an accelerated animation has been committed, we no longer want to force a layout.
    // This should have been performed by a call to ForceLayoutIfNeeded() prior to applying
    // pending accelerated actions.
    _needsForcedLayout = false;

    if (_pendingAcceleratedActions.empty())
    {
      return;
    }

    auto *renderer = Renderer();
    if (!renderer || !renderer->IsComposited())
    {
      // The renderer may no longer be composited because the accelerated animation ended before we had a
      // chance to update it, in which case if we asked for the animation to stop, we can discard the current
      // set of accelerated actions.
      if (_lastRecordedAcceleratedAction == AcceleratedAction::Stop)
      {
        _pendingAcceleratedActions.clear();
        _runningAccelerated = RunningAccelerated::NotStarted;
      }
      return;
    }

    auto pendingAcceleratedActions = _pendingAcceleratedActions;
    _pendingAcceleratedActions.clear();

    auto timeOffset = [&]
    {
      // To simplify the code we use a default of 0s for an unresolved current time since for a Stop action
      // that is acceptable.
      auto currentTime = Animation()->CurrentTime().value_or(WebAnimationTime(Seconds {0}));
      auto cssNumberishTimeOffset = currentTime - Delay();
      krys_debug_assert(cssNumberishTimeOffset.Time());
      return cssNumberishTimeOffset.Time()->count();
    };

    auto startAnimation = [&]() -> RunningAccelerated
    {
      if (IsRunningAccelerated())
      {
        renderer->AnimationFinished(_blendingKeyframes);
      }

      krys_debug_assert(_target);
      auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier);
      krys_debug_assert(effectStack);

      if ((_blendingKeyframes.HasWidthDependentTransform()
           && effectStack->ContainsProperty(css::PropertyId::Width))
          || (_blendingKeyframes.HasHeightDependentTransform()
              && effectStack->ContainsProperty(css::PropertyId::Height)))
      {
        return RunningAccelerated::Prevented;
      }

      if (!effectStack->AllowsAcceleration())
      {
        return RunningAccelerated::Prevented;
      }

      if (!_hasImplicitKeyframeForAcceleratedProperty)
      {
        return renderer->StartAnimation(timeOffset(), *BackingAnimationForCompositedRenderer(),
                                        _blendingKeyframes)
                 ? RunningAccelerated::Yes
                 : RunningAccelerated::Failed;
      }

      // We need to resolve all animations up to this point to ensure any forward-filling
      // effect is accounted for when computing the "from" value for the accelerated animation.
      auto underlyingStyle = [&]()
      {
        if (const auto *lastStyleChangeEventStyle =
              _target->LastStyleChangeEventStyle(_pseudoElementIdentifier))
        {
          return RenderStyle::ClonePtr(*lastStyleChangeEventStyle);
        }
        return RenderStyle::ClonePtr(renderer->Style());
      }();

      for (const auto &effect : effectStack->SortedEffects())
      {
        if (this == effect.get())
        {
          break;
        }
        auto computedTiming = effect->GetComputedTiming();
        if (computedTiming.Progress)
        {
          effect->SetAnimatedPropertiesInStyle(*underlyingStyle, computedTiming);
        }
      }

      BlendingKeyframes explicitKeyframes(_blendingKeyframes.Identifier());
      explicitKeyframes.CopyKeyframes(_blendingKeyframes);
      explicitKeyframes.FillImplicitKeyframes(*this, *underlyingStyle);
      return renderer->StartAnimation(timeOffset(), *BackingAnimationForCompositedRenderer(),
                                      explicitKeyframes)
               ? RunningAccelerated::Yes
               : RunningAccelerated::Failed;
    };

    for (const auto &action : pendingAcceleratedActions)
    {
      switch (action)
      {
        case AcceleratedAction::Play:
          _runningAccelerated = startAnimation();
          if (!IsRunningAccelerated())
          {
            _lastRecordedAcceleratedAction = AcceleratedAction::Stop;
            return;
          }
          break;
        case AcceleratedAction::Pause: renderer->AnimationPaused(timeOffset(), _blendingKeyframes); break;
        case AcceleratedAction::UpdateProperties:
          _runningAccelerated = startAnimation();
          if (Animation()->GetPlayState() == WebAnimation::PlayState::Paused)
          {
            renderer->AnimationPaused(timeOffset(), _blendingKeyframes);
          }
          break;
        case AcceleratedAction::Stop:
          krys_debug_assert(Document());
          renderer->AnimationFinished(_blendingKeyframes);
          if (!Document()->RenderTreeBeingDestroyed())
          {
            _target->InvalidateStyleAndLayerComposition();
          }
          _runningAccelerated =
            CanBeAccelerated() ? RunningAccelerated::NotStarted : RunningAccelerated::Prevented;
          break;
        case AcceleratedAction::TransformChange: renderer->TransformRelatedPropertyDidChange(); break;
      }
    }
  }

  Ref<const GraphicsLayerAnimation> KeyframeEffect::BackingAnimationForCompositedRenderer() noexcept
  {
    Ref effectAnimation = ShareRef(*Animation());

    // FIXME: The iterationStart and endDelay AnimationEffectTiming properties do not have
    // corresponding GraphicsLayerAnimation properties.
    auto animation = GraphicsLayerAnimation::Create();
    animation->SetDuration(IterationDuration().Time()->count());
    animation->SetDelay(Delay().Time()->count());
    animation->SetIterationCount(Iterations());
    animation->SetTimingFunction(TimingFunction()->Clone());
    animation->SetPlaybackRate(effectAnimation->PlaybackRate());
    animation->SetCompositeOperation(_compositeOperation);

    switch (Fill())
    {
      case FillMode::None:
      case FillMode::Auto:      animation->SetFillMode(GraphicsLayerAnimation::FillMode::None); break;
      case FillMode::Backwards: animation->SetFillMode(GraphicsLayerAnimation::FillMode::Backwards); break;
      case FillMode::Forwards:  animation->SetFillMode(GraphicsLayerAnimation::FillMode::Forwards); break;
      case FillMode::Both:      animation->SetFillMode(GraphicsLayerAnimation::FillMode::Both); break;
    }

    switch (Direction())
    {
      case PlaybackDirection::Normal:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Normal);
        break;
      case PlaybackDirection::Alternate:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Alternate);
        break;
      case PlaybackDirection::Reverse:
        animation->SetDirection(GraphicsLayerAnimation::Direction::Reverse);
        break;
      case PlaybackDirection::AlternateReverse:
        animation->SetDirection(GraphicsLayerAnimation::Direction::AlternateReverse);
        break;
    }

    // In the case of CSS Animations, we must set the default timing function for keyframes to match
    // the current value set for animation-timing-function on the target element which affects only
    // keyframes and not the animation-wide timing.
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(effectAnimation.get()))
    {
      animation->SetDefaultTimingFunctionForKeyframes(
        ShareRefPtr(cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get()));
    }

    return animation;
  }

  dom::Document *KeyframeEffect::Document() const noexcept
  {
    if (_document)
    {
      return _document.get();
    }
    return _target ? &_target->NodeDocument() : nullptr;
  }

  RenderElement *KeyframeEffect::Renderer() const noexcept
  {
    if (auto target = TargetStyleable())
    {
      return target->Renderer();
    }
    return nullptr;
  }

  const RenderStyle &KeyframeEffect::CurrentStyle() const noexcept
  {
    if (auto *renderer = Renderer())
    {
      return renderer->Style();
    }
    return RenderStyle::DefaultStyleSingleton();
  }

  bool KeyframeEffect::ComputeExtentOfTransformAnimation(LayoutRect &bounds) const noexcept
  {
    krys_debug_assert(AnimatablePropertiesContainTransformRelatedProperty(_blendingKeyframes.Properties()));

    auto *box = DynamicDowncast<RenderBox>(Renderer());
    if (!box)
    {
      return true; // Non-boxes don't get transformed;
    }

    // FIXME: add extent computation for animated CSS Motion Path properties.
    if (AnimatesMotionPath())
    {
      return true;
    }

    auto rendererBox = SnapRectToDevicePixels(box->BorderBoxRect(), box->GetDocument().DeviceScaleFactor());
    // FIXME: The motion path data of the box (TransformOperationData's renderer constructor), once there are
    // motion paths.
    TransformOperationData transformOperationData(rendererBox);
    LayoutRect cumulativeBounds;

    const auto *unanimatedStyle = [&]() -> const RenderStyle *
    {
      if (auto target = TargetStyleable())
      {
        if (const auto *lastStyleChangeEventStyle = target->LastStyleChangeEventStyle())
        {
          return lastStyleChangeEventStyle;
        }
      }
      return &box->Style();
    }();

    auto addStyleToCumulativeBounds = [&](const RenderStyle &style)
    {
      auto keyframeBounds = bounds;

      auto transform = style::TransformResolver::ComputeTransform(style, transformOperationData);
      if (!transform.IsAffine())
      {
        return false;
      }

      TransformationMatrix::Decomposed2Type fromDecomp;
      if (!transform.Decompose2(fromDecomp))
      {
        return false;
      }

      auto hasRotation = [&]
      {
        if (fromDecomp.Angle || style.GetRotate().AffectedByTransformOrigin())
        {
          return true;
        }
        for (const auto &operation : style.GetTransform())
        {
          if (operation->GetType() == style::TransformFunctionType::Rotate)
          {
            return true;
          }
        }
        return false;
      }();

      // FIXME: it may be better to find a way to replace all rotation angles
      // in transforms with a 90deg angle.
      if (hasRotation)
      {
        keyframeBounds = LayoutRect(BoundsOfRotatingRect(FloatRect(keyframeBounds)));
      }

      cumulativeBounds.Unite(transform.MapRect(keyframeBounds));
      return true;
    };

    for (const auto &keyframe : _blendingKeyframes)
    {
      if (!AnimatablePropertiesContainTransformRelatedProperty(keyframe.Properties()))
      {
        continue;
      }

      auto blendedStyleForKeyframe = RenderStyle::ClonePtr(*unanimatedStyle);

      ComputedEffectTiming computedTiming;
      computedTiming.CurrentIteration = 0;
      computedTiming.Progress = keyframe.Offset();

      SetAnimatedPropertiesInStyle(*blendedStyleForKeyframe, computedTiming);

      if (!addStyleToCumulativeBounds(*blendedStyleForKeyframe))
      {
        return false;
      }
    }

    if (_blendingKeyframes.HasImplicitKeyframes())
    {
      if (!addStyleToCumulativeBounds(*unanimatedStyle))
      {
        return false;
      }
    }

    bounds = cumulativeBounds;
    return true;
  }

  bool KeyframeEffect::RequiresPseudoElement() const noexcept
  {
    return _animationType == WebAnimationType::WebAnimation && TargetsPseudoElement();
  }

  Maybe<double> KeyframeEffect::ProgressUntilNextStep(double iterationProgress) const noexcept
  {
    krys_debug_assert(iterationProgress >= 0 && iterationProgress <= 1);

    if (auto progress = AnimationEffect::ProgressUntilNextStep(iterationProgress))
    {
      return progress;
    }

    if (!TimingFunction() || !Is<LinearTimingFunction>(*TimingFunction())
        || !_someKeyframesUseStepsTimingFunction)
    {
      return null;
    }

    if (_blendingKeyframes.IsEmpty())
    {
      return null;
    }

    auto progressUntilNextStepInInterval =
      [iterationProgress](double intervalStartProgress, double intervalEndProgress,
                          const boo::TimingFunction *timingFunction) -> Maybe<double>
    {
      auto *stepsTimingFunction = DynamicDowncast<StepsTimingFunction>(timingFunction);
      if (!stepsTimingFunction)
      {
        return null;
      }

      auto numberOfSteps = stepsTimingFunction->NumberOfSteps();
      auto intervalProgress = intervalEndProgress - intervalStartProgress;
      auto iterationProgressMappedToCurrentInterval =
        (iterationProgress - intervalStartProgress) / intervalProgress;
      auto nextStepProgress =
        std::ceil(iterationProgressMappedToCurrentInterval * numberOfSteps) / numberOfSteps;
      return (nextStepProgress - iterationProgressMappedToCurrentInterval) * intervalProgress;
    };

    for (size_t i = 0; i < _blendingKeyframes.Size(); ++i)
    {
      auto intervalEndProgress = _blendingKeyframes[i].Offset();
      // We can stop once we find a keyframe for which the progress is more than the provided iteration
      // progress.
      if (intervalEndProgress <= iterationProgress)
      {
        continue;
      }

      // In case we're on the first keyframe, then this means we are dealing with an implicit 0% keyframe.
      // This will be a linear timing function unless we're dealing with a CSS Animation which might have the
      // default timing function for its keyframes defined on its backing Animation object.
      if (!i)
      {
        if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
        {
          return progressUntilNextStepInInterval(
            0, intervalEndProgress, cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get());
        }
        return null;
      }

      return progressUntilNextStepInInterval(_blendingKeyframes[i - 1].Offset(), intervalEndProgress,
                                             TimingFunctionForKeyframeAtIndex(i - 1));
    }

    // If we end up here, then this means we are dealing with an implicit 100% keyframe. This will be a linear
    // timing function unless we're dealing with a CSS Animation which might have the default timing function
    // for its keyframes defined on its backing Animation object.
    const auto &lastExplicitKeyframe = _blendingKeyframes[_blendingKeyframes.Size() - 1];
    if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(Animation()))
    {
      return progressUntilNextStepInInterval(
        lastExplicitKeyframe.Offset(), 1,
        cssAnimation->BackingStyleAnimation().GetTimingFunction().Value.get());
    }

    // In any other case, we are not dealing with an interval with a steps() timing function.
    return null;
  }

  bool KeyframeEffect::TicksContinuouslyWhileActive() const noexcept
  {
    auto doesNotAffectStyles = _blendingKeyframes.IsEmpty() || _blendingKeyframes.Properties().empty();
    if (doesNotAffectStyles)
    {
      return false;
    }

    auto targetHasDisplayContents = [&]()
    {
      return _target && !_pseudoElementIdentifier && _target->HasDisplayContents();
    };

    if (!Renderer() && !_blendingKeyframes.Properties().contains(css::PropertyId::Display)
        && !targetHasDisplayContents())
    {
      return false;
    }

    if (IsCompletelyAccelerated() && IsRunningAccelerated())
    {
      return false;
    }

    return true;
  }

  Seconds KeyframeEffect::TimeToNextTick(const BasicEffectTiming &timing) noexcept
  {
    // CSS Animations need to trigger "animationiteration" events even if there is no need to update styles
    // while animating, so if we're dealing with one we must wait until the next iteration.
    // FIXME: Only when the document has `animationiteration` listeners (hasListenerType), once documents
    // track their listener types.
    if (timing.Phase == AnimationEffectPhase::Active && Animation() && Is<CSSAnimation>(*Animation())
        && !TicksContinuouslyWhileActive())
    {
      if (auto iterationProgress = GetComputedTiming().SimpleIterationProgress)
      {
        return static_cast<Seconds>(IterationDuration() * (1 - *iterationProgress));
      }
    }
    return AnimationEffect::TimeToNextTick(timing);
  }

  void KeyframeEffect::SetIterationComposite(
    boo::IterationCompositeOperation iterationCompositeOperation) noexcept
  {
    if (_iterationCompositeOperation == iterationCompositeOperation)
    {
      return;
    }

    _iterationCompositeOperation = iterationCompositeOperation;
    Invalidate();
  }

  boo::CompositeOperation KeyframeEffect::BindingsComposite() const noexcept
  {
    if (RefPtr styleOriginatedAnimation = ShareRefPtr(DynamicDowncast<StyleOriginatedAnimation>(Animation())))
    {
      styleOriginatedAnimation->FlushPendingStyleChanges();
    }
    return Composite();
  }

  void KeyframeEffect::SetBindingsComposite(boo::CompositeOperation compositeOperation) noexcept
  {
    SetComposite(compositeOperation);
    if (RefPtr cssAnimation = ShareRefPtr(DynamicDowncast<CSSAnimation>(Animation())))
    {
      cssAnimation->EffectCompositeOperationWasSetUsingBindings();
    }
  }

  void KeyframeEffect::SetComposite(boo::CompositeOperation compositeOperation) noexcept
  {
    if (_compositeOperation == compositeOperation)
    {
      return;
    }

    CanBeAcceleratedMutationScope mutationScope(this);
    _compositeOperation = compositeOperation;
    Invalidate();
  }

  bool KeyframeEffect::PreventsAnimationReadiness() const noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#ready
    // An animation cannot be ready if it's associated with a document that does not have a browsing context
    // since this will prevent the first frame of the animmation from being rendered.
    return Document() && !Document()->Frame();
  }

  const KeyframeInterpolation::Keyframe &KeyframeEffect::KeyframeAtIndex(size_t index) const noexcept
  {
    krys_debug_assert(index < _blendingKeyframes.Size());
    return _blendingKeyframes[index];
  }

  const TimingFunction *
    KeyframeEffect::TimingFunctionForKeyframe(const KeyframeInterpolation::Keyframe &keyframe) const noexcept
  {
    if (auto *blendingKeyframe = DynamicDowncast<BlendingKeyframe>(&keyframe))
    {
      return TimingFunctionForBlendingKeyframe(*blendingKeyframe);
    }

    krys_debug_assert(false);
    return nullptr;
  }

  void KeyframeEffect::ComputeHasImplicitKeyframeForAcceleratedProperty() noexcept
  {
    _hasImplicitKeyframeForAcceleratedProperty = [&]()
    {
      if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
      {
        return false;
      }

      krys_debug_assert(Document());
      const auto &settings = Document()->GetSettings();

      if (_blendingKeyframes.IsEmpty())
      {
        // We may not have computed keyframes yet, so we should check our parsed keyframes in the same way we
        // checked computed keyframes.
        for (const auto &keyframe : _parsedKeyframes)
        {
          // We keep three property lists, one which contains all properties seen across keyframes which will
          // be filtered eventually to only contain implicit properties, one containing properties seen on the
          // 0% keyframe and one containing properties seen on the 100% keyframe.
          HashSet<css::PropertyId> implicitProperties;
          HashSet<css::PropertyId> explicitZeroProperties;
          HashSet<css::PropertyId> explicitOneProperties;
          for (const auto &propertyReference : *keyframe.Style)
          {
            auto computedOffset = keyframe.ComputedOffset;
            if (std::isnan(computedOffset))
            {
              continue;
            }
            auto property = propertyReference.Id();
            // All properties may end up being implicit.
            implicitProperties.insert(property);
            if (!computedOffset)
            {
              explicitZeroProperties.insert(property);
            }
            else if (computedOffset == 1)
            {
              explicitOneProperties.insert(property);
            }
          }
          // Let's remove all properties found on the 0% and 100% keyframes from the list of potential
          // implicit properties.
          for (auto explicitProperty : explicitZeroProperties)
          {
            implicitProperties.erase(explicitProperty);
          }
          for (auto explicitProperty : explicitOneProperties)
          {
            implicitProperties.erase(explicitProperty);
          }
          // At this point all properties left in implicitProperties are known to be implicit, so we must
          // check them for any accelerated property.
          for (auto implicitProperty : implicitProperties)
          {
            if (style::interpolation::IsAccelerated(implicitProperty, settings))
            {
              return true;
            }
          }
        }
        return false;
      }

      // We make a list of all animated properties and consider them all
      // implicit until proven otherwise as we iterate through all keyframes.
      auto implicitZeroProperties = _blendingKeyframes.Properties();
      auto implicitOneProperties = _blendingKeyframes.Properties();
      for (const auto &keyframe : _blendingKeyframes)
      {
        // If the keyframe is for 0% or 100%, let's remove all of its properties from
        // our list of implicit properties.
        if (!implicitZeroProperties.empty() && !keyframe.Offset())
        {
          for (const auto &property : keyframe.Properties())
          {
            implicitZeroProperties.erase(property);
          }
        }
        if (!implicitOneProperties.empty() && keyframe.Offset() == 1)
        {
          for (const auto &property : keyframe.Properties())
          {
            implicitOneProperties.erase(property);
          }
        }
      }
      // The only properties left are known to be implicit properties, so we must
      // check them for any accelerated property.
      for (const auto &implicitProperty : implicitZeroProperties)
      {
        if (style::interpolation::IsAccelerated(implicitProperty, settings))
        {
          return true;
        }
      }
      for (const auto &implicitProperty : implicitOneProperties)
      {
        if (style::interpolation::IsAccelerated(implicitProperty, settings))
        {
          return true;
        }
      }
      return false;
    }();
  }

  void KeyframeEffect::ComputeHasKeyframeComposingAcceleratedProperty() noexcept
  {
    _hasKeyframeComposingAcceleratedProperty = [&]()
    {
      if (_acceleratedPropertiesState == AcceleratedPropertiesState::None)
      {
        return false;
      }

      krys_debug_assert(Document());
      const auto &settings = Document()->GetSettings();

      if (!_blendingKeyframes.IsEmpty())
      {
        for (const auto &keyframe : _blendingKeyframes)
        {
          // If we find a keyframe with a composite operation, we check whether one
          // of its properties is accelerated.
          if (auto keyframeComposite = keyframe.CompositeOperation())
          {
            if (*keyframeComposite != boo::CompositeOperation::Replace)
            {
              for (const auto &property : keyframe.Properties())
              {
                if (style::interpolation::IsAccelerated(property, settings))
                {
                  return true;
                }
              }
            }
          }
        }
        return false;
      }

      // We may not have computed keyframes yet, so we should check our parsed keyframes in the same way we
      // checked computed keyframes.
      for (const auto &keyframe : _parsedKeyframes)
      {
        if (keyframe.Composite != CompositeOperationOrAuto::Add
            && keyframe.Composite != CompositeOperationOrAuto::Accumulate)
        {
          continue;
        }
        for (const auto &property : *keyframe.Style)
        {
          if (style::interpolation::IsAccelerated(property.Id(), settings))
          {
            return true;
          }
        }
      }
      return false;
    }();
  }

  void KeyframeEffect::ComputeHasAcceleratedPropertyOverriddenByCascadeProperty() noexcept
  {
    if (!_inTargetEffectStack)
    {
      return;
    }

    krys_debug_assert(_target);
    auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier);
    if (!effectStack)
    {
      return;
    }

    const auto &animatedProperties = AnimatedProperties();
    _hasAcceleratedPropertyOverriddenByCascadeProperty =
      std::ranges::any_of(effectStack->AcceleratedPropertiesOverriddenByCascade(),
                          [&](const auto &property) { return animatedProperties.contains(property); });
  }

  void KeyframeEffect::ComputeHasReferenceFilter() noexcept
  {
    _hasReferenceFilter = [&]()
    {
      if (_blendingKeyframes.IsEmpty())
      {
        return false;
      }

      auto animatesFilterProperty = _blendingKeyframes.ContainsProperty(css::PropertyId::Filter)
                                    || _blendingKeyframes.ContainsProperty(css::PropertyId::BackdropFilter);

      if (!animatesFilterProperty)
      {
        return false;
      }

      auto styleContainsFilter = [](const RenderStyle &style)
      {
        return style.GetFilter().HasReferenceFilter() || style.BackdropFilter().HasReferenceFilter();
      };

      if (auto target = TargetStyleable())
      {
        if (const auto *style = target->LastStyleChangeEventStyle())
        {
          if (_blendingKeyframes.HasImplicitKeyframes() && styleContainsFilter(*style))
          {
            return true;
          }
        }
      }

      for (const auto &keyframe : _blendingKeyframes)
      {
        if (const auto *style = keyframe.Style())
        {
          if (styleContainsFilter(*style))
          {
            return true;
          }
        }
      }

      return false;
    }();
  }

  void KeyframeEffect::ComputeHasSizeDependentTransform() noexcept
  {
    _animatesSizeAndSizeDependentTransform =
      (_blendingKeyframes.HasWidthDependentTransform()
       && _blendingKeyframes.ContainsProperty(css::PropertyId::Width))
      || (_blendingKeyframes.HasHeightDependentTransform()
          && _blendingKeyframes.ContainsProperty(css::PropertyId::Height));

    // FIXME: View transition group pseudo-elements' size and transform animations, once there are view
    // transitions.
  }

  void KeyframeEffect::EffectStackNoLongerPreventsAcceleration() noexcept
  {
    if (_runningAccelerated == RunningAccelerated::Failed)
    {
      return;
    }

    if (_runningAccelerated == RunningAccelerated::Prevented)
    {
      _runningAccelerated = RunningAccelerated::NotStarted;
    }

    UpdateAcceleratedActions();
  }

  void KeyframeEffect::EffectStackNoLongerAllowsAcceleration() noexcept
  {
    AddPendingAcceleratedAction(AcceleratedAction::Stop);
  }

  void KeyframeEffect::EffectStackNoLongerAllowsAccelerationDuringAcceleratedActionApplication() noexcept
  {
    _pendingAcceleratedActions.push_back(AcceleratedAction::Stop);
    _lastRecordedAcceleratedAction = AcceleratedAction::Stop;
    ApplyPendingAcceleratedActions();
    _pendingAcceleratedActions.clear();
  }

  void KeyframeEffect::AbilityToBeAcceleratedDidChange() noexcept
  {
    if (!_inTargetEffectStack)
    {
      return;
    }

    krys_debug_assert(_target);
    if (auto *effectStack = _target->GetKeyframeEffectStack(_pseudoElementIdentifier))
    {
      effectStack->EffectAbilityToBeAcceleratedDidChange(*this);
    }
  }

  void KeyframeEffect::AcceleratedPropertiesOverriddenByCascadeDidChange() noexcept
  {
    CanBeAcceleratedMutationScope mutationScope(this);
    ComputeHasAcceleratedPropertyOverriddenByCascadeProperty();
  }

  KeyframeEffect::CanBeAcceleratedMutationScope::CanBeAcceleratedMutationScope(
    KeyframeEffect *effect) noexcept
      : _effect(effect)
  {
    krys_debug_assert(effect);
    _couldOriginallyPreventAcceleration = effect->PreventsAcceleration();
  }

  KeyframeEffect::CanBeAcceleratedMutationScope::~CanBeAcceleratedMutationScope() noexcept
  {
    if (!_effect)
    {
      return;
    }

    if (_couldOriginallyPreventAcceleration != _effect->PreventsAcceleration())
    {
      _effect->AbilityToBeAcceleratedDidChange();
    }
  }

  void KeyframeEffect::LastStyleChangeEventStyleDidChange(const RenderStyle *previousStyle,
                                                          const RenderStyle *currentStyle) noexcept
  {
    auto hasMotionPath = [](const RenderStyle *style)
    {
      return style && style->HasOffsetPath();
    };

    if (hasMotionPath(previousStyle) != hasMotionPath(currentStyle))
    {
      AbilityToBeAcceleratedDidChange();
    }
  }

  bool KeyframeEffect::IsPropertyAdditiveOrCumulative(KeyframeInterpolation::Property property) const noexcept
  {
    if (const auto *animatableCSSProperty = std::get_if<AnimatableCSSProperty>(&property))
    {
      return style::interpolation::IsAdditiveOrCumulative(*animatableCSSProperty);
    }

    krys_debug_assert(false);
    return false;
  }
}
