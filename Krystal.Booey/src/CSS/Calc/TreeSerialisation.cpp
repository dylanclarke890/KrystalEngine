#include "Krystal.Booey/CSS/Calc/TreeSerialisation.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeTraversal.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include <algorithm>
#include <cmath>
#include <utility>

namespace krys::boo::css::calc
{
  namespace
  {
    struct SerialisationState
    {
      enum class GroupingParenthesis
      {
        Omit,
        Include
      };

      KRYS_NODISCARD CSSOMStringView OpenGroup() const noexcept
      {
        return Grouping == GroupingParenthesis::Omit ? u8"" : u8"(";
      }

      KRYS_NODISCARD CSSOMStringView CloseGroup() const noexcept
      {
        return Grouping == GroupingParenthesis::Omit ? u8"" : u8")";
      }

      GroupingParenthesis Grouping = GroupingParenthesis::Include;
      calc::Stage Stage = calc::Stage::Specified;
      css::Range Range = css::All;
      const css::SerialisationContext &SerialisationContext;
    };

    struct ParenthesisSaver
    {
      KRYS_NON_COPY_MOVABLE(ParenthesisSaver);

      SerialisationState &State;
      SerialisationState::GroupingParenthesis SavedGrouping;

      explicit ParenthesisSaver(SerialisationState &state) noexcept
          : State {state}, SavedGrouping {state.Grouping}
      {
      }

      ~ParenthesisSaver() noexcept
      {
        State.Grouping = SavedGrouping;
      }
    };

    KRYS_NODISCARD bool IsNull(const CSSOMStringAtom &atom) noexcept
    {
      return atom == CSSOMStringAtom::Null();
    }

    void AppendIdentifier(StringBuilder &builder, const CSSOMStringAtom &atom) noexcept
    {
      SerialiseIdentifier(CSSOMString {atom.View()}, builder);
    }
  }

