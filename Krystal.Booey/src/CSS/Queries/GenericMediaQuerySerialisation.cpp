#include "Krystal.Booey/CSS/Queries/GenericMediaQuerySerialisation.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::mq
{
  static void Serialise(CSSOMString &builder, const QueryInParens &queryInParens) noexcept
  {
    krys::SwitchOn(
      queryInParens,
      [&](const auto &node)
      {
        if (node.FunctionId)
        {
          builder += ToString(*node.FunctionId);
        }
        builder += u8'(';
        Serialise(builder, node);
        builder += u8')';
      },
      [&](const GeneralEnclosed &generalEnclosed)
      {
        builder += generalEnclosed.Name;
        builder += u8'(';
        builder += generalEnclosed.Text;
        builder += u8')';
      });
  }

  void Serialise(CSSOMString &builder, const Condition &condition) noexcept
  {
    if (condition.Queries.size() == 1 && condition.LogicalOperator == LogicalOperator::Not)
    {
      builder += u8"not ";
      Serialise(builder, condition.Queries.front());
      return;
    }

    for (auto &query : condition.Queries)
    {
      if (&query != &condition.Queries.front())
      {
        builder += condition.LogicalOperator == LogicalOperator::And ? u8" and " : u8" or ";
      }
      Serialise(builder, query);
    }
  }

  void Serialise(CSSOMString &builder, const Feature &feature) noexcept
  {
    auto serialiseRangeComparisonOperator = [&](ComparisonOperator op)
    {
      builder += u8' ';
      switch (op)
      {
        case ComparisonOperator::LessThan:           builder += u8'<'; break;
        case ComparisonOperator::LessThanOrEqual:    builder += u8"<="; break;
        case ComparisonOperator::Equal:              builder += u8'='; break;
        case ComparisonOperator::GreaterThan:        builder += u8'>'; break;
        case ComparisonOperator::GreaterThanOrEqual: builder += u8">="; break;
      }
      builder += u8' ';
    };

    CSSOMString name {feature.Name.View()};

    switch (feature.Syntax)
    {
      case Syntax::Boolean: SerialiseIdentifier(name, builder); break;

      case Syntax::Plain:
        switch (feature.RightComparison->Op)
        {
          case ComparisonOperator::LessThanOrEqual:    builder += u8"max-"; break;
          case ComparisonOperator::Equal:              break;
          case ComparisonOperator::GreaterThanOrEqual: builder += u8"min-"; break;
          case ComparisonOperator::LessThan:
          case ComparisonOperator::GreaterThan:
            krys_debug_assert_msg(false, "Plain features only use <=, = and >=");
            break;
        }
        SerialiseIdentifier(name, builder);

        builder += u8": ";
        builder += feature.RightComparison->ProtectedValue()->CssText(DefaultSerialisationContext());
        break;

      case Syntax::Range:
        if (feature.LeftComparison)
        {
          builder += feature.LeftComparison->ProtectedValue()->CssText(DefaultSerialisationContext());
          serialiseRangeComparisonOperator(feature.LeftComparison->Op);
        }

        SerialiseIdentifier(name, builder);

        if (feature.RightComparison)
        {
          serialiseRangeComparisonOperator(feature.RightComparison->Op);
          builder += feature.RightComparison->ProtectedValue()->CssText(DefaultSerialisationContext());
        }
        break;
    }
  }
}
