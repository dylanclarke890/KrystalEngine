/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/FixedList.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/CheckedPtr.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"
#include "Krystal.Web/Css/Types/CSSOMString.hpp"
#include "Krystal.Web/Css/Values/ToLengthConversionData.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"
#include "Krystal.Web/Css/Values/ValueId.hpp"
#include "Krystal.Web/Rendering/RenderElement.hpp"

namespace krys::web
{
  class Document;
  class RenderElement;
}

namespace krys::web::css::mq
{
  enum class LogicalOperator : uint8
  {
    And,
    Or,
    Not,
  };

  enum class ComparisonOperator : uint8
  {
    LessThan,
    LessThanOrEqual,
    Equal,
    GreaterThan,
    GreaterThanOrEqual,
  };

  enum class Syntax : uint8
  {
    Boolean,
    Plain,
    Range,
  };

  struct Condition;
  struct FeatureSchema;

  /// @brief One side of a feature comparison: an operator and the value it compares against.
  struct Comparison
  {
    ComparisonOperator Op;
    RefPtr<css::Value> Value;
  };

  /// @brief A media or container feature, in boolean, plain (`name: value`) or range syntax.
  struct Feature
  {
    CSSOMStringAtom Name {null};
    mq::Syntax Syntax {mq::Syntax::Boolean};
    Maybe<Comparison> LeftComparison;
    Maybe<Comparison> RightComparison;

    Maybe<ValueId> FunctionId;

    const FeatureSchema *Schema {nullptr};
  };

  /// @brief An unrecognised `<general-enclosed>` production, kept for serialisation.
  struct GeneralEnclosed
  {
    CSSOMString Name;
    CSSOMString Text;
  };

  /// @brief A value in a style range: a custom property name, or a value to compare against.
  struct StyleRangeValue
  {
    CSSOMStringAtom CustomPropertyName {null};
    /// @brief The value, as a `CustomPropertyValue`, when this isn't a custom property name.
    RefPtr<css::Value> Value;
  };

  /// @brief A `<style-range>` in a container `style()` query: two or three values joined by comparisons, like
  /// `--a > 1px` or `0 < --a <= --b`.
  struct StyleRange
  {
    List<StyleRangeValue> Values;
    /// @brief The comparison between each pair of adjacent values.
    List<ComparisonOperator> Operators;

    Maybe<ValueId> FunctionId;

    const FeatureSchema *Schema {nullptr};
  };

  using QueryInParens = Variant<Condition, Feature, StyleRange, GeneralEnclosed>;

  /// @brief A condition: queries joined by one logical operator, or a single negated query.
  struct Condition
  {
    mq::LogicalOperator LogicalOperator {mq::LogicalOperator::And};
    List<QueryInParens> Queries;

    Maybe<ValueId> FunctionId;
  };

  enum class EvaluationResult : uint8
  {
    False,
    True,
    Unknown,
  };

  enum class MediaQueryDynamicDependency : uint8
  {
    Viewport,
    Appearance,
    Accessibility,
  };

  consteval MediaQueryDynamicDependency MaxEnumValue(MediaQueryDynamicDependency) noexcept
  {
    return MediaQueryDynamicDependency::Accessibility;
  }

  /// @brief What a feature is evaluated against.
  struct FeatureEvaluationContext
  {
    WeakRef<const web::Document> Document;
    ToLengthConversionData ConversionData {};
    CheckedPtr<const RenderElement> Renderer;
  };

  /// @brief Describes a feature the parser accepts: its name, whether it's discrete or a range, and the type
  /// of value it takes.
  struct FeatureSchema
  {
    enum class Type : uint8
    {
      Discrete,
      Range,
    };

    enum class ValueType : uint8
    {
      Integer,
      Number,
      Length,
      Ratio,
      Resolution,
      Identifier,
      CustomProperty,
    };

    CSSOMStringAtom Name;
    Type FeatureType;
    ValueType FeatureValueType;
    EnumSet<MediaQueryDynamicDependency> Dependencies;
    FixedList<ValueId> ValueIdentifiers;

    KRYS_NODISCARD virtual EvaluationResult Evaluate(const Feature &,
                                                     const FeatureEvaluationContext &) const noexcept
    {
      return EvaluationResult::Unknown;
    }

    KRYS_NODISCARD virtual EvaluationResult
      EvaluateStyleRange(const StyleRange &, const FeatureEvaluationContext &) const noexcept
    {
      return EvaluationResult::Unknown;
    }

    FeatureSchema(CSSOMStringAtom name, Type type, ValueType valueType,
                  EnumSet<MediaQueryDynamicDependency> dependencies,
                  FixedList<ValueId> valueIdentifiers = {}) noexcept
        : Name(krys::move(name)), FeatureType(type), FeatureValueType(valueType), Dependencies(dependencies),
          ValueIdentifiers(krys::move(valueIdentifiers))
    {
    }

    virtual ~FeatureSchema() noexcept = default;
  };

  template <typename TraverseFunction>
  void TraverseFeatures(const Condition &, TraverseFunction) noexcept;

  template <typename TraverseFunction>
  void TraverseFeatures(const QueryInParens &queryInParens, TraverseFunction function) noexcept
  {
    krys::SwitchOn(
      queryInParens, [&](const Condition &condition) { TraverseFeatures(condition, function); },
      [&](const Feature &feature) { function(feature); },
      [&](const StyleRange &range)
      {
        // Visit a range as the features it queries, one per custom property name.
        bool hasCustomPropertyName = false;
        for (auto &value : range.Values)
        {
          if (!value.CustomPropertyName.View().empty())
          {
            hasCustomPropertyName = true;
            Feature feature {value.CustomPropertyName, Syntax::Range};
            feature.Schema = range.Schema;
            function(feature);
          }
        }
        if (!hasCustomPropertyName)
        {
          Feature feature {null, Syntax::Range};
          feature.Schema = range.Schema;
          function(feature);
        }
      },
      [&](const GeneralEnclosed &)
      {
        Feature dummy {};
        function(dummy);
      });
  }

  template <typename TraverseFunction>
  void TraverseFeatures(const Condition &condition, TraverseFunction function) noexcept
  {
    for (auto &queryInParens : condition.Queries)
    {
      TraverseFeatures(queryInParens, function);
    }
  }
}
