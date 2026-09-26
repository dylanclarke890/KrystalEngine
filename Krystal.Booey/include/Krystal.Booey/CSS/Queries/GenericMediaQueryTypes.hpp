#pragma once

#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo
{
  class RenderElement;
}

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::css::mq
{
  enum class LogicalOperator : uint8
  {
    And,
    Or,
    Not
  };

  enum class ComparisonOperator : uint8
  {
    LessThan,
    LessThanOrEqual,
    Equal,
    GreaterThan,
    GreaterThanOrEqual
  };

  enum class Syntax : uint8
  {
    Boolean,
    Plain,
    Range
  };

  struct Condition;

  struct FeatureSchema;

  struct Comparison
  {
    ComparisonOperator Op;
    RefPtr<Value> Value;
  };

  struct Feature
  {
    CSSOMStringAtom Name;
    Syntax Syntax;
    Maybe<Comparison> LeftComparison;
    Maybe<Comparison> RightComparison;

    Maybe<ValueId> FunctionId {};

    const FeatureSchema *Schema {nullptr};
  };

  struct GeneralEnclosed
  {
    CSSOMString Name;
    CSSOMString Text;
  };

  using QueryInParens = Variant<Condition, Feature, GeneralEnclosed>;

  struct Condition
  {
    LogicalOperator LogicalOperator {LogicalOperator::And};
    SmallList<QueryInParens> Queries;

    Maybe<ValueId> FunctionId {};
  };

  enum class EvaluationResult : uint8
  {
    False,
    True,
    Unknown
  };

  enum class MediaQueryDynamicDependency : uint8
  {
    Viewport = 1 << 0,
    Appearance = 1 << 1,
    Accessibility = 1 << 2,
  };

  struct FeatureEvaluationContext
  {
    WeakRef<const dom::Document> Document;
    ToLengthConversionData ConversionData {};
    CheckedPtr<const RenderElement> Renderer {};
  };

  struct FeatureSchema
  {
    enum class Type : uint8
    {
      Discrete,
      Range
    };

    enum class ValueType : uint8
    {
      Integer,
      Number,
      Length,
      Ratio,
      Resolution,
      Identifier,
      CustomProperty
    };

    CSSOMStringAtom Name;
    Type FeatureType;
    ValueType FeatureValueType;
    MediaQueryDynamicDependency Dependencies;
    SmallList<ValueId> ValueIdentifiers;

    KRYS_NODISCARD virtual EvaluationResult Evaluate(const Feature &feature,
                                                     const FeatureEvaluationContext &context) const noexcept
    {
      return EvaluationResult::Unknown;
    }

    FeatureSchema(const CSSOMStringAtom &name, Type type, ValueType valueType,
                  MediaQueryDynamicDependency dependencies,
                  SmallList<ValueId> &&valueIdentifiers = {}) noexcept
        : Name(name), FeatureType(type), FeatureValueType(valueType), Dependencies(dependencies),
          ValueIdentifiers(::krys::move(valueIdentifiers))
    {
    }

    virtual ~FeatureSchema() noexcept = default;
  };

  template <typename TraverseFunction>
  void TraverseFeatures(const Condition &, TraverseFunction &&) noexcept;

  template <typename TraverseFunction>
  void TraverseFeatures(const QueryInParens &queryInParens, TraverseFunction &&function) noexcept
  {
    return krys::SwitchOn(
      queryInParens, [&](const Condition &condition) { TraverseFeatures(condition, function); },
      [&](const mq::Feature &feature) { function(feature); },
      [&](const mq::GeneralEnclosed &)
      {
        mq::Feature dummy {};
        function(dummy);
      });
  }

  template <typename TraverseFunction>
  void TraverseFeatures(const Condition &condition, TraverseFunction &&function) noexcept
  {
    for (auto &queryInParens : condition.Queries)
    {
      TraverseFeatures(queryInParens, function);
    }
  }
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::mq::MediaQueryDynamicDependency, 4uz);
