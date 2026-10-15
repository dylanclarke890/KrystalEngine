#include "Krystal.Booey/Style/Calc/TreeConversion.hpp"
#include "Krystal.Booey/CSS/Calc/Executor.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/TreeMappings.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::style::calc
{
  struct ToCSSConversionOptions
  {
    css::calc::Dimension CanonicalDimension;
    css::calc::SimplificationOptions Simplification;
    const RenderStyle &Style;
  };

  struct ToStyleConversionOptions
  {
    css::calc::EvaluationOptions Evaluation;
  };

  static auto ToCSS(const Random::Fixed &, const ToCSSConversionOptions &) noexcept
    -> css::calc::Random::SharingType;
  static auto ToCSS(const css::keywords::None &, const ToCSSConversionOptions &) noexcept
    -> css::keywords::None;
  static auto ToCSS(const ChildOrNone &, const ToCSSConversionOptions &) noexcept -> css::calc::ChildOrNone;
  static auto ToCSS(const Children &, const ToCSSConversionOptions &) noexcept -> css::calc::Children;
  static auto ToCSS(const Maybe<Child> &, const ToCSSConversionOptions &) noexcept -> Maybe<css::calc::Child>;
  static auto ToCSS(const Child &, const ToCSSConversionOptions &) noexcept -> css::calc::Child;
  static auto ToCSS(const Number &, const ToCSSConversionOptions &) noexcept -> css::calc::Child;
  static auto ToCSS(const Percentage &, const ToCSSConversionOptions &) noexcept -> css::calc::Child;
  static auto ToCSS(const Dimension &, const ToCSSConversionOptions &) noexcept -> css::calc::Child;
  static auto ToCSS(const IndirectNode<Blend> &, const ToCSSConversionOptions &) noexcept -> css::calc::Child;
  template <typename CalculationOp>
  static auto ToCSS(const IndirectNode<CalculationOp> &, const ToCSSConversionOptions &) noexcept
    -> css::calc::Child;

  static auto ToStyle(const css::calc::Random::SharingType &, const ToStyleConversionOptions &) noexcept
    -> Random::Fixed;
  static auto ToStyle(const Maybe<css::calc::Child> &, const ToStyleConversionOptions &) noexcept
    -> Maybe<Child>;
  static auto ToStyle(const css::keywords::None &, const ToStyleConversionOptions &) noexcept
    -> css::keywords::None;
  static auto ToStyle(const css::calc::ChildOrNone &, const ToStyleConversionOptions &) noexcept
    -> ChildOrNone;
  static auto ToStyle(const css::calc::Children &, const ToStyleConversionOptions &) noexcept -> Children;
  static auto ToStyle(const css::calc::Child &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::Number &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::Percentage &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::CanonicalDimension &, const ToStyleConversionOptions &) noexcept
    -> Child;
  static auto ToStyle(const css::calc::NonCanonicalDimension &, const ToStyleConversionOptions &) noexcept
    -> Child;
  static auto ToStyle(const css::calc::Symbol &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::SiblingCount &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::SiblingIndex &, const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::IndirectNode<css::calc::Anchor> &,
                      const ToStyleConversionOptions &) noexcept -> Child;
  static auto ToStyle(const css::calc::IndirectNode<css::calc::AnchorSize> &,
                      const ToStyleConversionOptions &) noexcept -> Child;
  template <typename Op>
  static auto ToStyle(const css::calc::IndirectNode<Op> &, const ToStyleConversionOptions &) noexcept
    -> Child;

  static css::calc::Dimension DetermineCanonicalDimension(css::Category category) noexcept
  {
    switch (category)
    {
      case css::Category::LengthPercentage: return css::calc::Dimension::Length;
      case css::Category::AnglePercentage:  return css::calc::Dimension::Angle;

      case css::Category::Integer:
      case css::Category::Number:
      case css::Category::Percentage:
      case css::Category::Length:
      case css::Category::Angle:
      case css::Category::Time:
      case css::Category::Frequency:
      case css::Category::Resolution:
      case css::Category::Flex:
      {
        break;
      }
    }

    krys_debug_assert_msg(false, "Only <length-percentage> and <angle-percentage> have a style::calc::Tree.");
    return css::calc::Dimension::Length;
  }

  // MARK: - From

  css::calc::Random::SharingType ToCSS(const Random::Fixed &randomFixed,
                                       const ToCSSConversionOptions &) noexcept
  {
    return css::calc::Random::SharingFixed {css::Number<css::ClosedUnitRange> {randomFixed.BaseValue}};
  }

  css::keywords::None ToCSS(const css::keywords::None &none, const ToCSSConversionOptions &) noexcept
  {
    return none;
  }

  css::calc::ChildOrNone ToCSS(const ChildOrNone &root, const ToCSSConversionOptions &options) noexcept
  {
    return krys::SwitchOn(root,
                          [&](const auto &root) { return css::calc::ChildOrNone {ToCSS(root, options)}; });
  }

  css::calc::Children ToCSS(const Children &children, const ToCSSConversionOptions &options) noexcept
  {
    List<css::calc::Child> result;
    result.reserve(children.size());
    for (const auto &child : children)
    {
      result.push_back(ToCSS(child, options));
    }
    return result;
  }

  Maybe<css::calc::Child> ToCSS(const Maybe<Child> &root, const ToCSSConversionOptions &options) noexcept
  {
    if (root)
    {
      return ToCSS(*root, options);
    }
    return null;
  }

  css::calc::Child ToCSS(const Child &root, const ToCSSConversionOptions &options) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return ToCSS(root, options); });
  }

  css::calc::Child ToCSS(const Number &number, const ToCSSConversionOptions &) noexcept
  {
    return css::calc::CreateChild(css::calc::Number {.Value = number.Value});
  }

  css::calc::Child ToCSS(const Percentage &percentage, const ToCSSConversionOptions &options) noexcept
  {
    return css::calc::CreateChild(
      css::calc::Percentage {.Value = percentage.Value,
                             .Hint = css::calc::Type::DeterminePercentHint(options.Simplification.Category)});
  }

  css::calc::Child ToCSS(const Dimension &root, const ToCSSConversionOptions &options) noexcept
  {
    switch (options.CanonicalDimension)
    {
      case css::calc::Dimension::Length:
      {
        return css::calc::CreateChild(css::calc::CanonicalDimension {
          .Value = AdjustFloatForAbsoluteZoom(static_cast<float>(root.Value), options.Style),
          .Dimension = options.CanonicalDimension});
      }

      case css::calc::Dimension::Angle:
      case css::calc::Dimension::Time:
      case css::calc::Dimension::Frequency:
      case css::calc::Dimension::Resolution:
      case css::calc::Dimension::Flex:
      {
        break;
      }
    }

    return css::calc::CreateChild(
      css::calc::CanonicalDimension {.Value = root.Value, .Dimension = options.CanonicalDimension});
  }

  css::calc::Child ToCSS(const IndirectNode<Blend> &root, const ToCSSConversionOptions &options) noexcept
  {
    // FIXME: (http://webkit.org/b/122036) Create a css::calc::Tree equivalent of Blend.

    auto createBlendHalf = [](const auto &child, const auto &options, auto progress) -> css::calc::Child
    {
      auto product = css::calc::Multiply(ToCSS(child, options),
                                         css::calc::CreateChild(css::calc::Number {.Value = progress}));

      if (auto replacement = css::calc::Simplify(product, options.Simplification))
      {
        return krys::move(*replacement);
      }

      auto type = css::calc::ToType(product);
      return css::calc::CreateChild(krys::move(product), *type);
    };

    auto sum = css::calc::Add(createBlendHalf(root->From, options, 1 - root->Progress),
                              createBlendHalf(root->To, options, root->Progress));

    if (auto replacement = css::calc::Simplify(sum, options.Simplification))
    {
      return krys::move(*replacement);
    }

    auto type = css::calc::ToType(sum);
    return css::calc::CreateChild(krys::move(sum), *type);
  }

  template <typename CalculationOp>
  css::calc::Child ToCSS(const IndirectNode<CalculationOp> &root,
                         const ToCSSConversionOptions &options) noexcept
  {
    using CalcOp = css::calc::ToCalcTreeOp<CalculationOp>;

    auto op = krys::apply([&](const auto &...x) { return CalcOp {ToCSS(x, options)...}; }, *root);

    if (auto replacement = css::calc::Simplify(op, options.Simplification))
    {
      return krys::move(*replacement);
    }

    auto type = css::calc::ToType(op);
    return css::calc::CreateChild(krys::move(op), *type);
  }

  // MARK: - To.

  Random::Fixed ToStyle(const css::calc::Random::SharingType &randomSharing,
                        const ToStyleConversionOptions &options) noexcept
  {
    krys_debug_assert(options.Evaluation.ConversionData);
    krys_debug_assert(options.Evaluation.ConversionData->StyleBuilderState());

    return krys::SwitchOn(
      randomSharing,
      [&](const css::calc::Random::SharingOptions &sharingOptions) -> Random::Fixed
      {
        if (!sharingOptions.ElementShared.has_value())
        {
          krys_debug_assert(options.Evaluation.ConversionData->StyleBuilderState()->GetElement());
        }

        auto baseValue =
          options.Evaluation.ConversionData->ProtectedStyleBuilderState()->LookupCSSRandomBaseValue(
            css::calc::RandomCachingKey {sharingOptions.Identifier}, sharingOptions.ElementShared);

        return Random::Fixed {baseValue};
      },
      [&](const css::calc::Random::SharingFixed &sharingFixed) -> Random::Fixed
      {
        return sharingFixed.Value.SwitchOn(
          [&](const css::Number<css::ClosedUnitRange>::Raw &raw) -> Random::Fixed
          { return Random::Fixed {raw.Value}; },
          [&](const css::Number<css::ClosedUnitRange>::Calc &calc) -> Random::Fixed
          {
            return Random::Fixed {calc.Evaluate(
              css::Category::Number, *options.Evaluation.ConversionData->ProtectedStyleBuilderState())};
          });
      });
  }

  Maybe<Child> ToStyle(const Maybe<css::calc::Child> &optionalChild,
                       const ToStyleConversionOptions &options) noexcept
  {
    if (optionalChild)
    {
      return ToStyle(*optionalChild, options);
    }
    return null;
  }

  css::keywords::None ToStyle(const css::keywords::None &none, const ToStyleConversionOptions &) noexcept
  {
    return none;
  }

  ChildOrNone ToStyle(const css::calc::ChildOrNone &root, const ToStyleConversionOptions &options) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return ChildOrNone {ToStyle(root, options)}; });
  }

  Children ToStyle(const css::calc::Children &children, const ToStyleConversionOptions &options) noexcept
  {
    List<Child> result;
    result.reserve(children.size());
    for (const auto &child : children)
    {
      result.push_back(ToStyle(child, options));
    }
    return result;
  }

  Child ToStyle(const css::calc::Child &root, const ToStyleConversionOptions &options) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return ToStyle(root, options); });
  }

  Child ToStyle(const css::calc::Number &root, const ToStyleConversionOptions &) noexcept
  {
    return CreateNumber(root.Value);
  }

  Child ToStyle(const css::calc::Percentage &root, const ToStyleConversionOptions &) noexcept
  {
    return CreatePercentage(root.Value);
  }

  Child ToStyle(const css::calc::CanonicalDimension &root, const ToStyleConversionOptions &options) noexcept
  {
    switch (root.Dimension)
    {
      case css::calc::Dimension::Length:
      {
        krys_debug_assert(options.Evaluation.ConversionData);
        return CreateDimension(
          ComputeNonCalcLengthDouble(root.Value, css::LengthUnit::Px, *options.Evaluation.ConversionData));
      }

      case css::calc::Dimension::Angle:
      case css::calc::Dimension::Time:
      case css::calc::Dimension::Frequency:
      case css::calc::Dimension::Resolution:
      case css::calc::Dimension::Flex:
      {
        break;
      }
    }

    return CreateDimension(root.Value);
  }

  Child ToStyle(const css::calc::NonCanonicalDimension &, const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Non-canonical numeric values are not supported in the Tree");
    return CreateNumber(0);
  }

  Child ToStyle(const css::calc::Symbol &, const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Unevaluated symbols are not supported in the Tree");
    return CreateNumber(0);
  }

  Child ToStyle(const css::calc::SiblingCount &, const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Unevaluated sibling-count() functions are not supported in the Tree");
    return CreateNumber(0);
  }

  Child ToStyle(const css::calc::SiblingIndex &, const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Unevaluated sibling-index() functions are not supported in the Tree");
    return CreateNumber(0);
  }

  Child ToStyle(const css::calc::IndirectNode<css::calc::Anchor> &, const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Unevaluated anchor() functions are not supported in the Tree");
    return CreateNumber(0);
  }

  Child ToStyle(const css::calc::IndirectNode<css::calc::AnchorSize> &,
                const ToStyleConversionOptions &) noexcept
  {
    krys_debug_assert_msg(false, "Unevaluated anchor-size() functions are not supported in the Tree");
    return CreateNumber(0);
  }

  template <typename Op>
  Child ToStyle(const css::calc::IndirectNode<Op> &root, const ToStyleConversionOptions &options) noexcept
  {
    using CalculationOp = css::calc::ToCalculationTreeOp<Op>;

    return CreateChild(
      krys::apply([&](const auto &...x) { return CalculationOp {ToStyle(x, options)...}; }, *root));
  }

  // MARK: - Exposed functions

  css::calc::Tree ToCSS(const Tree &tree, const ToCSSOptions &toCSSOptions) noexcept
  {
    auto conversionOptions = ToCSSConversionOptions {
      .CanonicalDimension = DetermineCanonicalDimension(toCSSOptions.Category),
      .Simplification =
        css::calc::SimplificationOptions {
          .Category = toCSSOptions.Category,
          .Range = toCSSOptions.Range,
          .ConversionData = null,
          .SymbolTable = {},
          .AllowZeroValueLengthRemovalFromSum = true,
        },
      .Style = toCSSOptions.Style,
    };

    auto root = ToCSS(tree.Root, conversionOptions);
    auto type = css::calc::GetType(root);

    return css::calc::Tree {
      .Root = krys::move(root),
      .Type = type,
      .Stage = css::calc::Stage::Computed,
    };
  }

  Tree ToStyle(const css::calc::Tree &tree, const ToStyleOptions &toStyleOptions) noexcept
  {
    krys_debug_assert(toStyleOptions.Category == css::Category::LengthPercentage
                      || toStyleOptions.Category == css::Category::AnglePercentage);

    auto simplificationOptions = css::calc::SimplificationOptions {
      .Category = toStyleOptions.Category,
      .Range = toStyleOptions.Range,
      .ConversionData = toStyleOptions.ConversionData,
      .SymbolTable = toStyleOptions.SymbolTable,
      .AllowZeroValueLengthRemovalFromSum = true,
    };
    auto simplifiedTree = css::calc::CopyAndSimplify(tree, simplificationOptions);

    auto conversionOptions = ToStyleConversionOptions {.Evaluation = css::calc::EvaluationOptions {
                                                         .Category = toStyleOptions.Category,
                                                         .Range = toStyleOptions.Range,
                                                         .ConversionData = toStyleOptions.ConversionData,
                                                         .SymbolTable = toStyleOptions.SymbolTable,
                                                       }};
    return Tree {ToStyle(simplifiedTree.Root, conversionOptions)};
  }
}
