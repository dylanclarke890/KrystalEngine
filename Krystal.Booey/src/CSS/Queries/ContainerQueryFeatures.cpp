#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/FrequencyDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/TimeDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Rendering/Grid/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Builder/Builder.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/Rules/MatchResult.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css::cq
{
  enum class UsesSVGZoomRulesForLength : bool
  {
    No,
    Yes
  };

  namespace
  {
    KRYS_NODISCARD LayoutUnit UnscaledSizeForPrincipleBox(const style::PreferredSize &computedSize,
                                                          LayoutUnit usedSize,
                                                          UsesSVGZoomRulesForLength usesSVGZoomRulesForLength,
                                                          float usedZoom) noexcept
    {
      if (usesSVGZoomRulesForLength == UsesSVGZoomRulesForLength::Yes || !computedSize.IsFixed())
      {
        return usedSize;
      }
      return LayoutUnit {usedSize / usedZoom};
    }

    KRYS_NODISCARD UsesSVGZoomRulesForLength UsesSVGZoomRules(const RenderStyle &style) noexcept
    {
      return style.UseSVGZoomRulesForLength() ? UsesSVGZoomRulesForLength::Yes
                                              : UsesSVGZoomRulesForLength::No;
    }
  }

  struct SizeFeatureSchema : public mq::FeatureSchema
  {
    SizeFeatureSchema(const CSSOMStringAtom &name, Type type, ValueType valueType,
                      mq::MediaQueryDynamicDependency dependencies,
                      FixedVector<ValueId> &&valueIdentifiers = {}) noexcept
        : mq::FeatureSchema(name, type, valueType, dependencies, krys::move(valueIdentifiers))
    {
    }

    KRYS_NODISCARD mq::EvaluationResult
      Evaluate(const mq::Feature &feature,
               const mq::FeatureEvaluationContext &context) const noexcept override
    {
      // "If the query container does not have a principal box, or the principal box is not a layout
      // containment box, or the query container does not support container size queries on the relevant axes,
      // then the result of evaluating the size feature is unknown."
      // https://drafts.csswg.org/css-contain-3/#size-container
      auto *renderer = DynamicDowncast<RenderBox>(context.Renderer.get());
      if (!renderer)
      {
        return mq::EvaluationResult::Unknown;
      }

      if (!renderer->HasEligibleContainmentForSizeQuery())
      {
        return mq::EvaluationResult::Unknown;
      }

      return Evaluate(feature, *renderer, context.ConversionData);
    }

    KRYS_NODISCARD virtual mq::EvaluationResult Evaluate(const mq::Feature &, const RenderBox &,
                                                         const ToLengthConversionData &) const noexcept = 0;
  };

  namespace Features
  {
    using Dependency = mq::MediaQueryDynamicDependency;

    struct WidthFeatureSchema : public SizeFeatureSchema
    {
      WidthFeatureSchema() noexcept
          : SizeFeatureSchema(u8"width", mq::FeatureSchema::Type::Range, mq::FeatureSchema::ValueType::Length,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto width = UnscaledSizeForPrincipleBox(renderStyle.Width(), renderer.ContentBoxWidth(),
                                                 UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return mq::EvaluateLengthFeature(feature, width, conversionData);
      }
    };

    struct HeightFeatureSchema : public SizeFeatureSchema
    {
      HeightFeatureSchema() noexcept
          : SizeFeatureSchema(u8"height", mq::FeatureSchema::Type::Range,
                              mq::FeatureSchema::ValueType::Length, Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto height = UnscaledSizeForPrincipleBox(renderStyle.Height(), renderer.ContentBoxHeight(),
                                                  UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return mq::EvaluateLengthFeature(feature, height, conversionData);
      }
    };

    struct InlineSizeFeatureSchema : public SizeFeatureSchema
    {
      InlineSizeFeatureSchema() noexcept
          : SizeFeatureSchema(u8"inline-size", mq::FeatureSchema::Type::Range,
                              mq::FeatureSchema::ValueType::Length, Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto logicalWidth =
          UnscaledSizeForPrincipleBox(renderStyle.LogicalWidth(), renderer.ContentBoxLogicalWidth(),
                                      UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return mq::EvaluateLengthFeature(feature, logicalWidth, conversionData);
      }
    };

    struct BlockSizeFeatureSchema : public SizeFeatureSchema
    {
      BlockSizeFeatureSchema() noexcept
          : SizeFeatureSchema(u8"block-size", mq::FeatureSchema::Type::Range,
                              mq::FeatureSchema::ValueType::Length, Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto logicalHeight =
          UnscaledSizeForPrincipleBox(renderStyle.LogicalHeight(), renderer.ContentBoxLogicalHeight(),
                                      UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return mq::EvaluateLengthFeature(feature, logicalHeight, conversionData);
      }
    };

    struct AspectRatioFeatureSchema : public SizeFeatureSchema
    {
      AspectRatioFeatureSchema() noexcept
          : SizeFeatureSchema(u8"aspect-ratio", mq::FeatureSchema::Type::Range,
                              mq::FeatureSchema::ValueType::Ratio, Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        return mq::EvaluateRatioFeature(feature, FloatSize(renderer.ContentBoxSize()), conversionData);
      }
    };

    struct OrientationFeatureSchema : public SizeFeatureSchema
    {
      OrientationFeatureSchema() noexcept
          : SizeFeatureSchema(u8"orientation", mq::FeatureSchema::Type::Discrete,
                              mq::FeatureSchema::ValueType::Identifier, Dependency::Viewport,
                              FixedVector {ValueId::Portrait, ValueId::Landscape})
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        bool isPortrait = renderer.ContentBoxHeight() >= renderer.ContentBoxWidth();
        return mq::EvaluateIdentifierFeature(feature, isPortrait ? ValueId::Portrait : ValueId::Landscape,
                                             conversionData);
      }
    };

    struct StyleFeatureSchema : public mq::FeatureSchema
    {
      StyleFeatureSchema() noexcept
          : mq::FeatureSchema(u8"style", mq::FeatureSchema::Type::Discrete,
                              mq::FeatureSchema::ValueType::CustomProperty, Dependency::None)
      {
      }

      // mq::FeatureSchema conformance

      KRYS_NODISCARD mq::EvaluationResult
        Evaluate(const mq::Feature &feature,
                 const mq::FeatureEvaluationContext &context) const noexcept override
      {
        if (!context.ConversionData.Style() || !context.ConversionData.ParentStyle())
        {
          return mq::EvaluationResult::False;
        }

        auto &style = *context.ConversionData.Style();

        auto *customPropertyValue = style.CustomPropertyValue(feature.Name);
        if (!feature.RightComparison)
        {
          return mq::ToEvaluationResult(customPropertyValue && !customPropertyValue->IsGuaranteedInvalid());
        }

        auto *featureValue = DynamicDowncast<CustomPropertyValue>(feature.RightComparison->Value.get());
        krys_debug_assert(featureValue);
        auto resolvedFeatureValue = ResolveQueriedValue(*featureValue, context);

        if (!resolvedFeatureValue)
        {
          return mq::EvaluationResult::False;
        }

        // Guaranteed-invalid values match guaranteed-invalid values.
        if (resolvedFeatureValue->IsGuaranteedInvalid())
        {
          return mq::ToEvaluationResult(!customPropertyValue || customPropertyValue->IsGuaranteedInvalid());
        }

        krys_debug_assert(feature.RightComparison->Op == mq::ComparisonOperator::Equal);
        return mq::ToEvaluationResult(customPropertyValue && *customPropertyValue == *resolvedFeatureValue);
      }

      // https://drafts.csswg.org/css-conditional-5/#evaluate-a-style-range
      KRYS_NODISCARD mq::EvaluationResult
        EvaluateStyleRange(const mq::StyleRange &range,
                           const mq::FeatureEvaluationContext &context) const noexcept override
      {
        if (!context.ConversionData.Style() || !context.ConversionData.ParentStyle())
        {
          return mq::EvaluationResult::False;
        }

        List<RefPtr<PrimitiveValue>> values;
        for (auto &rangeValue : range.Values)
        {
          // A custom property name is substituted as if it were var(), and other values have their
          // substitution functions replaced.
          auto substituted = [&]() -> RefPtr<const style::CustomProperty>
          {
            if (!rangeValue.Value)
            {
              return ShareRefPtr(
                context.ConversionData.Style()->CustomPropertyValue(rangeValue.CustomPropertyName));
            }
            return ResolveQueriedValue(Downcast<CustomPropertyValue>(*rangeValue.Value), context);
          }();
          if (!substituted || substituted->IsGuaranteedInvalid())
          {
            return mq::EvaluationResult::False;
          }

          // Then each must parse as a <number>, <percentage>, <length>, <angle>, <time>, <frequency> or
          // <resolution>.
          TokenRange tokens {Span<const Token>(substituted->Tokens())};
          tokens.DiscardWhitespace();
          auto state = PropertyParserState {.Context = StrictParserContext()};
          auto value = PrimitiveValueResolver<Number<>, Percentage<>, Length<>, Angle<>, Time<>, Frequency<>,
                                              Resolution<>>::ConsumeAndResolve(tokens, state);
          tokens.DiscardWhitespace();
          if (!value || !tokens.IsAtEnd())
          {
            return mq::EvaluationResult::False;
          }
          values.push_back(krys::move(value));
        }

        auto category = [](const PrimitiveValue &value)
        {
          return value.IsNumberOrInteger() ? UnitCategory::Number
                                           : UnitCategoryForType(value.PrimitiveType());
        };

        // A unitless zero is a zero length when compared against lengths.
        bool hasLength = std::ranges::any_of(values, [](auto &value) { return value->IsLength(); });

        Maybe<UnitCategory> commonCategory;
        List<double> numbers;
        auto &conversionData = context.ConversionData;
        for (auto &value : values)
        {
          auto valueCategory = category(*value);
          if (hasLength && valueCategory == UnitCategory::Number && !value->ResolveAsNumber(conversionData))
          {
            numbers.push_back(0.0);
            valueCategory = UnitCategoryForType(UnitType::px);
          }
          else
          {
            switch (valueCategory)
            {
              case UnitCategory::Number: numbers.push_back(value->ResolveAsNumber(conversionData)); break;
              case UnitCategory::Percent:
                numbers.push_back(value->ResolveAsPercentage(conversionData));
                break;
              case UnitCategory::Angle: numbers.push_back(value->ResolveAsAngle(conversionData)); break;
              case UnitCategory::Time:  numbers.push_back(value->ResolveAsTime(conversionData)); break;
              case UnitCategory::Resolution:
                numbers.push_back(value->ResolveAsResolution(conversionData));
                break;
              case UnitCategory::Frequency:
                numbers.push_back(PrimitiveValue::ComputeFrequency<FrequencyUnit::Hz>(
                  value->PrimitiveType(), value->Value<double>(conversionData)));
                break;
              default:
                if (!value->IsLength())
                {
                  return mq::EvaluationResult::False;
                }
                numbers.push_back(value->ResolveAsLength<double>(conversionData));
                valueCategory = UnitCategoryForType(UnitType::px);
                break;
            }
          }

          if (commonCategory && *commonCategory != valueCategory)
          {
            return mq::EvaluationResult::False;
          }
          commonCategory = valueCategory;
        }

        for (auto i = 0uz; i < range.Operators.size(); ++i)
        {
          if (!Compare(range.Operators[i], numbers[i], numbers[i + 1uz]))
          {
            return mq::EvaluationResult::False;
          }
        }
        return mq::EvaluationResult::True;
      }

    private:
      KRYS_NODISCARD static bool Compare(mq::ComparisonOperator op, double left, double right) noexcept
      {
        switch (op)
        {
          case mq::ComparisonOperator::LessThan:           return left < right;
          case mq::ComparisonOperator::LessThanOrEqual:    return left <= right;
          case mq::ComparisonOperator::Equal:              return left == right;
          case mq::ComparisonOperator::GreaterThan:        return left > right;
          case mq::ComparisonOperator::GreaterThanOrEqual: return left >= right;
        }
        krys_unreachable();
      }

      /// @brief Resolves a queried value for var() references, CSS-wide keywords and registered properties.
      KRYS_NODISCARD static RefPtr<const style::CustomProperty>
        ResolveQueriedValue(const CustomPropertyValue &value,
                            const mq::FeatureEvaluationContext &context) noexcept
      {
        auto builderContext =
          style::BuilderContext {ShareRefPtr(context.Document.get()), context.ConversionData.ParentStyle(),
                                 context.ConversionData.RootStyle(),
                                 ShareRefPtr(context.ConversionData.ElementForContainerUnitResolution())};

        auto dummyStyle = RenderStyle::Clone(*context.ConversionData.Style());
        auto dummyMatchResult = style::MatchResult::Create();

        auto styleBuilder = style::Builder {dummyStyle, krys::move(builderContext), *dummyMatchResult};
        return styleBuilder.ResolveCustomPropertyForContainerQueries(value);
      }
    };

    // MARK: - Singleton readonly instances of FeatureSchemas

    const mq::FeatureSchema &Width() noexcept
    {
      static MainThreadNeverDestroyed<WidthFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &Height() noexcept
    {
      static MainThreadNeverDestroyed<HeightFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &InlineSize() noexcept
    {
      static MainThreadNeverDestroyed<InlineSizeFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &BlockSize() noexcept
    {
      static MainThreadNeverDestroyed<BlockSizeFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &AspectRatio() noexcept
    {
      static MainThreadNeverDestroyed<AspectRatioFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &Orientation() noexcept
    {
      static MainThreadNeverDestroyed<OrientationFeatureSchema> schema;
      return schema;
    }

    const mq::FeatureSchema &Style() noexcept
    {
      static MainThreadNeverDestroyed<StyleFeatureSchema> schema;
      return schema;
    }

    List<const mq::FeatureSchema *> AllSchemas() noexcept
    {
      return {
        &Width(), &Height(), &InlineSize(), &BlockSize(), &AspectRatio(), &Orientation(),
      };
    }
  }
}
