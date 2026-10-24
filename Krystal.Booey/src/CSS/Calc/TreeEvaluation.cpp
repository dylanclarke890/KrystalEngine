#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/Executor.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeMappings.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::css::calc
{
  static auto Evaluate(const keywords::None &, const EvaluationOptions &) noexcept -> Maybe<keywords::None>;
  static auto Evaluate(const ChildOrNone &, const EvaluationOptions &) noexcept
    -> Maybe<Variant<double, keywords::None>>;
  static auto Evaluate(const Maybe<Child> &, const EvaluationOptions &) noexcept -> Maybe<Maybe<double>>;
  static auto Evaluate(const Child &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const Number &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const Percentage &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const CanonicalDimension &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const NonCanonicalDimension &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const Symbol &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const SiblingCount &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const SiblingIndex &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Sum> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Product> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Min> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Max> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Hypot> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Random> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<Anchor> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  static auto Evaluate(const IndirectNode<AnchorSize> &, const EvaluationOptions &) noexcept -> Maybe<double>;
  template <typename Op>
  static auto Evaluate(const IndirectNode<Op> &, const EvaluationOptions &) noexcept -> Maybe<double>;

  // MARK: Evaluation.

  template <typename Op, typename... Args>
  static Maybe<double> ExecuteMathOperationAfterUnwrapping(Args &&...args) noexcept
  {
    if ((!args.has_value() || ...))
    {
      return null;
    }

    return ExecuteOperation<ToCalculationTreeOp<Op>::Op>(args.value()...);
  }

  template <typename Op>
  static Maybe<double> ExecuteVariadicMathOperationAfterUnwrapping(const IndirectNode<Op> &op,
                                                                   const EvaluationOptions &options) noexcept
  {
    bool failure = false;
    auto result = ExecuteOperation<ToCalculationTreeOp<Op>::Op>(op->Children.Value,
                                                                [&](const auto &child) -> double
                                                                {
                                                                  if (auto value = Evaluate(child, options))
                                                                  {
                                                                    return *value;
                                                                  }
                                                                  failure = true;
                                                                  return Limits<double>::quiet_NaN();
                                                                });

    if (failure)
    {
      return null;
    }

    return result;
  }

  Maybe<keywords::None> Evaluate(const keywords::None &none, const EvaluationOptions &) noexcept
  {
    return none;
  }

  Maybe<Variant<double, keywords::None>> Evaluate(const ChildOrNone &root,
                                                  const EvaluationOptions &options) noexcept
  {
    return krys::SwitchOn(root,
                          [&](const auto &root) -> Maybe<Variant<double, keywords::None>>
                          {
                            if (auto value = Evaluate(root, options))
                            {
                              return Variant<double, keywords::None> {*value};
                            }
                            return null;
                          });
  }

  Maybe<double> Evaluate(const Child &root, const EvaluationOptions &options) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root) { return Evaluate(root, options); });
  }

  Maybe<Maybe<double>> Evaluate(const Maybe<Child> &root, const EvaluationOptions &options) noexcept
  {
    if (root)
    {
      return Maybe<double> {Evaluate(*root, options)};
    }
    return Maybe<double> {null};
  }

  Maybe<double> Evaluate(const Number &number, const EvaluationOptions &) noexcept
  {
    return number.Value;
  }

  Maybe<double> Evaluate(const Percentage &percentage, const EvaluationOptions &) noexcept
  {
    return percentage.Value;
  }

  Maybe<double> Evaluate(const CanonicalDimension &root, const EvaluationOptions &) noexcept
  {
    return root.Value;
  }

  Maybe<double> Evaluate(const NonCanonicalDimension &root, const EvaluationOptions &options) noexcept
  {
    if (auto canonical = Canonicalize(root, options.ConversionData))
    {
      return Evaluate(*canonical, options);
    }

    return null;
  }

  Maybe<double> Evaluate(const Symbol &root, const EvaluationOptions &options) noexcept
  {
    if (auto value = options.SymbolTable.Get(root.Id))
    {
      return Evaluate(CreateNumeric(value->Value, root.Unit), options);
    }

    krys_debug_assert_msg(false, "Unresolved symbol.");
    return null;
  }

  Maybe<double> Evaluate(const SiblingCount &, const EvaluationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }
    if (!options.ConversionData->StyleBuilderState()->GetElement())
    {
      return null;
    }

    return options.ConversionData->ProtectedStyleBuilderState()->SiblingCount();
  }

  Maybe<double> Evaluate(const SiblingIndex &, const EvaluationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }
    if (!options.ConversionData->StyleBuilderState()->GetElement())
    {
      return null;
    }

    return options.ConversionData->ProtectedStyleBuilderState()->SiblingIndex();
  }

  Maybe<double> Evaluate(const IndirectNode<Sum> &root, const EvaluationOptions &options) noexcept
  {
    return ExecuteVariadicMathOperationAfterUnwrapping(root, options);
  }

  Maybe<double> Evaluate(const IndirectNode<Product> &root, const EvaluationOptions &options) noexcept
  {
    return ExecuteVariadicMathOperationAfterUnwrapping(root, options);
  }

  Maybe<double> Evaluate(const IndirectNode<Min> &root, const EvaluationOptions &options) noexcept
  {
    return ExecuteVariadicMathOperationAfterUnwrapping(root, options);
  }

  Maybe<double> Evaluate(const IndirectNode<Max> &root, const EvaluationOptions &options) noexcept
  {
    return ExecuteVariadicMathOperationAfterUnwrapping(root, options);
  }

  Maybe<double> Evaluate(const IndirectNode<Hypot> &root, const EvaluationOptions &options) noexcept
  {
    return ExecuteVariadicMathOperationAfterUnwrapping(root, options);
  }

  Maybe<double> Evaluate(const IndirectNode<Random> &root, const EvaluationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }

    auto min = Evaluate(root->Min, options);
    if (!min)
    {
      return null;
    }

    auto max = Evaluate(root->Max, options);
    if (!max)
    {
      return null;
    }

    auto step = Evaluate(root->Step, options);
    if (!step)
    {
      return null;
    }

    auto randomBaseValue = krys::SwitchOn(
      root->Sharing,
      [&](const Random::SharingOptions &sharingOptions) -> Maybe<double>
      {
        if (!sharingOptions.ElementShared.has_value()
            && !options.ConversionData->StyleBuilderState()->GetElement())
        {
          return null;
        }

        return options.ConversionData->ProtectedStyleBuilderState()->LookupCSSRandomBaseValue(
          RandomCachingKey {sharingOptions.Identifier}, sharingOptions.ElementShared);
      },
      [&](const Random::SharingFixed &sharingFixed) -> Maybe<double>
      {
        return sharingFixed.Value.SwitchOn(
          [&](const css::Number<css::ClosedUnitRange>::Raw &raw) -> Maybe<double> { return raw.Value; },
          [&](const css::Number<css::ClosedUnitRange>::Calc &calc) -> Maybe<double>
          {
            return calc.Evaluate(css::Category::Number,
                                 *options.ConversionData->ProtectedStyleBuilderState());
          });
      });
    if (!randomBaseValue)
    {
      return null;
    }

    return ExecuteOperation<ToCalculationTreeOp<Random>::Op>(*randomBaseValue, *min, *max, *step);
  }

  Maybe<double> Evaluate(const IndirectNode<Anchor> &anchor, const EvaluationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }

    auto result = EvaluateWithoutFallback(*anchor, options);

    // https://drafts.csswg.org/css-anchor-position-1/#anchor-valid
    // "If any of these conditions are false, the anchor() function resolves to its specified fallback value.
    // If no fallback value is specified, it makes the declaration referencing it invalid at computed-value
    // time."
    if (!result && anchor->Fallback)
    {
      result = Evaluate(*anchor->Fallback, options);
    }

    if (!result)
    {
      options.ConversionData->ProtectedStyleBuilderState()->SetCurrentPropertyInvalidAtComputedValueTime();
    }

    return result;
  }

  Maybe<double> Evaluate(const IndirectNode<AnchorSize> &anchorSize,
                         const EvaluationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }

    auto builderState = options.ConversionData->ProtectedStyleBuilderState();

    Maybe<style::ScopedName> anchorSizeScopedName;
    if (anchorSize->ElementName != CSSOMStringAtom::Null())
    {
      anchorSizeScopedName = style::ScopedName {.Name = anchorSize->ElementName,
                                                .ScopeOrdinal = builderState->StyleScopeOrdinal()};
    }

    auto result = style::AnchorPositionEvaluator::EvaluateSize(*builderState, anchorSizeScopedName,
                                                               anchorSize->Dimension);

    if (!result && anchorSize->Fallback)
    {
      result = Evaluate(*anchorSize->Fallback, options);
    }

    if (!result)
    {
      options.ConversionData->ProtectedStyleBuilderState()->SetCurrentPropertyInvalidAtComputedValueTime();
    }

    return result;
  }

  template <typename Op>
  Maybe<double> Evaluate(const IndirectNode<Op> &root, const EvaluationOptions &options) noexcept
  {
    return krys::apply([&](const auto &...x)
                       { return ExecuteMathOperationAfterUnwrapping<Op>(Evaluate(x, options)...); }, *root);
  }

  Maybe<double> EvaluateDouble(const Tree &tree, const EvaluationOptions &options) noexcept
  {
    return Evaluate(tree.Root, options);
  }

  Maybe<double> EvaluateWithoutFallback(const Anchor &anchor, const EvaluationOptions &options) noexcept
  {
    auto builderState = options.ConversionData->ProtectedStyleBuilderState();

    auto side = krys::SwitchOn(
      anchor.Side, [&](const Child &percentage) -> style::AnchorPositionEvaluator::Side
      { return Evaluate(percentage, options).value_or(0) / 100; },
      [&](ValueId sideId) -> style::AnchorPositionEvaluator::Side { return sideId; });

    Maybe<style::ScopedName> anchorScopedName;
    if (anchor.ElementName != CSSOMStringAtom::Null())
    {
      anchorScopedName =
        style::ScopedName {.Name = anchor.ElementName, .ScopeOrdinal = builderState->StyleScopeOrdinal()};
    }

    return style::AnchorPositionEvaluator::Evaluate(*builderState, anchorScopedName, side);
  }
}
