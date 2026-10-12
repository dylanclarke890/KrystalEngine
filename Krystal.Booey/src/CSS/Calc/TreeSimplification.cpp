#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Calc/Executor.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKey.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeCopy.hpp"
#include "Krystal.Booey/CSS/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/CSS/Calc/TreeMappings.hpp"
#include "Krystal.Booey/CSS/Calc/TreeNumericIdentity.hpp"
#include "Krystal.Booey/CSS/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Utils/Apply.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <algorithm>
#include <array>
#include <utility>

namespace krys::boo::css::calc
{
  static auto CopyAndSimplify(const Random::SharingType &, const SimplificationOptions &) noexcept
    -> Random::SharingType;
  static auto CopyAndSimplify(const keywords::None &, const SimplificationOptions &) noexcept
    -> keywords::None;
  static auto CopyAndSimplify(const Children &, const SimplificationOptions &) noexcept -> Children;
  static auto CopyAndSimplify(const ChildOrNone &, const SimplificationOptions &) noexcept -> ChildOrNone;
  template <typename T>
  static auto CopyAndSimplify(const Maybe<T> &, const SimplificationOptions &) noexcept -> Maybe<T>;

  template <typename Op, typename... Args>
  static double ExecuteMathOperation(Args &&...args) noexcept
  {
    return ExecuteOperation<ToCalculationTreeOp<Op>::Op>(std::forward<Args>(args)...);
  }

  template <typename... F>
  static decltype(auto) SwitchTogether(const Child &a, const Child &b, F &&...f) noexcept
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
    using ResultType = decltype(visitor(std::declval<Number>(), std::declval<Number>()));

    if (a.index() != b.index())
    {
      return visitor(null, null);
    }

