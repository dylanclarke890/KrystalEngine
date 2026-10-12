#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"

namespace krys::boo::css::calc
{
  Child CreateNumeric(double value, UnitType unit) noexcept
  {
    switch (unit)
    {
      // Number
      case UnitType::Number:
      case UnitType::Integer:
      {
        return CreateChild(Number {.Value = value});
      }
      // Percentage
      case UnitType::Percentage:
      {
        return CreateChild(Percentage {.Value = value, .Hint = {}});
      }
      // Canonical Dimension
      case UnitType::px:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Length});
      }
      case UnitType::deg:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Angle});
      }
      case UnitType::s:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Time});
      }
      case UnitType::hz:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Frequency});
      }
      case UnitType::dppx:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Resolution});
      }
      case UnitType::fr:
      {
        return CreateChild(CanonicalDimension {.Value = value, .Dimension = Dimension::Flex});
      }
      // <length>
      case UnitType::cm:
      case UnitType::mm:
      case UnitType::Q:
      case UnitType::in:
      case UnitType::pt:
      case UnitType::pc:
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
      // <angle>
      case UnitType::rad:
      case UnitType::grad:
      case UnitType::turn:
      // <time>
      case UnitType::ms:
      // <frequency>
      case UnitType::khz:
      // <resolution>
      case UnitType::x:
      case UnitType::dpi:
      case UnitType::dpcm:
      {
        return CreateChild(NonCanonicalDimension {.Value = value, .Unit = unit});
      }
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

    krys_unreachable();
  }

  Type GetType(Dimension dimension) noexcept
  {
    switch (dimension)
    {
      case Dimension::Length:     return Type {.Length = 1};
      case Dimension::Angle:      return Type {.Angle = 1};
      case Dimension::Time:       return Type {.Time = 1};
      case Dimension::Frequency:  return Type {.Frequency = 1};
      case Dimension::Resolution: return Type {.Resolution = 1};
      case Dimension::Flex:       return Type {.Flex = 1};
    }

    krys_unreachable();
  }

  Type GetType(const Number &) noexcept
  {
    return Type {};
  }

  Type GetType(const Percentage &root) noexcept
  {
    auto type = Type {.Percent = 1};
    if (root.Hint)
    {
      type.ApplyPercentHint(*root.Hint);
    }

    return type;
  }

  Type GetType(const CanonicalDimension &root) noexcept
  {
    return GetType(root.Dimension);
  }

  Type GetType(const NonCanonicalDimension &root) noexcept
  {
    return Type::DetermineType(ToUnit(root));
  }

  Type GetType(const Symbol &root) noexcept
  {
    return Type::DetermineType(root.Unit);
  }

  Type GetType(const SiblingCount &) noexcept
  {
    return Type {};
  }

  Type GetType(const SiblingIndex &) noexcept
  {
    return Type {};
  }

  Type GetType(const Child &child) noexcept
  {
    return krys::SwitchOn(child, [&](const auto &root) { return GetType(root); });
  }

  Maybe<Type> ToType(const Sum &root) noexcept
  {
    Maybe<Type> type = GetType(root.Children[0]);
    for (size_t i = 1; i < root.Children.size(); ++i)
    {
      type = Type::Add(type, GetType(root.Children[i]));
    }

    return type;
  }

  Maybe<Type> ToType(const Product &root) noexcept
  {
    Maybe<Type> type = GetType(root.Children[0]);
    for (size_t i = 1; i < root.Children.size(); ++i)
    {
      type = Type::Multiply(type, GetType(root.Children[i]));
    }

    return type;
  }

  Maybe<Type> ToType(const Negate &root) noexcept
  {
    return GetType(root.A);
  }

  Maybe<Type> ToType(const Invert &root) noexcept
  {
    return Type::Invert(GetType(root.A));
  }

  // Utilities to deduce the right input/merge/output policies from the operation.

  template <typename Op>
  static Maybe<Type> GetValidatedTypeFor(const Op &, const Child &child) noexcept
  {
    auto type = GetType(child);
    if (ValidateType<Op::Input>(type))
    {
      return type;
    }

    return null;
  }

  template <typename Op>
  static Maybe<Type> MergeTypesFor(const Op &, Maybe<Type> a, Maybe<Type> b) noexcept
  {
    return MergeTypes<Op::Merge>(a, b);
  }

  template <typename Op, typename... Args>
  static Maybe<Type> TransformTypeFor(const Op &, Maybe<Type> a) noexcept
  {
    return TransformType<Op::Output>(a);
  }

  Maybe<Type> ToType(const Min &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Children[0]);
    for (size_t i = 1; i < root.Children.size(); ++i)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.Children[i]));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Max &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Children[0]);
    for (size_t i = 1; i < root.Children.size(); ++i)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.Children[i]));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Clamp &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Val);
    if (krys::HoldsAlternative<Child>(root.Min))
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, std::get<Child>(root.Min.Value)));
    }

    if (krys::HoldsAlternative<Child>(root.Max))
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, std::get<Child>(root.Max.Value)));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const RoundNearest &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.A);
    if (root.B)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.B));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const RoundUp &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.A);
    if (root.B)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.B));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const RoundDown &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.A);
    if (root.B)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.B));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const RoundToZero &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.A);
    if (root.B)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.B));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Mod &root) noexcept
  {
    return TransformTypeFor(
      root, MergeTypesFor(root, GetValidatedTypeFor(root, root.A), GetValidatedTypeFor(root, root.B)));
  }

  Maybe<Type> ToType(const Rem &root) noexcept
  {
    return TransformTypeFor(
      root, MergeTypesFor(root, GetValidatedTypeFor(root, root.A), GetValidatedTypeFor(root, root.B)));
  }

  Maybe<Type> ToType(const Sin &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Cos &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Tan &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Asin &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Acos &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Atan &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Atan2 &root) noexcept
  {
    return TransformTypeFor(
      root, MergeTypesFor(root, GetValidatedTypeFor(root, root.A), GetValidatedTypeFor(root, root.B)));
  }

  Maybe<Type> ToType(const Pow &root) noexcept
  {
    return TransformTypeFor(
      root, MergeTypesFor(root, GetValidatedTypeFor(root, root.A), GetValidatedTypeFor(root, root.B)));
  }

  Maybe<Type> ToType(const Sqrt &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Hypot &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Children[0]);

    for (size_t i = 1; i < root.Children.size(); ++i)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.Children[i]));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Log &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.A);

    if (root.B)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.B));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Exp &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Abs &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Sign &root) noexcept
  {
    return TransformTypeFor(root, GetValidatedTypeFor(root, root.A));
  }

  Maybe<Type> ToType(const Random &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Min);
    type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.Max));

    if (root.Step)
    {
      type = MergeTypesFor(root, type, GetValidatedTypeFor(root, *root.Step));
    }

    return TransformTypeFor(root, type);
  }

  Maybe<Type> ToType(const Progress &root) noexcept
  {
    auto type = GetValidatedTypeFor(root, root.Value);
    type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.Start));
    type = MergeTypesFor(root, type, GetValidatedTypeFor(root, root.End));

    return TransformTypeFor(root, type);
  }
}