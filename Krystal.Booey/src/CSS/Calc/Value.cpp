#include "Krystal.Booey/CSS/Calc/Value.hpp"
#include "Krystal.Booey/CSS/Calc/TreeComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericCategory.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::css::calc
{
  Ref<Value> Value::Create(css::Category category, css::Range range, calc::Tree &&tree) noexcept
  {
    return AdoptRef(*new Value(category, range, krys::move(tree)));
  }

  Value::Value(css::Category category, css::Range range, calc::Tree &&tree) noexcept
      : _category {category}, _range {range}, _tree {krys::move(tree)}
  {
  }

  Value::~Value() noexcept = default;

  UnitType Value::PrimitiveType() const noexcept
  {
    // This returns the UnitType associated with the value returned by DoubleValue, or, if
    // UnitType::CalcPercentageWithLength, that a call to CreateCalculationValue() is needed.

    switch (_category)
    {
      case css::Category::Integer:
      {
        return UnitType::Integer;
      }
      case css::Category::Number:
      {
        return UnitType::Number;
      }
      case css::Category::Percentage:
      {
        return UnitType::Percentage;
      }
      case css::Category::Length:
      {
        return UnitType::px;
      }
      case css::Category::Angle:
      {
        return UnitType::deg;
      }
      case css::Category::Time:
      {
        return UnitType::s;
      }
      case css::Category::Frequency:
      {
        return UnitType::hz;
      }
      case css::Category::Resolution:
      {
        return UnitType::dppx;
      }
      case css::Category::Flex:
      {
        return UnitType::fr;
      }
      case css::Category::LengthPercentage:
      {
        if (!_tree.Type.PercentHint)
        {
          return UnitType::px;
        }

        if (krys::HoldsAlternative<Percentage>(_tree.Root))
        {
          return UnitType::Percentage;
        }

        return UnitType::CalcPercentageWithLength;
      }
      case css::Category::AnglePercentage:
      {
        if (!_tree.Type.PercentHint)
        {
          return UnitType::deg;
        }

        if (krys::HoldsAlternative<Percentage>(_tree.Root))
        {
          return UnitType::Percentage;
        }

        return UnitType::CalcPercentageWithAngle;
      }
    }

    krys_unreachable();
  }

  void Value::CollectComputedStyleDependencies(ComputedStyleDependencies &dependencies) const noexcept
  {
    calc::CollectComputedStyleDependencies(_tree, dependencies);
  }

  CSSOMString Value::CssText(const SerialisationContext &context) const noexcept
  {
    const auto options = SerialisationOptions {
      .Range = _range,
      .SerialisationContext = context,
    };
    return SerialisationForCSS(_tree, options);
  }

  bool Value::Equals(const Value &other) const noexcept
  {
    return _tree.Root == other._tree.Root;
  }

  double Value::ClampToPermittedRange(double value) const noexcept
  {
    // If a top-level calculation would produce a value whose numeric part is NaN,
    // it instead act as though the numeric part is 0.
    value = std::isnan(value) ? 0 : value;

    // If an <angle> must be converted due to exceeding the implementation-defined range of supported values,
    // it must be clamped to the nearest supported multiple of 360deg.
    if (_category == css::Category::Angle && std::isinf(value))
    {
      return 0;
    }

    if (_category == css::Category::Integer)
    {
      value = std::floor(value + 0.5);
    }

    return std::clamp(value, _range.Min, _range.Max);
  }
}
