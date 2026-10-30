#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/Executor.hpp"
#include "Krystal.Booey/Style/Calc/Tree.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Core/Utils/Apply.hpp"

namespace krys::boo::style::calc
{
  static auto Evaluate(const css::keywords::None &, double percentResolutionLength,
                       const ZoomFactor &) noexcept -> css::keywords::None;
  static auto Evaluate(const ChildOrNone &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> Variant<double, css::keywords::None>;
  static auto Evaluate(const Maybe<Child> &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> Maybe<double>;
  static auto Evaluate(const Child &, double percentResolutionLength, const ZoomFactor &) noexcept -> double;
  static auto Evaluate(const Number &, double percentResolutionLength, const ZoomFactor &) noexcept -> double;
  static auto Evaluate(const Percentage &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;
  static auto Evaluate(const Dimension &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;
  static auto Evaluate(const IndirectNode<Sum> &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;
  static auto Evaluate(const IndirectNode<Product> &, double percentResolutionLength,
                       const ZoomFactor &) noexcept -> double;
  static auto Evaluate(const IndirectNode<Min> &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;
  static auto Evaluate(const IndirectNode<Max> &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;
  static auto Evaluate(const IndirectNode<Hypot> &, double percentResolutionLength,
                       const ZoomFactor &) noexcept -> double;
  static auto Evaluate(const IndirectNode<Random> &, double percentResolutionLength,
                       const ZoomFactor &) noexcept -> double;
  static auto Evaluate(const IndirectNode<Blend> &, double percentResolutionLength,
                       const ZoomFactor &) noexcept -> double;
  template <typename Op>
  static auto Evaluate(const IndirectNode<Op> &, double percentResolutionLength, const ZoomFactor &) noexcept
    -> double;

  template <typename Op, typename... Args>
  static double ExecuteMathOperation(Args &&...args) noexcept
  {
    return css::calc::ExecuteOperation<Op::Op>(std::forward<Args>(args)...);
  }

  // MARK: Evaluation.

  css::keywords::None Evaluate(const css::keywords::None &root, double, const ZoomFactor &) noexcept
  {
    return root;
  }

  Variant<double, css::keywords::None> Evaluate(const ChildOrNone &root, double percentResolutionLength,
                                                const ZoomFactor &usedZoom) noexcept
  {
    return krys::SwitchOn(
      root, [&](const auto &root)
      { return Variant<double, css::keywords::None> {Evaluate(root, percentResolutionLength, usedZoom)}; });
  }

  double Evaluate(const Child &root, double percentResolutionLength, const ZoomFactor &usedZoom) noexcept
  {
    return krys::SwitchOn(root, [&](const auto &root)
                          { return Evaluate(root, percentResolutionLength, usedZoom); });
  }

  Maybe<double> Evaluate(const Maybe<Child> &root, double percentResolutionLength,
                         const ZoomFactor &usedZoom) noexcept
  {
    if (root)
    {
      return static_cast<double>(Evaluate(*root, percentResolutionLength, usedZoom));
    }
    return null;
  }

  double Evaluate(const Number &number, double, const ZoomFactor &usedZoom) noexcept
  {
    return number.Value * usedZoom.Value;
  }

  double Evaluate(const Percentage &percentage, double percentResolutionLength, const ZoomFactor &) noexcept
  {
    return percentResolutionLength * percentage.Value / 100.0;
  }

  double Evaluate(const Dimension &root, double, const ZoomFactor &usedZoom) noexcept
  {
    return root.Value * usedZoom.Value;
  }

  double Evaluate(const IndirectNode<Sum> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return ExecuteMathOperation<Sum>(root->Children.Value, [&](const auto &child) -> double
                                     { return Evaluate(child, percentResolutionLength, usedZoom); });
  }

  double Evaluate(const IndirectNode<Product> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return ExecuteMathOperation<Product>(root->Children.Value, [&](const auto &child) -> double
                                         { return Evaluate(child, percentResolutionLength, usedZoom); });
  }

  double Evaluate(const IndirectNode<Min> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return ExecuteMathOperation<Min>(root->Children.Value, [&](const auto &child) -> double
                                     { return Evaluate(child, percentResolutionLength, usedZoom); });
  }

  double Evaluate(const IndirectNode<Max> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return ExecuteMathOperation<Max>(root->Children.Value, [&](const auto &child) -> double
                                     { return Evaluate(child, percentResolutionLength, usedZoom); });
  }

  double Evaluate(const IndirectNode<Hypot> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return ExecuteMathOperation<Hypot>(root->Children.Value, [&](const auto &child) -> double
                                       { return Evaluate(child, percentResolutionLength, usedZoom); });
  }

  double Evaluate(const IndirectNode<Random> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    auto min = Evaluate(root->Min, percentResolutionLength, usedZoom);
    auto max = Evaluate(root->Max, percentResolutionLength, usedZoom);
    auto step = Evaluate(root->Step, percentResolutionLength, usedZoom);

    return ExecuteMathOperation<Random>(root->FixedValue.BaseValue, min, max, step);
  }

  double Evaluate(const IndirectNode<Blend> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return (1.0 - root->Progress) * Evaluate(root->From, percentResolutionLength, usedZoom)
           + root->Progress * Evaluate(root->To, percentResolutionLength, usedZoom);
  }

  template <typename Op>
  double Evaluate(const IndirectNode<Op> &root, double percentResolutionLength,
                  const ZoomFactor &usedZoom) noexcept
  {
    return krys::apply(
      [&](const auto &...x)
      { return ExecuteMathOperation<Op>(Evaluate(x, percentResolutionLength, usedZoom)...); }, *root);
  }

  double Evaluate(const Tree &tree, double percentResolutionLength, const ZoomFactor &zoomFactor) noexcept
  {
    return Evaluate(tree.Root, percentResolutionLength, zoomFactor);
  }

  double Evaluate(const Tree &tree, double percentResolutionLength, const ZoomNeeded &) noexcept
  {
    return Evaluate(tree.Root, percentResolutionLength, ZoomFactor {1.0f, 1.0f});
  }
}