  // https://drafts.csswg.org/css-values-4/#serialize-a-math-function
  static void SerialiseMathFunction(StringBuilder &, const Child &, SerialisationState &) noexcept;
  static void SerialiseMathFunction(StringBuilder &, const Symbol &, SerialisationState &) noexcept;
  static void SerialiseMathFunction(StringBuilder &, const SiblingCount &, SerialisationState &) noexcept;
  static void SerialiseMathFunction(StringBuilder &, const SiblingIndex &, SerialisationState &) noexcept;
  template <Numeric Op>
  static void SerialiseMathFunction(StringBuilder &, const Op &, SerialisationState &) noexcept;
  template <typename Op>
  static void SerialiseMathFunction(StringBuilder &, const IndirectNode<Op> &, SerialisationState &) noexcept;

  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<Sum> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<Product> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<Negate> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<Invert> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<RoundNearest> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<RoundUp> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<RoundDown> &,
                                          SerialisationState &) noexcept;
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<RoundToZero> &,
                                          SerialisationState &) noexcept;
  template <typename Op>
  static void SerialiseMathFunctionPrefix(StringBuilder &, const IndirectNode<Op> &,
                                          SerialisationState &) noexcept;

  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<Sum> &,
                                             SerialisationState &) noexcept;
  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<Product> &,
                                             SerialisationState &) noexcept;
  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<Random> &,
                                             SerialisationState &) noexcept;
  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<Anchor> &,
                                             SerialisationState &) noexcept;
  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<AnchorSize> &,
                                             SerialisationState &) noexcept;
  template <typename Op>
  static void SerialiseMathFunctionArguments(StringBuilder &, const IndirectNode<Op> &,
                                             SerialisationState &) noexcept;

  static void SerialiseWithoutOmittingPrefix(StringBuilder &, const Child &, SerialisationState &) noexcept;

  // https://drafts.csswg.org/css-values-4/#serialize-a-calculation-tree
  static void SerialiseCalculationTree(StringBuilder &, const Child &, SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const ChildOrNone &, SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const keywords::None &,
                                       SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const Symbol &, SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const SiblingCount &, SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const SiblingIndex &, SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const IndirectNode<Sum> &,
                                       SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const IndirectNode<Product> &,
                                       SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const IndirectNode<Negate> &,
                                       SerialisationState &) noexcept;
  static void SerialiseCalculationTree(StringBuilder &, const IndirectNode<Invert> &,
                                       SerialisationState &) noexcept;
  template <Numeric Op>
  static void SerialiseCalculationTree(StringBuilder &, const Op &, SerialisationState &) noexcept;
  template <typename Op>
  static void SerialiseCalculationTree(StringBuilder &, const IndirectNode<Op> &,
                                       SerialisationState &) noexcept;

  // MARK: Sorting

  static uint32 SortPriority(UnitType unit) noexcept
  {
    // Sort order: number, percentage, dimension (by unit, ordered ASCII case-insensitively), other.

    switch (unit)
    {
      // number
      case UnitType::Number:
      case UnitType::Integer:    return 0;
      // percentage
      case UnitType::Percentage: return 1;

      // dimension (by unit, ordered ASCII case-insensitively)
      case UnitType::cap:        return 2;
      case UnitType::ch:         return 3;
      case UnitType::cm:         return 4;
      case UnitType::cqb:        return 5;
      case UnitType::cqh:        return 6;
      case UnitType::cqi:        return 7;
      case UnitType::cqmax:      return 8;
      case UnitType::cqmin:      return 9;
      case UnitType::cqw:        return 10;
      case UnitType::deg:        return 11;
      case UnitType::dpcm:       return 12;
      case UnitType::dpi:        return 13;
      case UnitType::dppx:       return 14;
      case UnitType::dvb:        return 15;
      case UnitType::dvh:        return 16;
      case UnitType::dvi:        return 17;
      case UnitType::dvmax:      return 18;
      case UnitType::dvmin:      return 19;
      case UnitType::dvw:        return 20;
      case UnitType::em:         return 21;
      case UnitType::ex:         return 22;
      case UnitType::fr:         return 23;
      case UnitType::grad:       return 24;
      case UnitType::hz:         return 25;
      case UnitType::ic:         return 26;
      case UnitType::in:         return 27;
      case UnitType::khz:        return 28;
      case UnitType::lh:         return 29;
      case UnitType::lvb:        return 30;
      case UnitType::lvh:        return 31;
      case UnitType::lvi:        return 32;
      case UnitType::lvmax:      return 33;
      case UnitType::lvmin:      return 34;
      case UnitType::lvw:        return 35;
      case UnitType::mm:         return 36;
      case UnitType::ms:         return 37;
      case UnitType::pc:         return 38;
      case UnitType::pt:         return 39;
      case UnitType::px:         return 40;
      case UnitType::Q:          return 41;
      case UnitType::rad:        return 42;
      case UnitType::rcap:       return 43;
      case UnitType::rch:        return 44;
      case UnitType::rem:        return 45;
      case UnitType::rex:        return 46;
      case UnitType::ric:        return 47;
      case UnitType::rlh:        return 48;
      case UnitType::s:          return 49;
      case UnitType::svb:        return 50;
      case UnitType::svh:        return 51;
      case UnitType::svi:        return 52;
      case UnitType::svmax:      return 53;
      case UnitType::svmin:      return 54;
      case UnitType::svw:        return 55;
      case UnitType::turn:       return 56;
      case UnitType::vb:         return 57;
      case UnitType::vh:         return 58;
      case UnitType::vi:         return 59;
      case UnitType::vmax:       return 60;
      case UnitType::vmin:       return 61;
      case UnitType::vw:         return 62;
      case UnitType::x:          return 63;

      // Non-numeric types are not supported.
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

    krys_debug_assert(false);
    return 64;
  }

  static uint32 SortPriority(const Child &child) noexcept
  {
    // https://drafts.csswg.org/css-values-4/#sort-a-calculations-children

    return krys::SwitchOn(
      child, []<Numeric T>(const T &root) -> uint32 { return SortPriority(ToUnit(root)); },
      [](const auto &) -> uint32
      {
        return 65; // NOTE: 65 is greater than any numeric unit type, even the error case.
      });
  }

  namespace
  {
    struct ChildRepresentation
    {
      // Offset in the operations `Children` list.
      size_t Index;

      // Value used to order children during sort, based on unit.
      uint32 SortPriority;
    };
  }

  static SmallList<ChildRepresentation, 16> GenerateSortedChildrenMap(const Children &children) noexcept
  {
    SmallList<ChildRepresentation, 16> sortedChildrenMap;
    sortedChildrenMap.reserve(children.size());

    for (size_t i = 0; i < children.size(); ++i)
    {
      sortedChildrenMap.push_back(
        ChildRepresentation {.Index = i, .SortPriority = SortPriority(children[i])});
    }

    std::ranges::stable_sort(sortedChildrenMap, {}, &ChildRepresentation::SortPriority);

    return sortedChildrenMap;
  }

  // MARK: Math Function
  // https://drafts.csswg.org/css-values-4/#serialize-a-math-function

  static double ClampValue(double value, css::Range range) noexcept
  {
    value = std::isnan(value) ? 0 : value;
    return std::clamp(value, range.Min, range.Max);
  }

  void SerialiseMathFunction(StringBuilder &builder, const Child &fn, SerialisationState &state) noexcept
  {
    krys::SwitchOn(fn, [&builder, &state](const auto &root) { SerialiseMathFunction(builder, root, state); });
  }

  template <Numeric Op>
  void SerialiseMathFunction(StringBuilder &builder, const Op &fn, SerialisationState &state) noexcept
  {
    // 1. If the root of the calculation tree fn represents is a numeric value (number, percentage, or
    // dimension), and the serialization being produced is of a computed value or later, then clamp the value
    // to the range allowed for its context (if necessary), then serialize the value as normal and return the
    // result.

    if (state.Stage == Stage::Computed)
    {
      auto clampedFn = CreateChildWithValueBasedOn(ClampValue(fn.Value, state.Range), fn);
      SerialiseCalculationTree(builder, clampedFn, state);
      return;
    }

    // `SerialisableNumber` serialisation implements the appropriate logic for steps 2 & steps 3-5 for Numeric
    // expressions.

    // 2. If fn represents an infinite or NaN value: let s be the string "calc(".
    // 2.1. Let s be the string "calc(".
    // 2.2. Serialize the keyword infinity, -infinity, or NaN, as appropriate to represent the value, and
    // append it to s.
    // 2.3. If fn’s type is anything other than «[ ]» (empty, representing a <number>), append " * " to s.
    // Create a numeric value in the canonical unit for fn’s type (such as px for <length>), with a value
    // of 1. Serialize this numeric value and append it to s.

    // [...]

    // 3. If the calculation tree’s root node is a numeric value, or a calc-operator node, let s be a string
    // initially containing "calc(".
    // 4. For each child of the root node, serialize the calculation tree. [...]
    // 5. Append ")" (close parenthesis) to s.

    builder.Append(u8"calc(");
    SerialiseCalculationTree(builder, fn, state);
    builder.Append(u8')');
  }

  void SerialiseMathFunction(StringBuilder &builder, const Symbol &fn, SerialisationState &state) noexcept
  {
    builder.Append(u8"calc(");
    SerialiseCalculationTree(builder, fn, state);
    builder.Append(u8')');
  }

  void SerialiseMathFunction(StringBuilder &builder, const SiblingCount &fn,
                             SerialisationState &state) noexcept
  {
    SerialiseCalculationTree(builder, fn, state);
  }

  void SerialiseMathFunction(StringBuilder &builder, const SiblingIndex &fn,
                             SerialisationState &state) noexcept
  {
    SerialiseCalculationTree(builder, fn, state);
  }

  template <typename Op>
  void SerialiseMathFunction(StringBuilder &builder, const IndirectNode<Op> &fn,
                             SerialisationState &state) noexcept
  {
    // 3. If the calculation tree’s root node is a numeric value, or a calc-operator node, let s be a string
    // initially containing "calc(".
    //
    //    Otherwise, let s be a string initially containing the name of the root node, lowercased (such as
    //    "sin" or "max"), followed by a "(" (open parenthesis).

    // Both clauses of step 3 are handle via the appropriate overloaded function.

    SerialiseMathFunctionPrefix(builder, fn, state);

    // 4. For each child of the root node, serialize the calculation tree. If a result of this serialization
    // starts with a "(" (open parenthesis) and ends with a ")" (close parenthesis), remove those characters
    // from the result. Concatenate all of the results using ", " (comma followed by space), then append the
    // result to s.
    {
      ParenthesisSaver saver {state};
      state.Grouping = SerialisationState::GroupingParenthesis::Omit;

      SerialiseMathFunctionArguments(builder, fn, state);
    }

    // 5. Append ")" (close parenthesis) to s.
    builder.Append(u8')');
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<Sum> &,
                                   SerialisationState &) noexcept
  {
    builder.Append(u8"calc(");
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<Product> &,
                                   SerialisationState &) noexcept
  {
    builder.Append(u8"calc(");
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<Negate> &,
                                   SerialisationState &) noexcept
  {
    builder.Append(u8"calc(");
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<Invert> &,
                                   SerialisationState &) noexcept
  {
    builder.Append(u8"calc(");
  }

  static void SerialiseRoundPrefix(StringBuilder &builder, ValueId strategy) noexcept
  {
    builder.Append(ToString(ValueId::Round));
    builder.Append(u8'(');
    builder.Append(ToString(strategy));
    builder.Append(u8", ");
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<RoundNearest> &,
                                   SerialisationState &) noexcept
  {
    SerialiseRoundPrefix(builder, RoundNearest::Id);
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<RoundUp> &,
                                   SerialisationState &) noexcept
  {
    SerialiseRoundPrefix(builder, RoundUp::Id);
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<RoundDown> &,
                                   SerialisationState &) noexcept
  {
    SerialiseRoundPrefix(builder, RoundDown::Id);
  }

  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<RoundToZero> &,
                                   SerialisationState &) noexcept
  {
    SerialiseRoundPrefix(builder, RoundToZero::Id);
  }

  template <typename Op>
  void SerialiseMathFunctionPrefix(StringBuilder &builder, const IndirectNode<Op> &,
                                   SerialisationState &) noexcept
  {
    builder.Append(ToString(Op::Id));
    builder.Append(u8'(');
  }

  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<Sum> &fn,
                                      SerialisationState &state) noexcept
  {
    SerialiseCalculationTree(builder, fn, state);
  }

  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<Product> &fn,
                                      SerialisationState &state) noexcept
  {
    SerialiseCalculationTree(builder, fn, state);
  }

  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<Random> &fn,
                                      SerialisationState &state) noexcept
  {
    krys::SwitchOn(
      fn->Sharing,
      [&](const Random::SharingOptions &options)
      {
        const bool hasIdentifier = std::holds_alternative<CSSOMStringAtom>(options.Identifier);
        if (hasIdentifier && options.ElementShared)
        {
          builder.Append(std::get<CSSOMStringAtom>(options.Identifier).View());
          builder.Append(u8' ');
          builder.Append(ToString(ValueId::ElementShared));
          builder.Append(u8", ");
        }
        else if (hasIdentifier)
        {
          builder.Append(std::get<CSSOMStringAtom>(options.Identifier).View());
          builder.Append(u8", ");
        }
        else if (options.ElementShared)
        {
          builder.Append(ToString(ValueId::ElementShared));
          builder.Append(u8", ");
        }
      },
      [&](const Random::SharingFixed &fixed)
      {
        builder.Append(ToString(ValueId::Fixed));
        builder.Append(u8' ');
        Serialise(builder, state.SerialisationContext, fixed.Value);
        builder.Append(u8", ");
      });

    SerialiseCalculationTree(builder, fn->Min, state);
    builder.Append(u8", ");
    SerialiseCalculationTree(builder, fn->Max, state);

    if (fn->Step)
    {
      builder.Append(u8", ");
      SerialiseCalculationTree(builder, *fn->Step, state);
    }
  }

  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<Anchor> &anchor,
                                      SerialisationState &state) noexcept
  {
    if (!IsNull(anchor->ElementName))
    {
      AppendIdentifier(builder, anchor->ElementName);
      builder.Append(u8' ');
    }

    krys::SwitchOn(
      anchor->Side, [&](ValueId valueId) { builder.Append(ToString(valueId)); },
      [&](const Child &percentage)
      {
        // As anchor() is not actually a "math function", calc() can't be omitted in arguments.
        SerialiseWithoutOmittingPrefix(builder, percentage, state);
      });

    if (anchor->Fallback)
    {
      builder.Append(u8", ");
      SerialiseWithoutOmittingPrefix(builder, *anchor->Fallback, state);
    }
  }

  static void SerialiseAnchorSizeDimension(StringBuilder &builder,
                                           style::AnchorSizeDimension dimension) noexcept
  {
    switch (dimension)
    {
      case style::AnchorSizeDimension::Width:
      {
        builder.Append(u8"width");
        break;
      }
      case style::AnchorSizeDimension::Height:
      {
        builder.Append(u8"height");
        break;
      }
      case style::AnchorSizeDimension::Block:
      {
        builder.Append(u8"block");
        break;
      }
      case style::AnchorSizeDimension::Inline:
      {
        builder.Append(u8"inline");
        break;
      }
      case style::AnchorSizeDimension::SelfBlock:
      {
        builder.Append(u8"self-block");
        break;
      }
      case style::AnchorSizeDimension::SelfInline:
      {
        builder.Append(u8"self-inline");
        break;
      }
    }
  }

  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<AnchorSize> &anchorSize,
                                      SerialisationState &state) noexcept
  {
    const bool hasElementName = !IsNull(anchorSize->ElementName);

    if (hasElementName)
    {
      AppendIdentifier(builder, anchorSize->ElementName);
    }

    if (anchorSize->Dimension)
    {
      if (hasElementName)
      {
        builder.Append(u8' ');
      }

      SerialiseAnchorSizeDimension(builder, *anchorSize->Dimension);
    }

    if (anchorSize->Fallback)
    {
      if (hasElementName || anchorSize->Dimension)
      {
        builder.Append(u8", ");
      }

      SerialiseWithoutOmittingPrefix(builder, *anchorSize->Fallback, state);
    }
  }

  template <typename Op>
  void SerialiseMathFunctionArguments(StringBuilder &builder, const IndirectNode<Op> &fn,
                                      SerialisationState &state) noexcept
  {
    CSSOMStringView separator = u8"";
    ForAllChildren(*fn, krys::CreateVisitor(
                          [&](const Maybe<Child> &root)
                          {
                            if (root)
                            {
                              builder.Append(std::exchange(separator, u8", "));
                              SerialiseCalculationTree(builder, *root, state);
                            }
                          },
                          [&](const CSSOMStringAtom &root)
                          {
                            if (!IsNull(root))
                            {
                              builder.Append(std::exchange(separator, u8", "));
                              AppendIdentifier(builder, root);
                            }
                          },
                          [&](const auto &root)
                          {
                            builder.Append(std::exchange(separator, u8", "));
                            SerialiseCalculationTree(builder, root, state);
                          }));
  }

  void SerialiseWithoutOmittingPrefix(StringBuilder &builder, const Child &child,
                                      SerialisationState &state) noexcept
  {
    krys::SwitchOn(
      child, [&](const Leaf auto &op) { SerialiseCalculationTree(builder, op, state); },
      [&](const auto &op) { SerialiseMathFunction(builder, op, state); });
  }

  // MARK: Calculation Tree
  // https://drafts.csswg.org/css-values-4/#serialize-a-calculation-tree

  void SerialiseCalculationTree(StringBuilder &builder, const Child &root, SerialisationState &state) noexcept
  {
    krys::SwitchOn(root,
                   [&builder, &state](const auto &root) { SerialiseCalculationTree(builder, root, state); });
  }

  void SerialiseCalculationTree(StringBuilder &builder, const ChildOrNone &root,
                                SerialisationState &state) noexcept
  {
    krys::SwitchOn(root,
                   [&builder, &state](const auto &root) { SerialiseCalculationTree(builder, root, state); });
  }

  void SerialiseCalculationTree(StringBuilder &builder, const keywords::None &root,
                                SerialisationState &state) noexcept
  {
    Serialise(builder, state.SerialisationContext, root);
  }

  template <Numeric Op>
  void SerialiseCalculationTree(StringBuilder &builder, const Op &root, SerialisationState &) noexcept
  {
    // 2. If root is a numeric value, or a non-math function, serialize root per the normal rules for it and
    // return the result.

    FormatCSSNumberValue(builder,
                         SerialisableNumber {root.Value, PrimitiveValue::UnitTypeString(ToUnit(root))});
  }

  void SerialiseCalculationTree(StringBuilder &builder, const Symbol &root, SerialisationState &) noexcept
  {
    // 2. If root is a numeric value, or a non-math function, serialize root per the normal rules for it and
    // return the result.

    builder.Append(ToString(root.Id));
  }

  void SerialiseCalculationTree(StringBuilder &builder, const SiblingCount &root,
                                SerialisationState &) noexcept
  {
    // 2. If root is a numeric value, or a non-math function, serialize root per the normal rules for it and
    // return the result.

    builder.Append(ToString(root.Id));
    builder.Append(u8"()");
  }

  void SerialiseCalculationTree(StringBuilder &builder, const SiblingIndex &root,
                                SerialisationState &) noexcept
  {
    // 2. If root is a numeric value, or a non-math function, serialize root per the normal rules for it and
    // return the result.

    builder.Append(ToString(root.Id));
    builder.Append(u8"()");
  }

  void SerialiseCalculationTree(StringBuilder &builder, const IndirectNode<Sum> &root,
                                SerialisationState &state) noexcept
  {
    krys_debug_assert(!root->Children.empty());

    // 6. If root is a Sum node,

    // - let s be a string initially containing "(".
    builder.Append(state.OpenGroup());

    // - Sort root’s children.

    // NOTE: Rather than actually sorting the children, which we can't because they are immutable, we generate
    // a map of offsets to sorted offsets we can use while iterating.
    auto sortedChildrenMap = GenerateSortedChildrenMap(root->Children);

    {
      ParenthesisSaver saver {state};
      state.Grouping = SerialisationState::GroupingParenthesis::Include;

      // - Serialize root’s first child, and append it to s.
      SerialiseCalculationTree(builder, root->Children[sortedChildrenMap[0].Index], state);

      // - For each child of root beyond the first:
      for (size_t i = 1; i < root->Children.size(); ++i)
      {
        krys::SwitchOn(
          root->Children[sortedChildrenMap[i].Index],
          [&builder, &state](const IndirectNode<Negate> &child)
          {
            // 1. If child is a Negate node, append " - " to s, then serialize the Negate’s child and append
            // the result to s.
            builder.Append(u8" - ");
            SerialiseCalculationTree(builder, child->A, state);
          },
          [&builder, &state]<Numeric T>(const T &child)
          {
            // 2. If child is a negative numeric value, append " - " to s, then serialize the negation of
            // child as normal and append the result to s.
            if (child.Value < 0)
            {
              builder.Append(u8" - ");
              SerialiseCalculationTree(builder, CreateChildWithValueBasedOn(-child.Value, child), state);
              return;
            }

            // 3. Otherwise, append " + " to s, then serialize child and append the result to s.
            builder.Append(u8" + ");
            SerialiseCalculationTree(builder, child, state);
          },
          [&builder, &state](const auto &child)
          {
            // 3. Otherwise, append " + " to s, then serialize child and append the result to s.
            builder.Append(u8" + ");
            SerialiseCalculationTree(builder, child, state);
          });
      }
    }

    // - Finally, append ")" to s and return it.
    builder.Append(state.CloseGroup());
  }

  void SerialiseCalculationTree(StringBuilder &builder, const IndirectNode<Product> &root,
                                SerialisationState &state) noexcept
  {
    krys_debug_assert(!root->Children.empty());

    // 7. If root is a Product node,

    // - let s be a string initially containing "(".
    builder.Append(state.OpenGroup());

    // - Sort root’s children.

    // NOTE: Rather than actually sorting the children, which we can't because they are immutable, we generate
    // a map of offsets to sorted offsets we can use while iterating.
    auto sortedChildrenMap = GenerateSortedChildrenMap(root->Children);

    {
      ParenthesisSaver saver {state};
      state.Grouping = SerialisationState::GroupingParenthesis::Include;

      // - Serialize root’s first child, and append it to s.
      SerialiseCalculationTree(builder, root->Children[sortedChildrenMap[0].Index], state);

      // - For each child of root beyond the first:
      for (size_t i = 1; i < root->Children.size(); ++i)
      {
        krys::SwitchOn(
          root->Children[sortedChildrenMap[i].Index],
          [&builder, &state](const IndirectNode<Invert> &child)
          {
            // 1. If child is an Invert node, append " / " to s, then serialize the Invert’s child and append
            // the result to s.
            builder.Append(u8" / ");
            SerialiseCalculationTree(builder, child->A, state);
          },
          [&builder, &state](const auto &child)
          {
            // 2. Otherwise, append " * " to s, then serialize child and append the result to s.
            builder.Append(u8" * ");
            SerialiseCalculationTree(builder, child, state);
          });
      }
    }

    // - Finally, append ")" to s and return it.
    builder.Append(state.CloseGroup());
  }

  void SerialiseCalculationTree(StringBuilder &builder, const IndirectNode<Negate> &root,
                                SerialisationState &state) noexcept
  {
    // 4. If root is a Negate node,

    // - let s be a string initially containing "(-1 * ".
    builder.Append(state.OpenGroup());
    builder.Append(u8"-1 * ");

    {
      ParenthesisSaver saver {state};
      state.Grouping = SerialisationState::GroupingParenthesis::Include;

      // - Serialize root’s child, and append it to s.
      SerialiseCalculationTree(builder, root->A, state);
    }

    // - Append ")" to s, then return it.
    builder.Append(state.CloseGroup());
  }

  void SerialiseCalculationTree(StringBuilder &builder, const IndirectNode<Invert> &root,
                                SerialisationState &state) noexcept
  {
    // 5. If root is an Invert node,

    // - let s be a string initially containing "(1 / ".
    builder.Append(state.OpenGroup());
    builder.Append(u8"1 / ");

    {
      ParenthesisSaver saver {state};
      state.Grouping = SerialisationState::GroupingParenthesis::Include;

      // - Serialize root’s child, and append it to s.
      SerialiseCalculationTree(builder, root->A, state);
    }

    // - Append ")" to s, then return it.
    builder.Append(state.CloseGroup());
  }

  template <typename Op>
  void SerialiseCalculationTree(StringBuilder &builder, const IndirectNode<Op> &root,
                                SerialisationState &state) noexcept
  {
    // 3. If root is anything but a Sum, Negate, Product, or Invert node, serialize a math function for the
    // function corresponding to the node type, treating the node’s children as the function’s
    // comma-separated calculation arguments, and return the result.
    SerialiseMathFunction(builder, root, state);
  }

  // MARK: Exposed interface

  void SerialisationForCSS(StringBuilder &builder, const Tree &tree,
                           const SerialisationOptions &options) noexcept
  {
    SerialisationState state {
      .Stage = tree.Stage,
      .Range = options.Range,
      .SerialisationContext = options.SerialisationContext,
    };
    SerialiseMathFunction(builder, tree.Root, state);
  }

  CSSOMString SerialisationForCSS(const Tree &tree, const SerialisationOptions &options) noexcept
  {
    StringBuilder builder;
    SerialisationForCSS(builder, tree, options);
    return krys::move(builder).ToString();
  }

  void SerialisationForCSS(StringBuilder &builder, const Child &child,
                           const SerialisationOptions &options) noexcept
  {
    SerialisationState state {
      .Range = options.Range,
      .SerialisationContext = options.SerialisationContext,
    };
    SerialiseCalculationTree(builder, child, state);
  }

  CSSOMString SerialisationForCSS(const Child &child, const SerialisationOptions &options) noexcept
  {
    StringBuilder builder;
    SerialisationForCSS(builder, child, options);
    return krys::move(builder).ToString();
  }
}
