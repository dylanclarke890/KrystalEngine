#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::mq
{
  static void Serialise(StringBuilder &builder, const QueryInParens &queryInParens) noexcept
  {
    krys::SwitchOn(
      queryInParens,
      [&](const auto &node)
      {
        if (node.FunctionId)
        {
          builder.Append(ToString(*node.FunctionId));
        }
        builder.Append(u8'(');
        Serialise(builder, node);
        builder.Append(u8')');
      },
      [&](const GeneralEnclosed &generalEnclosed)
      {
        builder.Append(generalEnclosed.Name);
        builder.Append(u8'(');
        builder.Append(generalEnclosed.Text);
        builder.Append(u8')');
      });
  }

  void Serialise(StringBuilder &builder, const Condition &condition) noexcept
  {
    if (condition.Queries.size() == 1 && condition.LogicalOperator == LogicalOperator::Not)
    {
      builder.Append(u8"not ");
      Serialise(builder, condition.Queries.front());
      return;
    }

    for (auto &query : condition.Queries)
    {
      if (&query != &condition.Queries.front())
      {
        builder.Append(condition.LogicalOperator == LogicalOperator::And ? u8" and " : u8" or ");
      }
      Serialise(builder, query);
    }
  }

  void Serialise(StringBuilder &builder, const Feature &feature) noexcept
  {
    auto serialiseRangeComparisonOperator = [&](ComparisonOperator op)
    {
      builder.Append(u8' ');
      switch (op)
      {
        case ComparisonOperator::LessThan:           builder.Append(u8'<'); break;
        case ComparisonOperator::LessThanOrEqual:    builder.Append(u8"<="); break;
        case ComparisonOperator::Equal:              builder.Append(u8'='); break;
        case ComparisonOperator::GreaterThan:        builder.Append(u8'>'); break;
        case ComparisonOperator::GreaterThanOrEqual: builder.Append(u8">="); break;
      }
      builder.Append(u8' ');
    };

    CSSOMString name {feature.Name.View()};

    switch (feature.Syntax)
    {
      case Syntax::Boolean: SerialiseIdentifier(name, builder); break;

      case Syntax::Plain:
        switch (feature.RightComparison->Op)
        {
          case ComparisonOperator::LessThanOrEqual:    builder.Append(u8"max-"); break;
          case ComparisonOperator::Equal:              break;
          case ComparisonOperator::GreaterThanOrEqual: builder.Append(u8"min-"); break;
          case ComparisonOperator::LessThan:
          case ComparisonOperator::GreaterThan:
            krys_debug_assert_msg(false, "Plain features only use <=, = and >=");
            break;
        }
        SerialiseIdentifier(name, builder);

        builder.Append(u8": ");
        builder.Append(feature.RightComparison->ProtectedValue()->CssText(DefaultSerialisationContext()));
        break;

      case Syntax::Range:
        if (feature.LeftComparison)
        {
          builder.Append(feature.LeftComparison->ProtectedValue()->CssText(DefaultSerialisationContext()));
          serialiseRangeComparisonOperator(feature.LeftComparison->Op);
        }

        SerialiseIdentifier(name, builder);

        if (feature.RightComparison)
        {
          serialiseRangeComparisonOperator(feature.RightComparison->Op);
          builder.Append(feature.RightComparison->ProtectedValue()->CssText(DefaultSerialisationContext()));
        }
        break;
    }
  }
}
