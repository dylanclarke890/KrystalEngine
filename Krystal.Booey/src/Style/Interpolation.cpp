#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+SettersInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/InterpolationClient.hpp"
#include "Krystal.Booey/Style/InterpolationFunctions.hpp"
#include "Krystal.Booey/Style/InterpolationWrapperBase.hpp"
#include "Krystal.Booey/Style/InterpolationWrapperMap.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformList.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::style::interpolation
{
  namespace
  {
    // MARK: - Standard property interpolation support

    void InterpolateStandardProperty(css::PropertyId property, RenderStyle &destination,
                                     const RenderStyle &from, const RenderStyle &to, double progress,
                                     CompositeOperation compositeOperation,
                                     IterationCompositeOperation iterationCompositeOperation,
                                     double currentIteration, const Client &client) noexcept
    {
      krys_debug_assert(property != css::PropertyId::Invalid && property != css::PropertyId::Custom);

      auto *wrapper = WrapperMap::Singleton().GetWrapper(property);
      if (!wrapper)
      {
        return;
      }

      auto isDiscrete = !wrapper->CanInterpolate(from, to, compositeOperation);
      Context context {property,
                       progress,
                       isDiscrete,
                       compositeOperation,
                       iterationCompositeOperation,
                       currentIteration,
                       from.GetColor(),
                       to.GetColor(),
                       client};
      if (!css::Property::AnimationUsesNonNormalizedDiscreteInterpolation(property))
      {
        context.NormalizeProgress();
      }
      wrapper->Interpolate(destination, from, to, context);
    }

    // MARK: - Custom property interpolation support

    KRYS_NODISCARD Maybe<CustomProperty::Value> InterpolateSyntaxValues(const RenderStyle &fromStyle,
                                                                        const RenderStyle &toStyle,
                                                                        const CustomProperty::Value &from,
                                                                        const CustomProperty::Value &to,
                                                                        const Context &context) noexcept
    {
      if (from.index() != to.index())
      {
        return null;
      }

      return std::visit(
        krys::CreateVisitor(
          [&]<Numeric T>(const T &fromNumeric) -> Maybe<CustomProperty::Value>
          { return CustomProperty::Value {style::Blend(fromNumeric, std::get<T>(to), context)}; },
          [&](const Color &fromStyleColor) -> Maybe<CustomProperty::Value>
          {
            const auto &toStyleColor = std::get<Color>(to);
            if (!fromStyleColor.IsCurrentColor() || !toStyleColor.IsCurrentColor())
            {
              return CustomProperty::Value {
                Color {BlendFunc(fromStyle.ColorResolvingCurrentColor(fromStyleColor),
                                 toStyle.ColorResolvingCurrentColor(toStyleColor), context)}};
            }
            return null;
          },
          [&](const TransformFunction &fromTransform) -> Maybe<CustomProperty::Value>
          {
            return CustomProperty::Value {
              style::Blend(fromTransform, std::get<TransformFunction>(to), context)};
          },
          [&](const auto &) -> Maybe<CustomProperty::Value> { return null; }),
        from);
    }

    KRYS_NODISCARD Maybe<CustomProperty::Value>
      FirstValueInSyntaxValueLists(const CustomProperty::ValueList &a,
                                   const CustomProperty::ValueList &b) noexcept
    {
      if (!a.Values.empty())
      {
        return a.Values[0];
      }
      if (!b.Values.empty())
      {
        return b.Values[0];
      }
      return null;
    }

    KRYS_NODISCARD Maybe<CustomProperty::ValueList>
      InterpolateSyntaxValueLists(const RenderStyle &fromStyle, const RenderStyle &toStyle,
                                  const CustomProperty::ValueList &from, const CustomProperty::ValueList &to,
                                  const Context &context) noexcept
    {
      // We should only attempt to interpolate lists containing the same types. Since we know all items in a
      // list are of the same type, it is sufficient to check the first value from each list.
      if (!from.Values.empty() && !to.Values.empty()
          && from.Values.front().index() != to.Values.front().index())
      {
        return null;
      }

      // https://drafts.css-houdini.org/css-properties-values-api-1/#animation-behavior-of-custom-properties
      auto firstValue = FirstValueInSyntaxValueLists(from, to);

      if (!firstValue)
      {
        return null;
      }

      // <transform-function> lists are special in that they don't require matching numbers of items.
      if (std::holds_alternative<TransformFunction>(*firstValue))
      {
        auto transformListFromSyntaxValueList = [](const CustomProperty::ValueList &list)
        {
          return TransformList {TransformList::Container::Map(
            list.Values,
            [&](const auto &syntaxValue)
            {
              krys_debug_assert(std::holds_alternative<TransformFunction>(syntaxValue));
              return std::get<TransformFunction>(syntaxValue);
            })};
        };

        auto fromTransformList = transformListFromSyntaxValueList(from);
        auto toTransformList = transformListFromSyntaxValueList(to);
        auto interpolatedTransformList = style::Blend(fromTransformList, toTransformList, context);

        List<CustomProperty::Value> interpolatedSyntaxValues;
        interpolatedSyntaxValues.reserve(interpolatedTransformList.size());
        for (const auto &transformFunction : interpolatedTransformList)
        {
          interpolatedSyntaxValues.push_back(transformFunction);
        }

        return CustomProperty::ValueList {krys::move(interpolatedSyntaxValues), from.Separator};
      }

      // Other lists must have matching sizes.
      if (from.Values.size() != to.Values.size())
      {
        return null;
      }

      List<CustomProperty::Value> interpolatedSyntaxValues;
      interpolatedSyntaxValues.reserve(from.Values.size());
      for (size_t i = 0; i < from.Values.size(); ++i)
      {
        auto interpolatedSyntaxValue =
          InterpolateSyntaxValues(fromStyle, toStyle, from.Values[i], to.Values[i], context);
        if (!interpolatedSyntaxValue)
        {
          return null;
        }
        interpolatedSyntaxValues.push_back(krys::move(*interpolatedSyntaxValue));
      }

      return CustomProperty::ValueList {krys::move(interpolatedSyntaxValues), from.Separator};
    }

    KRYS_NODISCARD Ref<const CustomProperty> InterpolatedCustomProperty(const RenderStyle &fromStyle,
                                                                        const RenderStyle &toStyle,
                                                                        const CustomProperty &from,
                                                                        const CustomProperty &to,
                                                                        const Context &context) noexcept
    {
      if (std::holds_alternative<CustomProperty::Value>(from.GetValue())
          && std::holds_alternative<CustomProperty::Value>(to.GetValue()))
      {
        const auto &fromSyntaxValue = std::get<CustomProperty::Value>(from.GetValue());
        const auto &toSyntaxValue = std::get<CustomProperty::Value>(to.GetValue());
        if (auto interpolatedSyntaxValue =
              InterpolateSyntaxValues(fromStyle, toStyle, fromSyntaxValue, toSyntaxValue, context))
        {
          return CustomProperty::CreateForValue(from.GetName(), krys::move(*interpolatedSyntaxValue));
        }
      }

      if (std::holds_alternative<CustomProperty::ValueList>(from.GetValue())
          && std::holds_alternative<CustomProperty::ValueList>(to.GetValue()))
      {
        const auto &fromSyntaxValueList = std::get<CustomProperty::ValueList>(from.GetValue());
        const auto &toSyntaxValueList = std::get<CustomProperty::ValueList>(to.GetValue());
        if (auto interpolatedSyntaxValueList = InterpolateSyntaxValueLists(
              fromStyle, toStyle, fromSyntaxValueList, toSyntaxValueList, context))
        {
          return CustomProperty::CreateForValueList(from.GetName(), krys::move(*interpolatedSyntaxValueList));
        }
      }

      // Use a discrete interpolation for all other cases.
      return ShareRef(context.Progress < 0.5 ? from : to);
    }

    KRYS_NODISCARD Pair<const CustomProperty *, const CustomProperty *>
      CustomPropertyValuesForInterpolation(const css::CSSOMStringAtom &customProperty,
                                           const RenderStyle &fromStyle, const RenderStyle &toStyle) noexcept
    {
      return {fromStyle.CustomPropertyValue(customProperty), toStyle.CustomPropertyValue(customProperty)};
    }

    void InterpolateCustomProperty(const css::CSSOMStringAtom &customProperty, RenderStyle &destination,
                                   const RenderStyle &from, const RenderStyle &to, double progress,
                                   CompositeOperation compositeOperation,
                                   IterationCompositeOperation iterationCompositeOperation,
                                   double currentIteration, const Client &client) noexcept
    {
      Context context {customProperty,
                       progress,
                       false,
                       compositeOperation,
                       iterationCompositeOperation,
                       currentIteration,
                       from.GetColor(),
                       to.GetColor(),
                       client};

      auto [fromValue, toValue] = CustomPropertyValuesForInterpolation(customProperty, from, to);
      if (!fromValue || !toValue)
      {
        return;
      }

      bool isInherited =
        client.Document()->StyleScope().GetCustomPropertyRegistry().IsInherited(customProperty);
      destination.SetCustomPropertyValue(InterpolatedCustomProperty(from, to, *fromValue, *toValue, context),
                                         isInherited);
    }

    KRYS_NODISCARD bool SyntaxValuesRequireInterpolationForAccumulativeIteration(
      const CustomProperty::Value &a, const CustomProperty::Value &b, bool isList) noexcept
    {
      return std::visit(krys::CreateVisitor(
                          [&b, isList](const LengthPercentage<> &aLengthPercentage)
                          {
                            krys_debug_assert(std::holds_alternative<LengthPercentage<>>(b));
                            return !isList
                                   && style::RequiresInterpolationForAccumulativeIteration(
                                     aLengthPercentage, std::get<LengthPercentage<>>(b));
                          },
                          [](const TransformFunction &) { return true; }, [](const Color &) { return true; },
                          [](const auto &) { return false; }),
                        a);
    }

    KRYS_NODISCARD bool TypeOfSyntaxValueCanBeInterpolated(const CustomProperty::Value &syntaxValue) noexcept
    {
      return std::visit(krys::CreateVisitor([]<Numeric T>(const T &) { return true; },
                                            [](const ImageWrapper &) { return false; }, [](const Color &)
                                            { return true; }, [](const URL &) { return false; },
                                            [](const css::CustomIdentifier &) { return false; },
                                            [](const css::CSSOMString &) { return false; },
                                            [](const TransformFunction &) { return true; }),
                        syntaxValue);
    }
  }

  // MARK: - Exposed functions

  bool IsAdditiveOrCumulative(const AnimatableCSSProperty &property) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        [](css::PropertyId propertyId)
        { return !css::Property::AnimationUsesNonAdditiveOrCumulativeInterpolation(propertyId); },
        [](const css::CSSOMStringAtom &) { return true; }),
      property);
  }

  bool IsAccelerated(const AnimatableCSSProperty &property, const Settings &settings) noexcept
  {
    return std::visit(
      krys::CreateVisitor([&](css::PropertyId propertyId)
                          { return css::Property::AnimationIsAccelerated(propertyId, settings); },
                          [](const css::CSSOMStringAtom &) { return false; }),
      property);
  }

  bool CanInterpolate(const AnimatableCSSProperty &property) noexcept
  {
    return std::visit(krys::CreateVisitor(
                        [](css::PropertyId propertyId)
                        {
                          return propertyId == css::PropertyId::Custom
                                 || WrapperMap::Singleton().GetWrapper(propertyId) != nullptr;
                        },
                        [](const css::CSSOMStringAtom &)
                        {
                          // FIXME: This should only be true for properties that are registered custom
                          // properties.
                          return true;
                        }),
                      property);
  }

  bool Equals(const AnimatableCSSProperty &property, const RenderStyle &a, const RenderStyle &b,
              const dom::Document &) noexcept
  {
    return std::visit(krys::CreateVisitor(
                        [&](css::PropertyId propertyId)
                        {
                          if (auto *wrapper = WrapperMap::Singleton().GetWrapper(propertyId))
                          {
                            return wrapper->Equals(a, b);
                          }
                          return true;
                        },
                        [&](const css::CSSOMStringAtom &customProperty)
                        {
                          auto [aCustomPropertyValue, bCustomPropertyValue] =
                            CustomPropertyValuesForInterpolation(customProperty, a, b);
                          if (aCustomPropertyValue && bCustomPropertyValue)
                          {
                            return *aCustomPropertyValue == *bCustomPropertyValue;
                          }
                          return !aCustomPropertyValue && !bCustomPropertyValue;
                        }),
                      property);
  }

  bool CanInterpolate(const AnimatableCSSProperty &property, const RenderStyle &a, const RenderStyle &b,
                      const dom::Document &) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        [&](css::PropertyId propertyId)
        {
          if (auto *wrapper = WrapperMap::Singleton().GetWrapper(propertyId))
          {
            return wrapper->CanInterpolate(a, b, CompositeOperation::Replace);
          }
          return true;
        },
        [&](const css::CSSOMStringAtom &customProperty)
        {
          auto [aCustomPropertyValue, bCustomPropertyValue] =
            CustomPropertyValuesForInterpolation(customProperty, a, b);
          if (!aCustomPropertyValue || !bCustomPropertyValue || aCustomPropertyValue == bCustomPropertyValue)
          {
            return false;
          }
          const auto &aVariantValue = aCustomPropertyValue->GetValue();
          const auto &bVariantValue = bCustomPropertyValue->GetValue();
          if (aVariantValue.index() != bVariantValue.index())
          {
            return false;
          }
          return std::visit(krys::CreateVisitor(
                              [&bVariantValue](const CustomProperty::ValueList &aValueList)
                              {
                                const auto &bValueList = std::get<CustomProperty::ValueList>(bVariantValue);
                                if (aValueList == bValueList)
                                {
                                  return false;
                                }
                                if (auto firstValue = FirstValueInSyntaxValueLists(aValueList, bValueList))
                                {
                                  // List sizes must match except for transform lists.
                                  if (!std::holds_alternative<TransformFunction>(*firstValue)
                                      && aValueList.Values.size() != bValueList.Values.size())
                                  {
                                    return false;
                                  }
                                  return TypeOfSyntaxValueCanBeInterpolated(*firstValue);
                                }
                                return false;
                              },
                              [&bVariantValue](const CustomProperty::Value &aSyntaxValue)
                              {
                                const auto &bSyntaxValue = std::get<CustomProperty::Value>(bVariantValue);
                                return aSyntaxValue != bSyntaxValue
                                       && TypeOfSyntaxValueCanBeInterpolated(aSyntaxValue);
                              },
                              [](const auto &) { return false; }),
                            aVariantValue);
        }),
      property);
  }

  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   IterationCompositeOperation iterationCompositeOperation, double currentIteration,
                   const Client &client) noexcept
  {
    std::visit(
      krys::CreateVisitor(
        [&](css::PropertyId propertyId)
        {
          InterpolateStandardProperty(propertyId, destination, from, to, progress, compositeOperation,
                                      iterationCompositeOperation, currentIteration, client);
        },
        [&](const css::CSSOMStringAtom &customProperty)
        {
          InterpolateCustomProperty(customProperty, destination, from, to, progress, compositeOperation,
                                    iterationCompositeOperation, currentIteration, client);
        }),
      property);
  }

  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   const Client &client) noexcept
  {
    Interpolate(property, destination, from, to, progress, compositeOperation,
                IterationCompositeOperation::Replace, 0, client);
  }

  bool RequiresInterpolationForAccumulativeIteration(const AnimatableCSSProperty &property,
                                                     const RenderStyle &a, const RenderStyle &b,
                                                     const Client &) noexcept
  {
    return std::visit(
      krys::CreateVisitor(
        [&](css::PropertyId propertyId)
        {
          if (auto *wrapper = WrapperMap::Singleton().GetWrapper(propertyId))
          {
            return wrapper->RequiresInterpolationForAccumulativeIteration(a, b);
          }
          return false;
        },
        [&](const css::CSSOMStringAtom &customProperty)
        {
          auto [from, to] = CustomPropertyValuesForInterpolation(customProperty, a, b);
          if (!from || !to)
          {
            return false;
          }

          if (std::holds_alternative<CustomProperty::ValueList>(from->GetValue())
              && std::holds_alternative<CustomProperty::ValueList>(to->GetValue()))
          {
            const auto &fromSyntaxValues = std::get<CustomProperty::ValueList>(from->GetValue()).Values;
            const auto &toSyntaxValues = std::get<CustomProperty::ValueList>(to->GetValue()).Values;
            if (fromSyntaxValues.size() != toSyntaxValues.size())
            {
              return false;
            }
            for (size_t i = 0; i < fromSyntaxValues.size(); ++i)
            {
              if (!SyntaxValuesRequireInterpolationForAccumulativeIteration(fromSyntaxValues[i],
                                                                            toSyntaxValues[i], true))
              {
                return false;
              }
            }
            return true;
          }

          if (std::holds_alternative<CustomProperty::Value>(from->GetValue())
              && std::holds_alternative<CustomProperty::Value>(to->GetValue()))
          {
            const auto &fromSyntaxValue = std::get<CustomProperty::Value>(from->GetValue());
            const auto &toSyntaxValue = std::get<CustomProperty::Value>(to->GetValue());
            return SyntaxValuesRequireInterpolationForAccumulativeIteration(fromSyntaxValue, toSyntaxValue,
                                                                            false);
          }

          return false;
        }),
      property);
  }
}
