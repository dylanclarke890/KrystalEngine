#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Queries/GenericMediaQueryEvaluator.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

namespace krys::boo::css::cq
{
  using namespace mq;

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

  struct SizeFeatureSchema : public FeatureSchema
  {
    SizeFeatureSchema(const CSSOMStringAtom &name, Type type, ValueType valueType,
                      MediaQueryDynamicDependency dependencies,
                      FixedVector<ValueId> &&valueIdentifiers = {}) noexcept
        : FeatureSchema(name, type, valueType, dependencies, krys::move(valueIdentifiers))
    {
    }

    KRYS_NODISCARD EvaluationResult Evaluate(const Feature &feature,
                                             const FeatureEvaluationContext &context) const noexcept override
    {
      // "If the query container does not have a principal box, or the principal box is not a layout
      // containment box, or the query container does not support container size queries on the relevant axes,
      // then the result of evaluating the size feature is unknown."
      // https://drafts.csswg.org/css-contain-3/#size-container
      auto *renderer = DynamicDowncast<RenderBox>(context.Renderer.get());
      if (!renderer)
      {
        return EvaluationResult::Unknown;
      }

      if (!renderer->HasEligibleContainmentForSizeQuery())
      {
        return EvaluationResult::Unknown;
      }

      return Evaluate(feature, *renderer, context.ConversionData);
    }

    KRYS_NODISCARD virtual EvaluationResult Evaluate(const Feature &, const RenderBox &,
                                                     const ToLengthConversionData &) const noexcept = 0;
  };

  namespace Features
  {
    using Dependency = MediaQueryDynamicDependency;

    struct WidthFeatureSchema : public SizeFeatureSchema
    {
      WidthFeatureSchema() noexcept
          : SizeFeatureSchema(u8"width", FeatureSchema::Type::Range, FeatureSchema::ValueType::Length,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto width = UnscaledSizeForPrincipleBox(renderStyle.Width(), renderer.ContentBoxWidth(),
                                                 UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return EvaluateLengthFeature(feature, width, conversionData);
      }
    };

    struct HeightFeatureSchema : public SizeFeatureSchema
    {
      HeightFeatureSchema() noexcept
          : SizeFeatureSchema(u8"height", FeatureSchema::Type::Range, FeatureSchema::ValueType::Length,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto height = UnscaledSizeForPrincipleBox(renderStyle.Height(), renderer.ContentBoxHeight(),
                                                  UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return EvaluateLengthFeature(feature, height, conversionData);
      }
    };

    struct InlineSizeFeatureSchema : public SizeFeatureSchema
    {
      InlineSizeFeatureSchema() noexcept
          : SizeFeatureSchema(u8"inline-size", FeatureSchema::Type::Range, FeatureSchema::ValueType::Length,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto logicalWidth =
          UnscaledSizeForPrincipleBox(renderStyle.LogicalWidth(), renderer.ContentBoxLogicalWidth(),
                                      UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return EvaluateLengthFeature(feature, logicalWidth, conversionData);
      }
    };

    struct BlockSizeFeatureSchema : public SizeFeatureSchema
    {
      BlockSizeFeatureSchema() noexcept
          : SizeFeatureSchema(u8"block-size", FeatureSchema::Type::Range, FeatureSchema::ValueType::Length,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        const auto &renderStyle = renderer.Style();
        auto logicalHeight =
          UnscaledSizeForPrincipleBox(renderStyle.LogicalHeight(), renderer.ContentBoxLogicalHeight(),
                                      UsesSVGZoomRules(renderStyle), renderStyle.UsedZoom());
        return EvaluateLengthFeature(feature, logicalHeight, conversionData);
      }
    };

    struct AspectRatioFeatureSchema : public SizeFeatureSchema
    {
      AspectRatioFeatureSchema() noexcept
          : SizeFeatureSchema(u8"aspect-ratio", FeatureSchema::Type::Range, FeatureSchema::ValueType::Ratio,
                              Dependency::Viewport)
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        return EvaluateRatioFeature(feature, FloatSize(renderer.ContentBoxSize()), conversionData);
      }
    };