    return krys::SwitchOn(a, [&]<typename T>(const T &aT) -> ResultType { return visitor(aT, get<T>(b)); });
  }

  // MARK: Predicate: PercentageResolveToDimension

  static bool PercentageResolveToDimension(const SimplificationOptions &options) noexcept
  {
    switch (options.Category)
    {
      case css::Category::Integer:
      case css::Category::Number:
      case css::Category::Length:
      case css::Category::Percentage:
      case css::Category::Angle:
      case css::Category::Time:
      case css::Category::Frequency:
      case css::Category::Resolution:
      case css::Category::Flex:
      {
        return false;
      }

      case css::Category::AnglePercentage:
      case css::Category::LengthPercentage:
      {
        return true;
      }
    }

    krys_debug_assert_msg(false, "Unknown category.");
    return false;
  }

  // MARK: Predicate: UnitsMatch

  constexpr bool UnitsMatch(const Number &, const Number &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  constexpr bool UnitsMatch(const Percentage &, const Percentage &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  static bool UnitsMatch(const CanonicalDimension &a, const CanonicalDimension &b,
                         const SimplificationOptions &) noexcept
  {
    return a.Dimension == b.Dimension;
  }

  static bool UnitsMatch(const NonCanonicalDimension &a, const NonCanonicalDimension &b,
                         const SimplificationOptions &) noexcept
  {
    return a.Unit == b.Unit;
  }

  // MARK: Predicate: MagnitudeComparable

  constexpr bool MagnitudeComparable(const Number &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  static bool MagnitudeComparable(const Percentage &, const SimplificationOptions &options) noexcept
  {
    return !PercentageResolveToDimension(options);
  }

  constexpr bool MagnitudeComparable(const CanonicalDimension &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  constexpr bool MagnitudeComparable(const NonCanonicalDimension &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  // MARK: Predicate: FullyResolved

  constexpr bool FullyResolved(const Number &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  static bool FullyResolved(const Percentage &, const SimplificationOptions &options) noexcept
  {
    return !PercentageResolveToDimension(options);
  }

  constexpr bool FullyResolved(const CanonicalDimension &, const SimplificationOptions &) noexcept
  {
    return true;
  }

  constexpr bool FullyResolved(const NonCanonicalDimension &, const SimplificationOptions &) noexcept
  {
    return false;
  }

  Maybe<CanonicalDimension> Canonicalize(NonCanonicalDimension root,
                                         const Maybe<ToLengthConversionData> &conversionData) noexcept
  {
    auto makeCanonical = [&](double value, calc::Dimension dimension) -> Maybe<CanonicalDimension>
    {
      return CanonicalDimension {.Value = value, .Dimension = dimension};
    };

    auto tryMakeCanonical = [&](double value, LengthUnit lengthUnit) -> Maybe<CanonicalDimension>
    {
      if (conversionData)
      {
        return CanonicalDimension {
          .Value = style::ComputeCanonicalNonCalcLengthDouble(value, lengthUnit, *conversionData),
          .Dimension = calc::Dimension::Length};
      }
      return null;
    };

    switch (root.Unit)
    {
      // Absolute Lengths (can be canonicalized without conversion data).
      case UnitType::cm: return makeCanonical(root.Value * pxPerCm, calc::Dimension::Length);
      case UnitType::mm: return makeCanonical(root.Value * pxPerMm, calc::Dimension::Length);
      case UnitType::Q:  return makeCanonical(root.Value * pxPerQ, calc::Dimension::Length);
      case UnitType::in: return makeCanonical(root.Value * pxPerInch, calc::Dimension::Length);
      case UnitType::pt: return makeCanonical(root.Value * pxPerPt, calc::Dimension::Length);
      case UnitType::pc: return makeCanonical(root.Value * pxPerPc, calc::Dimension::Length);

      // Font, Viewport and Container relative Lengths (require conversion data for canonicalization).
      case UnitType::em:
      case UnitType::ex:
      case UnitType::lh:
      case UnitType::cap:
      case UnitType::ch:
      case UnitType::ic:
      case UnitType::rcap:
      case UnitType::rch:
      case UnitType::rem:
      case UnitType::rex:
      case UnitType::ric:
      case UnitType::rlh:
      case UnitType::vw:
      case UnitType::vh:
      case UnitType::vmin:
      case UnitType::vmax:
      case UnitType::vb:
      case UnitType::vi:
      case UnitType::svw:
      case UnitType::svh:
      case UnitType::svmin:
      case UnitType::svmax:
      case UnitType::svb:
      case UnitType::svi:
      case UnitType::lvw:
      case UnitType::lvh:
      case UnitType::lvmin:
      case UnitType::lvmax:
      case UnitType::lvb:
      case UnitType::lvi:
      case UnitType::dvw:
      case UnitType::dvh:
      case UnitType::dvmin:
      case UnitType::dvmax:
      case UnitType::dvb:
      case UnitType::dvi:
      case UnitType::cqw:
      case UnitType::cqh:
      case UnitType::cqi:
      case UnitType::cqb:
      case UnitType::cqmin:
      case UnitType::cqmax:
      {
        return tryMakeCanonical(root.Value, *ToLengthUnit(root.Unit));
      }

      // <angle>
      case UnitType::rad: return makeCanonical(root.Value * degreesPerRadianDouble, calc::Dimension::Angle);
      case UnitType::grad:
        return makeCanonical(root.Value * degreesPerGradientDouble, calc::Dimension::Angle);
      case UnitType::turn: return makeCanonical(root.Value * degreesPerTurnDouble, calc::Dimension::Angle);

      // <time>
      case UnitType::ms:   return makeCanonical(root.Value * secsPerMs, calc::Dimension::Time);

      // <frequency>
      case UnitType::khz:  return makeCanonical(root.Value * hertzPerKilohertz, calc::Dimension::Frequency);

      // <resolution>
      case UnitType::x:    return makeCanonical(root.Value * dppxPerX, calc::Dimension::Resolution);
      case UnitType::dpi:  return makeCanonical(root.Value * dppxPerDpi, calc::Dimension::Resolution);
      case UnitType::dpcm: return makeCanonical(root.Value * dppxPerDpcm, calc::Dimension::Resolution);

      // Canonical dimensional types should never be stored in a NonCanonicalDimension.
      case UnitType::px:
      case UnitType::deg:
      case UnitType::s:
      case UnitType::hz:
      case UnitType::dppx:
      case UnitType::fr:
      // Non-dimensional types should never be stored in a NonCanonicalDimension.
      case UnitType::Number:
      case UnitType::Integer:
      case UnitType::Percentage:
      // Non-numeric types should never be stored in a NonCanonicalDimension.
      case UnitType::Attr:
      case UnitType::Calc:
      case UnitType::CalcPercentageWithAngle:
      case UnitType::CalcPercentageWithLength:
      case UnitType::Dimension:
      case UnitType::FontFamily:
      case UnitType::Ident:
      case UnitType::PropertyId:
      case UnitType::QuirkyEm:
      case UnitType::String:
      case UnitType::Unknown:
      case UnitType::ValueId:
      case UnitType::CustomIdent:
      {
        break;
      }
    }

    krys_debug_assert_msg(false, "Unexpected unit in a NonCanonicalDimension.");
    return null;
  }

  // MARK: Generic partial evaluation functions

  template <typename Op>
  static Maybe<Child> SimplifyForOperation(Child &a, Child &b, const SimplificationOptions &options) noexcept
  {
    return SwitchTogether(
      a, b,
      [&]<Numeric T>(const T &numericA, const T &numericB) -> Maybe<Child>
      {
        if (!UnitsMatch(numericA, numericB, options) || !FullyResolved(numericA, options))
        {
          return null;
        }

        return CreateChildWithValueBasedOn(ExecuteMathOperation<Op>(numericA.Value, numericB.Value),
                                           numericA);
      },
      [](const auto &, const auto &) -> Maybe<Child> { return null; });
  }

  template <typename Op, typename Completion>
  static Maybe<Child> SimplifyForOperationWithCompletion(Child &a, Child &b,
                                                         const SimplificationOptions &options,
                                                         Completion &&completion) noexcept
  {
    return SwitchTogether(
      a, b,
      [&]<Numeric T>(const T &numericA, const T &numericB) -> Maybe<Child>
      {
        if (!UnitsMatch(numericA, numericB, options) || !FullyResolved(numericA, options))
        {
          return null;
        }

        return completion(ExecuteMathOperation<Op>(numericA.Value, numericB.Value));
      },
      [](const auto &, const auto &) -> Maybe<Child> { return null; });
  }

  template <typename Op>
  static Maybe<Child> SimplifyForRound(Op &root, const SimplificationOptions &options) noexcept
  {
    if (root.B)
    {
      return SimplifyForOperation<Op>(root.A, *root.B, options);
    }

    if (auto *numberA = get_if<Number>(&root.A))
    {
      return CreateChild(Number {.Value = ExecuteMathOperation<Op>(numberA->Value, 1.0)});
    }

    return null;
  }

  template <typename Op>
  static Maybe<Child> SimplifyForTrig(Op &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` has been type checked by this point to be `<number>` or an `<angle>`, though they may not
    // be able to be fully resolved yet. If its an `<angle>`, it is also already been converted to canonical
    // units via earlier simplification.

    return krys::SwitchOn(
      root.A, [&](const Number &a) -> Maybe<Child>
      { return CreateChild(Number {.Value = ExecuteMathOperation<Op>(a.Value)}); },
      [&](const CanonicalDimension &a) -> Maybe<Child>
      {
        krys_debug_assert(a.Dimension == calc::Dimension::Angle);
        return CreateChild(Number {.Value = ExecuteMathOperation<Op>(deg2rad(a.Value))});
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  template <typename Op>
  static Maybe<Child> SimplifyForArcTrig(Op &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` has been type checked by this point to be `<number>`, though they may not
    // be able to be fully resolved yet.

    return krys::SwitchOn(
      root.A,
      [&](const Number &a) -> Maybe<Child>
      {
        return CreateChild(CanonicalDimension {.Value = ExecuteMathOperation<Op>(a.Value),
                                               .Dimension = calc::Dimension::Angle});
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  template <typename Op>
  static Maybe<Child> SimplifyForMinMax(Op &root, const SimplificationOptions &options) noexcept
  {
    krys_debug_assert(!root.Children.empty());

    // This function implements shared logic for Min and Max simplification:

    // 5.1. For each node child of root’s children:
    //      If child is a numeric value with enough information to compare magnitudes with another child of
    //      the same unit (see note in previous step), and there are other children of root that are numeric
    //      values with the same unit, combine all such children with the appropriate operator per root, and
    //      replace child with the result, removing all other child nodes involved.
    // 5.2. If root has only one child, return the child.
    // 5.3. Otherwise, return root.

    // --

    // These steps are implemented as a two phase procedure.
    //    1. Iterate children to find "merge opportunities", counting the total number of merges that will
    //       happen, and storing the index of the first child of each merge type in a lookup table.
    //    2. Perform merges based on data from step 1.
    //
    // By splitting it up, we can perform two optimizations:
    //    1. If the result of step 1 shows that the number of "merge opportunities" will lead to only one
    //       remaining child, we can avoid allocating a new Children list, and just merge directly into the
    //       child.
    //    2. If the result of step 1 shows that the number of "merge opportunities" will lead to more than one
    //       remaining child, we can precisely allocate the Children list to be (existing children - "merge
    //       opportunities").

    auto evaluate = [](const Child &a, const Child &b) -> Child
    {
      krys_debug_assert(a.index() == b.index());

      return krys::SwitchOn(
        a,
        [&]<Numeric T>(const T &aNumeric) -> Child
        {
          krys_debug_assert(ToNumericIdentity(aNumeric) == ToNumericIdentity(get<T>(b)));
          return CreateChildWithValueBasedOn(ExecuteMathOperation<Op>(aNumeric.Value, get<T>(b).Value),
                                             aNumeric);
        },
        [](const auto &) -> Child
        {
          krys_debug_assert_msg(false, "Only numeric children can be merged.");
          return CreateChild(Number {.Value = 0});
        });
    };

    // Special case a root with one child to avoid doing any work at all, and just returning the child.
    if (root.Children.size() == 1)
    {
      return krys::move(root.Children[0]);
    }

    // Map of unit types (via NumericIdentity) to the first index in `root.Children` where a value with that
    // unit can be found. More specifically, it maps the unit to the index + 1, as 0 is used to indicate no
    // units of that type have been found.
    // FIXME: This should be turned into a type with an interface that doesn't require explicit use of
    // static_cast<uint8> by the caller.
    std::array<size_t, NumberOfNumericIdentityTypes> offsetOfFirstInstance {};

    bool canMergePercentages = !PercentageResolveToDimension(options);

    uint32 numberOfMergeOpportunities = 0;
    for (size_t i = 0; i < root.Children.size(); ++i)
    {
      numberOfMergeOpportunities += krys::SwitchOn(
        root.Children[i],
        [&]<Numeric T>(const T &child) -> uint32
        {
          auto id = ToNumericIdentity(child);
          if (id == NumericIdentity::Percentage && !canMergePercentages)
          {
            return 0;
          }

          if (auto offset = offsetOfFirstInstance[static_cast<uint8>(id)])
          {
            // There has already been an instance of this type. This is a merge opportunity.

            // Merge the value into first instance.
            root.Children[offset - 1] = evaluate(root.Children[offset - 1], root.Children[i]);

            // Return 1 to increment the number of merge opportunities observed.
            return 1;
          }

          // First instance of this. Store the index (well, index + 1, since 0 is the unset value).
          offsetOfFirstInstance[static_cast<uint8>(id)] = i + 1;

          // Give this was the first instance, it is not yet a merge opportunity.
          return 0;
        },
        [](const auto &) -> uint32 { return 0; });
    }

    // If there are no merge opportunities, no further simplification is possible.
    if (!numberOfMergeOpportunities)
    {
      return null;
    }

    auto combinedChildrenSize = root.Children.size() - numberOfMergeOpportunities;

    // If all the removal from merges leaves a single child, that means everything merged into the first
    // child.
    if (combinedChildrenSize == 1)
    {
      return krys::move(root.Children[0]);
    }

    List<Child> combinedChildren;
    combinedChildren.reserve(combinedChildrenSize);

    for (size_t i = 0; i < root.Children.size(); ++i)
    {
      krys::SwitchOn(
        root.Children[i],
        [&]<Numeric T>(const T &child)
        {
          auto offset = offsetOfFirstInstance[static_cast<uint8>(ToNumericIdentity(child))];

          // If the stored offset for this type is unset (as it would be for percentages if merging them is
          // disallowed) or is set to this index (as it would be for the first instance of a merged type),
          // append the child as normal.
          if (!offset || (offset - 1) == i)
          {
            combinedChildren.push_back(krys::move(root.Children[i]));
            return;
          }

          // Otherwise, it's one that can be dropped.
        },
        [&](const auto &) { combinedChildren.push_back(krys::move(root.Children[i])); });
    }
    root.Children = krys::move(combinedChildren);

    return null;
  }

  // MARK: In-place simplification / replacement finding.

  Maybe<Child> Simplify(Number &, const SimplificationOptions &) noexcept
  {
    // No further simplification possible for <number>.
    return null;
  }

  Maybe<Child> Simplify(Percentage &, const SimplificationOptions &) noexcept
  {
    // 1.1. If root is a percentage that will be resolved against another value, and there is enough
    // information available to resolve it, do so, and express the resulting numeric value in the appropriate
    // canonical unit. Return the value.
    // NOTE: Handled by the style::calc::Tree / style::calc::Value types at use time.
    return null;
  }

  Maybe<Child> Simplify(CanonicalDimension &, const SimplificationOptions &) noexcept
  {
    // No further simplification possible for canonical <dimension>.
    return null;
  }

  Maybe<Child> Simplify(NonCanonicalDimension &root, const SimplificationOptions &options) noexcept
  {
    // NOTE: This implements the non-canonical dimension relevant parts of the numeric value simplification
    // steps.

    // 1.2. If root is a dimension that is not expressed in its canonical unit, and there is enough
    // information available to convert it to the canonical unit, do so, and return the value.
    if (auto canonical = Canonicalize(root, options.ConversionData))
    {
      return CreateChild(krys::move(*canonical));
    }

    return null;
  }

  Maybe<Child> Simplify(Symbol &root, const SimplificationOptions &options) noexcept
  {
    // NOTE: This implements the keyword relevant parts of the numeric value simplification steps.

    // 1.3. If root is a <calc-keyword> that can be resolved, return what it resolves to, simplified.
    if (auto value = options.SymbolTable.Get(root.Id))
    {
      return CopyAndSimplify(CreateNumeric(value->Value, root.Unit), options);
    }

    return null;
  }

  Maybe<Child> Simplify(SiblingCount &, const SimplificationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }
    if (!options.ConversionData->StyleBuilderState()->GetElement())
    {
      return null;
    }

    return CreateChild(Number {
      .Value = static_cast<double>(options.ConversionData->ProtectedStyleBuilderState()->SiblingCount())});
  }

  Maybe<Child> Simplify(SiblingIndex &, const SimplificationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }
    if (!options.ConversionData->StyleBuilderState()->GetElement())
    {
      return null;
    }

    return CreateChild(Number {
      .Value = static_cast<double>(options.ConversionData->ProtectedStyleBuilderState()->SiblingIndex())});
  }

  Maybe<Child> Simplify(Sum &root, const SimplificationOptions &options) noexcept
  {
    krys_debug_assert(!root.Children.empty());

    // 8. If root is a Sum node:

    // 8.1. For each of root’s children that are Sum nodes, replace them with their children.
    if (std::ranges::any_of(root.Children,
                            [](auto &child) { return krys::HoldsAlternative<IndirectNode<Sum>>(child); }))
    {
      List<Child> newChildren;
      for (auto &child : root.Children)
      {
        if (auto *childSum = get_if<IndirectNode<Sum>>(&child))
        {
          for (auto &grandchild : (*childSum)->Children)
          {
            newChildren.push_back(krys::move(grandchild));
          }
        }
        else
        {
          newChildren.push_back(krys::move(child));
        }
      }
      root.Children = krys::move(newChildren);
    }

    // 8.2. For each set of root’s children that are numeric values with identical units, remove those
    // children and replace them with a single numeric value containing the sum of the removed nodes, and with
    // the same unit. (E.g. combine numbers, combine percentages, combine px values, etc.) 8.3. If root has
    // only a single child at this point, return the child. 8.4. Otherwise, return root

    // These steps are implemented as a two phase procedure.
    //    1. Iterate children to find "merge/removal opportunities", counting the total number of
    //    opportunities
    //       that will happen, and storing the index of the first child of each type in a lookup table.
    //    2. Perform merges and removals based on data from step 1.
    //
    // By splitting it up, we can perform two optimizations:
    //    1. If the result of step 1 shows that the number of "merge/removal opportunities" will lead to only
    //       one remaining child, we can avoid allocating a new Children list, and just merge directly into
    //       the child.
    //    2. If the result of step 1 shows that the number of "merge/removal opportunities" will lead to more
    //       than one remaining child, we can precisely allocate the Children list to be (existing children -
    //       "merge/removal opportunities").

    auto evaluate = [](const Child &a, const Child &b) -> std::pair<Child, double>
    {
      krys_debug_assert(a.index() == b.index());

      return krys::SwitchOn(
        a,
        [&]<Numeric T>(const T &aNumeric) -> std::pair<Child, double>
        {
          krys_debug_assert(ToNumericIdentity(aNumeric) == ToNumericIdentity(get<T>(b)));
          auto result = ExecuteMathOperation<Sum>(aNumeric.Value, get<T>(b).Value);
          return {CreateChildWithValueBasedOn(result, aNumeric), result};
        },
        [](const auto &) -> std::pair<Child, double>
        {
          krys_debug_assert_msg(false, "Only numeric children can be merged.");
          return {CreateChild(Number {.Value = 0}), 0};
        });
    };

    // Special case a root with one child to avoid doing any work at all, and just returning the child.
    if (root.Children.size() == 1)
    {
      return krys::move(root.Children[0]);
    }

    // Map of unit types (via NumericIdentity) to the first index in `root.Children` where a value with that
    // unit can be found. More specifically, it maps the unit to the index + 1, as 0 is used to indicate no
    // units of that type have been found.
    // FIXME: This should be turned into a type with an interface that doesn't require explicit use of
    // static_cast<uint8> by the caller.
    struct FirstInstance
    {
      size_t Offset = 0;
      uint32 Merges = 0;
      bool CanRemove = false;
    };
    std::array<FirstInstance, NumberOfNumericIdentityTypes> firstInstances {};

    for (size_t i = 0; i < root.Children.size(); ++i)
    {
      krys::SwitchOn(
        root.Children[i],
        [&]<Numeric T>(const T &child)
        {
          auto id = ToNumericIdentity(child);
          bool canRemoveIfZero = IsLength(id) && options.AllowZeroValueLengthRemovalFromSum;

          if (auto &firstInstance = firstInstances[static_cast<uint8>(id)]; firstInstance.Offset)
          {
            // There has already been an instance of this type. This is a merge opportunity.

            // Calculate the merged value.
            auto [mergedChild, mergedValue] =
              evaluate(root.Children[firstInstance.Offset - 1], root.Children[i]);

            // Store the merged value in the original array.
            root.Children[firstInstance.Offset - 1] = krys::move(mergedChild);

            // Update the `Merges` count and `CanRemove` bit for the new merged value.
            firstInstance.Merges += 1;
            firstInstance.CanRemove = canRemoveIfZero && !mergedValue;
            return;
          }

          // First instance of this. Store the index (well, index + 1, since 0 is the unset value) and the
          // CanRemove bit.
          firstInstances[static_cast<uint8>(id)] = {
            .Offset = i + 1, .Merges = 0, .CanRemove = canRemoveIfZero && !child.Value};
        },
        [](const auto &)
        {
          // Non-numeric values are not eligible for merge or removal.
        });
    }

    // Calculate the total number of children we will be able to remove from merges and removals.
    uint32 childrenToRemoveFromMerges = 0;
    uint32 childrenToRemoveTotal = 0;
    for (auto &firstInstance : firstInstances)
    {
      if (firstInstance.Offset)
      {
        childrenToRemoveFromMerges += firstInstance.Merges;
        childrenToRemoveTotal += firstInstance.Merges + (firstInstance.CanRemove ? 1 : 0);
      }
    }

    // If there are no merge/removal opportunities, no further simplification is possible.
    if (!childrenToRemoveTotal)
    {
      return null;
    }

    // If all the removal from merges leaves a single child, that means everything merged into the first
    // child.
    if ((root.Children.size() - childrenToRemoveFromMerges) == 1)
    {
      return krys::move(root.Children[0]);
    }

    auto combinedChildrenSize = root.Children.size() - childrenToRemoveTotal;

    // If the new size is 0, we removed too much. Return a single 0 value of type `length` to keep things
    // valid. A value of type `length` is returned because the only kind of node that can be removed is of
    // type `length`.
    if (!combinedChildrenSize)
    {
      return CreateChild(CanonicalDimension {.Value = 0, .Dimension = calc::Dimension::Length});
    }

    // If the new size is 1, we know there is one child, we just don't know which one yet.
    if (combinedChildrenSize == 1)
    {
      for (size_t i = 0; i < root.Children.size(); ++i)
      {
        auto replacement = krys::SwitchOn(
          root.Children[i],
          [&]<Numeric T>(const T &child) -> Maybe<Child>
          {
            auto &firstInstance = firstInstances[static_cast<uint8>(ToNumericIdentity(child))];
            krys_debug_assert(firstInstance.Offset);

            // If the stored offset for this type is set to this index and it's not one that can be removed,
            // this is the 1 child to return.
            if ((firstInstance.Offset - 1) == i && !firstInstance.CanRemove)
            {
              return krys::move(root.Children[i]);
            }

            // Otherwise, it's one that can be dropped.
            return null;
          },
          [&](const auto &) -> Maybe<Child> { return krys::move(root.Children[i]); });
        if (replacement)
        {
          return krys::move(*replacement);
        }
      }
    }

    List<Child> combinedChildren;
    combinedChildren.reserve(combinedChildrenSize);

    for (size_t i = 0; i < root.Children.size(); ++i)
    {
      krys::SwitchOn(
        root.Children[i],
        [&]<Numeric T>(const T &child)
        {
          auto &firstInstance = firstInstances[static_cast<uint8>(ToNumericIdentity(child))];
          krys_debug_assert(firstInstance.Offset);

          // If the stored offset for this type is set to this index and it's not one that can be removed,
          // append the child as normal
          if ((firstInstance.Offset - 1) == i && !firstInstance.CanRemove)
          {
            combinedChildren.push_back(krys::move(root.Children[i]));
            return;
          }

          // Otherwise, it's one that can be dropped.
        },
        [&](const auto &) { combinedChildren.push_back(krys::move(root.Children[i])); });
    }
    root.Children = krys::move(combinedChildren);

    return null;
  }

  Maybe<Child> Simplify(Product &root, const SimplificationOptions &options) noexcept
  {
    krys_debug_assert(!root.Children.empty());

    // 9. If root is a Product node:

    // NOTE: We merge steps 9.1. and 9.2, as they have significant overlap.

    // 9.1. For each of root’s children that are Product nodes, replace them with their children.
    //
    //   -- and --
    //
    // 9.2. If root has multiple children that are numbers (not percentages or dimensions), remove them and
    // replace them with a single number containing the product of the removed nodes.

    List<Child> newChildren;
    Maybe<Number> numericProduct;

    auto processChild = [&newChildren, &numericProduct](Child &child)
    {
      if (auto *childValue = get_if<Number>(&child))
      {
        if (numericProduct)
        {
          numericProduct = Number {.Value = childValue->Value * numericProduct->Value};
        }
        else
        {
          numericProduct = Number {.Value = childValue->Value};
        }
      }
      else
      {
        newChildren.push_back(krys::move(child));
      }
    };

    for (auto &child : root.Children)
    {
      if (auto *childProduct = get_if<IndirectNode<Product>>(&child))
      {
        for (auto &childProductChild : (*childProduct)->Children)
        {
          processChild(childProductChild);
        }
      }
      else
      {
        processChild(child);
      }
    }

    // If `numericProduct` has a value and `newChildren` is empty, that means all the children were numbers
    // and the product can be returned directly.
    if (numericProduct)
    {
      if (newChildren.empty())
      {
        return CreateChild(*numericProduct);
      }

      // 9.3. If root contains only two children, one of which is a number (not a percentage or dimension) and
      // the other of which is a Sum whose children are all numeric values, multiply all of the Sum’s children
      // by the number, then return the Sum.

      // We extend this step to include numeric and Invert children for the non-number child as an
      // optimization taking advantage of step 9.4, but for the case where the check is cheaper.

      // NOTE: Since we just merged all numeric values into `numericProduct`, we know that if `numericProduct`
      // is not null the last child is a singular `number` child. Therefore, we only need to check if there is
      // one child and is a Sum (or Numeric or Invert).

      if (newChildren.size() == 1)
      {
        auto replacement = krys::SwitchOn(
          newChildren[0], [&]<Numeric T>(T &numeric) -> Maybe<Child>
          { return CreateChildWithValueBasedOn(numeric.Value * numericProduct->Value, numeric); },
          [&](IndirectNode<Sum> &sum) -> Maybe<Child>
          {
            if (!std::ranges::all_of(sum->Children, [](const Child &child) { return IsNumeric(child); }))
            {
              return null;
            }

            for (auto &child : sum->Children)
            {
              krys::SwitchOn(
                child, [&]<Numeric T>(T &child) { child.Value *= numericProduct->Value; }, [](auto &) {});
            }

            return Child {krys::move(sum)};
          },
          [&](IndirectNode<Invert> &invert) -> Maybe<Child>
          {
            return krys::SwitchOn(
              invert->A, [&]<Numeric T>(const T &child) -> Maybe<Child>
              { return CreateChildWithValueBasedOn(child.Value * numericProduct->Value, child); },
              [](const auto &) -> Maybe<Child> { return null; });
          },
          [](auto &) -> Maybe<Child> { return null; });

        if (replacement)
        {
          return krys::move(*replacement);
        }
      }

      // If there was more than one child or no replacement was found, append the product from step 9.2 into
      // the newChildren list.
      newChildren.push_back(CreateChild(*numericProduct));
    }

    root.Children = krys::move(newChildren);

    // 9.4. If root contains only numeric values and/or Invert nodes containing numeric values, and
    // multiplying the types of all the children (noting that the type of an Invert node is the inverse of its
    // child’s type) results in a type that matches any of the types that a math function can resolve to,
    // return the result of multiplying all the values of the children (noting that the value of an Invert
    // node is the reciprocal of its child’s value), expressed in the result’s canonical unit.

    struct ProductResult
    {
      double Value;
      calc::Type Type;
    };
    auto productResult = ProductResult {.Value = 1, .Type = calc::Type {}};

    bool success = false;
    for (auto &child : root.Children)
    {
      success = krys::SwitchOn(
        child,
        [&](const Number &number) -> bool
        {
          // <number> is the identity type, so multiplying by it has no effect.
          productResult.Value *= number.Value;
          return true;
        },
        [&](const Percentage &percentage) -> bool
        {
          auto multipliedType = calc::Type::Multiply(productResult.Type, GetType(percentage));
          if (!multipliedType)
          {
            return false;
          }

          productResult.Type = *multipliedType;
          productResult.Value *= percentage.Value;
          return true;
        },
        [&](const CanonicalDimension &canonicalDimension) -> bool
        {
          auto multipliedType =
            calc::Type::Multiply(productResult.Type, GetType(canonicalDimension.Dimension));
          if (!multipliedType)
          {
            return false;
          }

          productResult.Type = *multipliedType;
          productResult.Value *= canonicalDimension.Value;
          return true;
        },
        [&](IndirectNode<Invert> &invertChild) -> bool
        {
          return krys::SwitchOn(
            invertChild->A,
            [&](const Number &number) -> bool
            {
              // <number> is the identity type, so multiplying / inverting by it has no effect.
              productResult.Value /= number.Value;
              return true;
            },
            [&](const Percentage &percentage) -> bool
            {
              auto invertedPercentageChildType = calc::Type::Invert(GetType(percentage));
              auto multipliedType = calc::Type::Multiply(productResult.Type, invertedPercentageChildType);
              if (!multipliedType)
              {
                return false;
              }

              productResult.Type = *multipliedType;
              productResult.Value /= percentage.Value;
              return true;
            },
            [&](const CanonicalDimension &canonicalDimension) -> bool
            {
              auto invertedCanonicalDimensionType = calc::Type::Invert(GetType(canonicalDimension));
              auto multipliedType = calc::Type::Multiply(productResult.Type, invertedCanonicalDimensionType);
              if (!multipliedType)
              {
                return false;
              }

              productResult.Type = *multipliedType;
              productResult.Value /= canonicalDimension.Value;
              return true;
            },
            [](const auto &) -> bool { return false; });
        },
        [](const auto &) -> bool { return false; });
      if (!success)
      {
        break;
      }
    }
    if (success)
    {
      if (auto category = productResult.Type.CalculationCategory())
      {
        switch (*category)
        {
          case css::Category::Integer:
          case css::Category::Number:
          {
            return CreateChild(Number {.Value = productResult.Value});
          }
          case css::Category::Percentage:
          {
            return CreateChild(Percentage {.Value = productResult.Value,
                                           .Hint = calc::Type::DeterminePercentHint(options.Category)});
          }
          case css::Category::LengthPercentage:
          {
            return CreateChild(Percentage {.Value = productResult.Value, .Hint = PercentHint::Length});
          }
          case css::Category::Length:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Length});
          }
          case css::Category::Angle:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Angle});
          }
          case css::Category::AnglePercentage:
          {
            return CreateChild(Percentage {.Value = productResult.Value, .Hint = PercentHint::Angle});
          }
          case css::Category::Time:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Time});
          }
          case css::Category::Frequency:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Frequency});
          }
          case css::Category::Resolution:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Resolution});
          }
          case css::Category::Flex:
          {
            return CreateChild(
              CanonicalDimension {.Value = productResult.Value, .Dimension = calc::Dimension::Flex});
          }
        }
      }
    }

    // 9.5. Return root.
    return null;
  }

  Maybe<Child> Simplify(Negate &root, const SimplificationOptions &) noexcept
  {
    // 6. If root is a Negate node:

    return krys::SwitchOn(
      root.A,
      [&]<Numeric T>(T &a) -> Maybe<Child>
      {
        // 6.1. If root’s child is a numeric value, return an equivalent numeric value, but with the value
        // negated (0 - value).
        return CreateChildWithValueBasedOn(0.0 - a.Value, a);
      },
      [](IndirectNode<Negate> &a) -> Maybe<Child>
      {
        // 6.2. If root’s child is a Negate node, return the child’s child.
        return krys::move(a->A);
      },
      [](IndirectNode<Sum> &a) -> Maybe<Child>
      {
        // Not stated in spec, but needed for tests.

        if (!std::ranges::all_of(a->Children, [](const Child &child) { return IsNumeric(child); }))
        {
          return null;
        }

        for (auto &child : a->Children)
        {
          krys::SwitchOn(child, [&]<Numeric T>(T &child) { child.Value = -child.Value; }, [](auto &) {});
        }

        return Child {krys::move(a)};
      },
      [](IndirectNode<Product> &a) -> Maybe<Child>
      {
        // Not stated in spec, but needed for tests.

        if (!std::ranges::all_of(a->Children, [](const Child &child) { return IsNumeric(child); }))
        {
          return null;
        }

        // Negating one factor negates the product.
        krys::SwitchOn(
          a->Children[0], [&]<Numeric T>(T &child) { child.Value = -child.Value; }, [](auto &) {});

        return Child {krys::move(a)};
      },
      [](auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Invert &root, const SimplificationOptions &) noexcept
  {
    // 7. If root is an Invert node:

    return krys::SwitchOn(
      root.A,
      [&](Number &a) -> Maybe<Child>
      {
        // 7.1. If root’s child is a number (not a percentage or dimension) return the reciprocal of the
        // child’s value.
        return CreateChild(Number {.Value = (1.0 / a.Value)});
      },
      [](IndirectNode<Invert> &a) -> Maybe<Child>
      {
        // 7.2. If root’s child is an Invert node, return the child’s child.
        return krys::move(a->A);
      },
      [](auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Min &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForMinMax(root, options);
  }

  Maybe<Child> Simplify(Max &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForMinMax(root, options);
  }

  Maybe<Child> Simplify(Clamp &root, const SimplificationOptions &options) noexcept
  {
    auto minIsNone = krys::HoldsAlternative<keywords::None>(root.Min);
    auto maxIsNone = krys::HoldsAlternative<keywords::None>(root.Max);

    if (minIsNone && maxIsNone)
    {
      // - clamp(none, VAL, none) is equivalent to just calc(VAL).
      return krys::move(root.Val);
    }

    // FIXME: Are any of these transforms kosher?
    // If only MIN and VAL have matching units, we can transform clamp(MIN, VAL, MAX) aka (max(MIN, min(VAL,
    // MAX)) into a min(newVAL, MAX). If only VAL and MAX have matching units, we can transform clamp(MIN,
    // VAL, MAX) aka (max(MIN, min(VAL, MAX)) into a max(MIN, newVAL).

    return krys::SwitchOn(
      root.Val,
      [&]<Numeric T>(T &val) -> Maybe<Child>
      {
        if (minIsNone)
        {
          auto &maxChild = get<Child>(root.Max);
          if (!krys::HoldsAlternative<T>(maxChild))
          {
            return null;
          }

          auto &max = get<T>(maxChild);

          if (!UnitsMatch(val, max, options))
          {
            return null;
          }

          // As units already match, we only have to check that one of the arguments is `MagnitudeComparable`.
          if (!MagnitudeComparable(val, options))
          {
            return null;
          }

          // - clamp(none, VAL, MAX) is equivalent to min(VAL, MAX)
          return CreateChildWithValueBasedOn(ExecuteMathOperation<Min>(val.Value, max.Value), val);
        }
        else if (maxIsNone)
        {
          auto &minChild = get<Child>(root.Min);
          if (!krys::HoldsAlternative<T>(minChild))
          {
            return null;
          }

          auto &min = get<T>(minChild);

          if (!UnitsMatch(min, val, options))
          {
            return null;
          }

          // As units already match, we only have to check that one of the arguments is `MagnitudeComparable`.
          if (!MagnitudeComparable(val, options))
          {
            return null;
          }

          // - clamp(MIN, VAL, none) is equivalent to max(MIN, VAL)
          return CreateChildWithValueBasedOn(ExecuteMathOperation<Max>(min.Value, val.Value), val);
        }
        else
        {
          auto &minChild = get<Child>(root.Min);
          auto &maxChild = get<Child>(root.Max);

          // If all three parameters have the same unit, we can perform the clamp in full.
          if (!krys::HoldsAlternative<T>(minChild) || !krys::HoldsAlternative<T>(maxChild))
          {
            return null;
          }

          auto &min = get<T>(minChild);
          auto &max = get<T>(maxChild);

          if (!UnitsMatch(min, val, options) || !UnitsMatch(val, max, options))
          {
            return null;
          }

          // As units already match, we only have to check that one of the arguments is `MagnitudeComparable`.
          if (!MagnitudeComparable(val, options))
          {
            return null;
          }

          return CreateChildWithValueBasedOn(ExecuteMathOperation<Clamp>(min.Value, val.Value, max.Value),
                                             val);
        }
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(RoundNearest &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForRound(root, options);
  }

  Maybe<Child> Simplify(RoundUp &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForRound(root, options);
  }

  Maybe<Child> Simplify(RoundDown &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForRound(root, options);
  }

  Maybe<Child> Simplify(RoundToZero &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForRound(root, options);
  }

  Maybe<Child> Simplify(Mod &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForOperation<Mod>(root.A, root.B, options);
  }

  Maybe<Child> Simplify(Rem &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForOperation<Rem>(root.A, root.B, options);
  }

  Maybe<Child> Simplify(Sin &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForTrig(root, options);
  }

  Maybe<Child> Simplify(Cos &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForTrig(root, options);
  }

  Maybe<Child> Simplify(Tan &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForTrig(root, options);
  }

  Maybe<Child> Simplify(Asin &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForArcTrig(root, options);
  }

  Maybe<Child> Simplify(Acos &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForArcTrig(root, options);
  }

  Maybe<Child> Simplify(Atan &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForArcTrig(root, options);
  }

  Maybe<Child> Simplify(Atan2 &root, const SimplificationOptions &options) noexcept
  {
    return SimplifyForOperationWithCompletion<Atan2>(
      root.A, root.B, options, [](double value) -> Maybe<Child>
      { return CreateChild(CanonicalDimension {.Value = value, .Dimension = calc::Dimension::Angle}); });
  }

  Maybe<Child> Simplify(Pow &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` and `b` have been type checked by this point to be `<number>`, though they may not
    // be able to be fully resolved yet.

    return SwitchTogether(
      root.A, root.B, [&](const Number &a, const Number &b) -> Maybe<Child>
      { return CreateChild(Number {.Value = ExecuteMathOperation<Pow>(a.Value, b.Value)}); },
      [](const auto &, const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Sqrt &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` has been type checked by this point to be `<number>`, though they may not
    // be able to be fully resolved yet.

    return krys::SwitchOn(
      root.A, [&](const Number &a) -> Maybe<Child>
      { return CreateChild(Number {.Value = ExecuteMathOperation<Sqrt>(a.Value)}); },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Hypot &root, const SimplificationOptions &options) noexcept
  {
    // Hypot can be simplified if all its children are the same type, and it is both canonical (for lengths)
    // and fully resolved (for percentages). We optimistically assume that the children fit this criteria, and
    // execute the operation over the children, checking each one as it is requested. If we find out our
    // assumption was incorrect (e.g. a child is non-canonical or non-resolved), we set a flag indicating the
    // evaluation failed, but due to the evaluation API's interface, must evaluate all the remaining children.
    // Once the evaluation is complete, if the fail bit is set, we failed to simplify, if it is not, we can
    // return the new numeric result.

    struct NumberTag
    {
    };
    struct PercentageTag
    {
    };
    struct DimensionTag
    {
      calc::Dimension Dimension;
    };
    struct FailureTag
    {
    };
    Variant<std::monostate, NumberTag, PercentageTag, DimensionTag, FailureTag> result;

    double value = ExecuteMathOperation<Hypot>(
      root.Children.Value,
      [&](const auto &child)
      {
        return krys::SwitchOn(
          result,
          [&](const std::monostate &) -> double
          {
            // First iteration.
            return krys::SwitchOn(
              child,
              [&](const Number &number) -> double
              {
                result = NumberTag {};
                return number.Value;
              },
              [&](const Percentage &percentage) -> double
              {
                if (PercentageResolveToDimension(options))
                {
                  result = FailureTag {};
                  return Limits<double>::quiet_NaN();
                }
                result = PercentageTag {};
                return percentage.Value;
              },
              [&](const CanonicalDimension &dimension) -> double
              {
                result = DimensionTag {dimension.Dimension};
                return dimension.Value;
              },
              [&](const auto &) -> double
              {
                result = FailureTag {};
                return Limits<double>::quiet_NaN();
              });
          },
          [&](const NumberTag &) -> double
          {
            if (auto *numberChild = get_if<Number>(&child))
            {
              return numberChild->Value;
            }
            result = FailureTag {};
            return Limits<double>::quiet_NaN();
          },
          [&](const PercentageTag &) -> double
          {
            if (auto *percentageChild = get_if<Percentage>(&child))
            {
              return percentageChild->Value;
            }
            result = FailureTag {};
            return Limits<double>::quiet_NaN();
          },
          [&](const DimensionTag &tag) -> double
          {
            if (auto *dimensionChild = get_if<CanonicalDimension>(&child);
                dimensionChild && dimensionChild->Dimension == tag.Dimension)
            {
              return dimensionChild->Value;
            }
            result = FailureTag {};
            return Limits<double>::quiet_NaN();
          },
          [&](const FailureTag &) -> double { return Limits<double>::quiet_NaN(); });
      });

    return krys::SwitchOn(
      result, [&](const NumberTag &) -> Maybe<Child> { return CreateChild(Number {.Value = value}); },
      [&](const PercentageTag &) -> Maybe<Child>
      {
        return CreateChild(
          Percentage {.Value = value, .Hint = calc::Type::DeterminePercentHint(options.Category)});
      },
      [&](const DimensionTag &tag) -> Maybe<Child>
      { return CreateChild(CanonicalDimension {.Value = value, .Dimension = tag.Dimension}); },
      [&](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Log &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` and `b` have been type checked by this point to be `<number>`, though they may not
    // be able to be fully resolved yet.

    if (root.B)
    {
      return SwitchTogether(
        root.A, *root.B, [&](const Number &a, const Number &b) -> Maybe<Child>
        { return CreateChild(Number {.Value = ExecuteMathOperation<Log>(a.Value, b.Value)}); },
        [](const auto &, const auto &) -> Maybe<Child> { return null; });
    }

    return krys::SwitchOn(
      root.A, [](const Number &a) -> Maybe<Child>
      { return CreateChild(Number {.Value = ExecuteMathOperation<Log>(a.Value)}); },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Exp &root, const SimplificationOptions &) noexcept
  {
    // NOTE: `a` has been type checked by this point to be `<number>`, though they may not
    // be able to be fully resolved yet.

    return krys::SwitchOn(
      root.A, [](const Number &a) -> Maybe<Child>
      { return CreateChild(Number {.Value = ExecuteMathOperation<Exp>(a.Value)}); },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Abs &root, const SimplificationOptions &options) noexcept
  {
    return krys::SwitchOn(
      root.A,
      [&]<Numeric T>(const T &a) -> Maybe<Child>
      {
        if (!MagnitudeComparable(a, options))
        {
          return null;
        }
        return CreateChildWithValueBasedOn(ExecuteMathOperation<Abs>(a.Value), a);
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Sign &root, const SimplificationOptions &options) noexcept
  {
    return krys::SwitchOn(
      root.A,
      [&]<Numeric T>(const T &a) -> Maybe<Child>
      {
        if (!MagnitudeComparable(a, options))
        {
          return null;
        }
        return CreateChild(Number {.Value = ExecuteMathOperation<Sign>(a.Value)});
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Random &root, const SimplificationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }
    if (root.Min.index() != root.Max.index() || (root.Step && root.Step->index() != root.Min.index()))
    {
      return null;
    }

    return krys::SwitchOn(
      root.Min,
      [&]<Numeric T>(const T &numericMin) -> Maybe<Child>
      {
        auto numericMax = get<T>(root.Max);

        if (!UnitsMatch(numericMin, numericMax, options) || !FullyResolved(numericMin, options))
        {
          return null;
        }

        Maybe<double> valueStep;
        if (root.Step)
        {
          auto numericStep = get<T>(*root.Step);

          if (!UnitsMatch(numericMin, numericStep, options))
          {
            return null;
          }

          valueStep = numericStep.Value;
        }

        auto randomBaseValue = krys::SwitchOn(
          root.Sharing,
          [&](const Random::SharingOptions &sharingOptions) -> Maybe<double>
          {
            // A value that isn't element-shared is keyed on the element, so it needs one.
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
              [](const css::Number<css::ClosedUnitRange>::Raw &raw) -> Maybe<double> { return raw.Value; },
              [](const css::Number<css::ClosedUnitRange>::Calc &) -> Maybe<double> { return null; });
          });
        if (!randomBaseValue)
        {
          return null;
        }

        return CreateChildWithValueBasedOn(
          ExecuteMathOperation<Random>(*randomBaseValue, numericMin.Value, numericMax.Value, valueStep),
          numericMin);
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Progress &root, const SimplificationOptions &options) noexcept
  {
    if (root.Value.index() != root.Start.index() || root.Start.index() != root.End.index())
    {
      return null;
    }

    return krys::SwitchOn(
      root.Value,
      [&]<Numeric T>(const T &numericValue) -> Maybe<Child>
      {
        const auto &numericStart = get<T>(root.Start);
        const auto &numericEnd = get<T>(root.End);

        if (!UnitsMatch(numericValue, numericStart, options) || !UnitsMatch(numericStart, numericEnd, options)
            || !FullyResolved(numericValue, options))
        {
          return null;
        }

        return CreateChild(Number {
          .Value = ExecuteMathOperation<Progress>(numericValue.Value, numericStart.Value, numericEnd.Value)});
      },
      [](const auto &) -> Maybe<Child> { return null; });
  }

  Maybe<Child> Simplify(Anchor &anchor, const SimplificationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }

    auto evaluationOptions = EvaluationOptions {.Category = options.Category,
                                                .Range = css::All,
                                                .ConversionData = options.ConversionData,
                                                .SymbolTable = options.SymbolTable};

    auto result = EvaluateWithoutFallback(anchor, evaluationOptions);
    if (!result)
    {
      // https://drafts.csswg.org/css-anchor-position-1/#anchor-valid
      // "If any of these conditions are false, the anchor() function resolves to its specified fallback
      // value. If no fallback value is specified, it makes the declaration referencing it invalid at
      // computed-value time."

      if (!anchor.Fallback)
      {
        options.ConversionData->ProtectedStyleBuilderState()->SetCurrentPropertyInvalidAtComputedValueTime();
      }

      // Replace the anchor node with the fallback node.
      return std::exchange(anchor.Fallback, Maybe<Child> {});
    }
    return CreateChild(CanonicalDimension {.Value = *result, .Dimension = calc::Dimension::Length});
  }

  Maybe<Child> Simplify(AnchorSize &anchorSize, const SimplificationOptions &options) noexcept
  {
    if (!options.ConversionData || !options.ConversionData->StyleBuilderState())
    {
      return null;
    }

    auto builderState = options.ConversionData->ProtectedStyleBuilderState();

    Maybe<style::ScopedName> anchorSizeScopedName;
    if (anchorSize.ElementName != CSSOMStringAtom::Null())
    {
      anchorSizeScopedName =
        style::ScopedName {.Name = anchorSize.ElementName, .ScopeOrdinal = builderState->StyleScopeOrdinal()};
    }

    auto result =
      style::AnchorPositionEvaluator::EvaluateSize(*builderState, anchorSizeScopedName, anchorSize.Dimension);

    if (!result)
    {
      if (!anchorSize.Fallback)
      {
        options.ConversionData->ProtectedStyleBuilderState()->SetCurrentPropertyInvalidAtComputedValueTime();
      }

      return std::exchange(anchorSize.Fallback, Maybe<Child> {});
    }

    return CreateChild(CanonicalDimension {.Value = *result, .Dimension = calc::Dimension::Length});
  }

  // MARK: Copy & Simplify.

  Random::SharingType CopyAndSimplify(const Random::SharingType &root, const SimplificationOptions &) noexcept
  {
    return root;
  }

  keywords::None CopyAndSimplify(const keywords::None &root, const SimplificationOptions &) noexcept
  {
    return root;
  }

  Children CopyAndSimplify(const Children &children, const SimplificationOptions &options) noexcept
  {
    List<Child> result;
    result.reserve(children.size());
    for (const auto &child : children)
    {
      result.push_back(CopyAndSimplify(child, options));
    }
    return result;
  }

  ChildOrNone CopyAndSimplify(const ChildOrNone &root, const SimplificationOptions &options) noexcept
  {
    return krys::SwitchOn(root,
                          [&](const auto &root) { return ChildOrNone {CopyAndSimplify(root, options)}; });
  }

  template <typename T>
  Maybe<T> CopyAndSimplify(const Maybe<T> &root, const SimplificationOptions &options) noexcept
  {
    if (root)
    {
      return CopyAndSimplify(*root, options);
    }
    return null;
  }

  template <Leaf Op>
  static Op CopyAndSimplifyChildren(const Op &op, const SimplificationOptions &) noexcept
  {
    return op;
  }

  template <typename Op>
  static Op CopyAndSimplifyChildren(const IndirectNode<Op> &root,
                                    const SimplificationOptions &options) noexcept
  {
    return krys::apply([&](const auto &...x) { return Op {CopyAndSimplify(x, options)...}; }, *root);
  }

  static Anchor CopyAndSimplifyChildren(const IndirectNode<Anchor> &anchor,
                                        const SimplificationOptions &options) noexcept
  {
    return Anchor {.ElementName = anchor->ElementName,
                   .Side = Copy(anchor->Side),
                   .Fallback = CopyAndSimplify(anchor->Fallback, options)};
  }

  static AnchorSize CopyAndSimplifyChildren(const IndirectNode<AnchorSize> &anchorSize,
                                            const SimplificationOptions &options) noexcept
  {
    return AnchorSize {.ElementName = anchorSize->ElementName,
                       .Dimension = anchorSize->Dimension,
                       .Fallback = CopyAndSimplify(anchorSize->Fallback, options)};
  }

  Child CopyAndSimplify(const Child &root, const SimplificationOptions &options) noexcept
  {
    return krys::SwitchOn(root,
                          [&](const auto &root) -> Child
                          {
                            // Create a simplified copy by recursively calling simplify on all children.
                            auto simplified = CopyAndSimplifyChildren(root, options);

                            // Attempt to simplify the term itself, using the result as a replacement if
                            // successful.
                            if (auto replacement = Simplify(simplified, options))
                            {
                              return krys::move(*replacement);
                            }

                            return CreateChild(krys::move(simplified), GetType(root));
                          });
  }

  Tree CopyAndSimplify(const Tree &tree, const SimplificationOptions &options) noexcept
  {
    return Tree {
      .Root = CopyAndSimplify(tree.Root, options),
      .Type = tree.Type,
      .Stage = tree.Stage,
      .RequiresConversionData = tree.RequiresConversionData,
    };
  }

  // MARK: - Can Simplify

  bool CanSimplify(const Tree &tree, const SimplificationOptions &) noexcept
  {
    // NOTE: This is a simple and conservative implementation of `CanSimplify`. A more precise implementation
    // is possible by utilizing the provided `SimplificationOptions` if that should be necessary.

    return krys::SwitchOn(
      tree.Root, [&](const Number &) -> bool { return false; },
      [&](const Percentage &) -> bool { return false; },
      [&](const CanonicalDimension &) -> bool { return false; }, [&](const auto &) -> bool { return true; });
  }
}
