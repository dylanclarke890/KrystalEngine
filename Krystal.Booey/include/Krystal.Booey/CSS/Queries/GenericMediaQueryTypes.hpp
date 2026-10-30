#pragma once

#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
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

  /// @brief One side of a feature comparison: an operator and the value it compares against.
  struct Comparison
  {
    ComparisonOperator Op;
    RefPtr<css::Value> Value;

    KRYS_NODISCARD RefPtr<css::Value> ProtectedValue() const noexcept
    {
      return Value;
    }
  };

  /// @brief A media or container feature, in boolean, plain (`name: value`) or range syntax.
  struct Feature
  {
    CSSOMStringAtom Name {CSSOMStringAtom::Null()};
    mq::Syntax Syntax {mq::Syntax::Boolean};
    Maybe<Comparison> LeftComparison;
    Maybe<Comparison> RightComparison;

    Maybe<ValueId> FunctionId {};

    const FeatureSchema *Schema {nullptr};
  };

  /// @brief An unrecognised `<general-enclosed>` production, kept for serialisation.
  struct GeneralEnclosed
  {
    CSSOMString Name;
    CSSOMString Text;
  };

  using QueryInParens = Variant<Condition, Feature, GeneralEnclosed>;

  /// @brief A condition: queries joined by one logical operator, or a single negated query.
  struct Condition
  {
    mq::LogicalOperator LogicalOperator {mq::LogicalOperator::And};
    List<QueryInParens> Queries;

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
    None = 0,
    Viewport = 1 << 0,
    Appearance = 1 << 1,
    Accessibility = 1 << 2,
  };

  /// @brief What a feature is evaluated against.
  struct FeatureEvaluationContext
  {
    WeakRef<const dom::Document> Document;
    ToLengthConversionData ConversionData {};
    CheckedPtr<const RenderElement> Renderer {};
  };

  /// @brief Describes a feature the parser accepts: its name, whether it's discrete or a range, and the type
  /// of value it takes.
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
    FixedVector<ValueId> ValueIdentifiers;

    KRYS_NODISCARD virtual EvaluationResult Evaluate(const Feature &,
                                                     const FeatureEvaluationContext &) const noexcept
    {
      return EvaluationResult::Unknown;
    }

    FeatureSchema(const CSSOMStringAtom &name, Type type, ValueType valueType,
                  MediaQueryDynamicDependency dependencies,
                  FixedVector<ValueId> valueIdentifiers = {}) noexcept
        : Name(name), FeatureType(type), FeatureValueType(valueType), Dependencies(dependencies),
          ValueIdentifiers(krys::move(valueIdentifiers))
    {
    }

    virtual ~FeatureSchema() noexcept = default;
  };

  template <typename TraverseFunction>
  void TraverseFeatures(const Condition &, TraverseFunction &&) noexcept;

  template <typename TraverseFunction>
  void TraverseFeatures(const QueryInParens &queryInParens, TraverseFunction &&function) noexcept
  {
    krys::SwitchOn(
      queryInParens, [&](const Condition &condition) { TraverseFeatures(condition, function); },
      [&](const Feature &feature) { function(feature); },
      [&](const GeneralEnclosed &)
      {
        Feature dummy {};
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

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::mq::MediaQueryDynamicDependency, 3uz);