    struct OrientationFeatureSchema : public SizeFeatureSchema
    {
      OrientationFeatureSchema() noexcept
          : SizeFeatureSchema(u8"orientation", FeatureSchema::Type::Discrete,
                              FeatureSchema::ValueType::Identifier, Dependency::Viewport,
                              FixedVector {ValueId::Portrait, ValueId::Landscape})
      {
      }

      // SizeFeatureSchema conformance

      using SizeFeatureSchema::Evaluate;
      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const RenderBox &renderer,
                 const ToLengthConversionData &conversionData) const noexcept override
      {
        bool isPortrait = renderer.ContentBoxHeight() >= renderer.ContentBoxWidth();
        return EvaluateIdentifierFeature(feature, isPortrait ? ValueId::Portrait : ValueId::Landscape,
                                         conversionData);
      }
    };

    struct StyleFeatureSchema : public FeatureSchema
    {
      StyleFeatureSchema() noexcept
          : FeatureSchema(u8"style", FeatureSchema::Type::Discrete, FeatureSchema::ValueType::CustomProperty,
                          Dependency::None)
      {
      }

      // FeatureSchema conformance

      KRYS_NODISCARD EvaluationResult
        Evaluate(const Feature &feature, const FeatureEvaluationContext &context) const noexcept override
      {
        if (!context.ConversionData.Style() || !context.ConversionData.ParentStyle())
        {
          return EvaluationResult::False;
        }

        auto &style = *context.ConversionData.Style();

        auto *customPropertyValue = style.CustomPropertyValue(feature.Name);
        if (!feature.RightComparison)
        {
          return ToEvaluationResult(customPropertyValue && !customPropertyValue->IsGuaranteedInvalid());
        }

        auto resolvedFeatureValue = [&]() -> RefPtr<const style::CustomProperty>
        {
          auto *featureValue = DynamicDowncast<CustomPropertyValue>(feature.RightComparison->Value.get());
          krys_debug_assert(featureValue);

          // Resolve the queried custom property value for var() references, css-wide keywords and registered
          // properties.
          auto builderContext =
            style::BuilderContext {ShareRefPtr(context.Document.get()), context.ConversionData.ParentStyle(),
                                   context.ConversionData.RootStyle(),
                                   ShareRefPtr(context.ConversionData.ElementForContainerUnitResolution())};

          auto dummyStyle = RenderStyle::Clone(style);
          auto dummyMatchResult = style::MatchResult::Create();

          auto styleBuilder = style::Builder {dummyStyle, krys::move(builderContext), *dummyMatchResult};
          return styleBuilder.ResolveCustomPropertyForContainerQueries(*featureValue);
        }();

        if (!resolvedFeatureValue)
        {
          return EvaluationResult::False;
        }

        // Guaranteed-invalid values match guaranteed-invalid values.
        if (resolvedFeatureValue->IsGuaranteedInvalid())
        {
          return ToEvaluationResult(!customPropertyValue || customPropertyValue->IsGuaranteedInvalid());
        }

        krys_debug_assert(feature.RightComparison->Op == ComparisonOperator::Equal);
        return ToEvaluationResult(customPropertyValue && *customPropertyValue == *resolvedFeatureValue);
      }
    };

    // MARK: - Singleton readonly instances of FeatureSchemas

    const FeatureSchema &Width() noexcept
    {
      static MainThreadNeverDestroyed<WidthFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &Height() noexcept
    {
      static MainThreadNeverDestroyed<HeightFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &InlineSize() noexcept
    {
      static MainThreadNeverDestroyed<InlineSizeFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &BlockSize() noexcept
    {
      static MainThreadNeverDestroyed<BlockSizeFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &AspectRatio() noexcept
    {
      static MainThreadNeverDestroyed<AspectRatioFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &Orientation() noexcept
    {
      static MainThreadNeverDestroyed<OrientationFeatureSchema> schema;
      return schema;
    }

    const FeatureSchema &Style() noexcept
    {
      static MainThreadNeverDestroyed<StyleFeatureSchema> schema;
      return schema;
    }

    List<const FeatureSchema *> AllSchemas() noexcept
    {
      return {
        &Width(), &Height(), &InlineSize(), &BlockSize(), &AspectRatio(), &Orientation(),
      };
    }
  }
}
