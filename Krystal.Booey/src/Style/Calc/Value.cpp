#include "Krystal.Booey/Style/Calc/Value.hpp"
#include "Krystal.Booey/Style/Calc/TreeCopy.hpp"
#include "Krystal.Booey/Style/Calc/TreeEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include <algorithm>
#include <cmath>

namespace krys::boo::style::calc
{
  Ref<Value> Value::Create(css::Category category, css::Range range, calc::Tree &&tree) noexcept
  {
    return AdoptRef(*new Value(category, range, krys::move(tree)));
  }

  Value::Value(css::Category category, css::Range range, calc::Tree &&tree) noexcept
      : _category(category), _range(range), _tree(krys::move(tree))
  {
  }

  Value::~Value() noexcept = default;

  double Value::Evaluate(double percentResolutionLength, const ZoomFactor &usedZoom) const noexcept
  {
    auto result = calc::Evaluate(_tree, percentResolutionLength, usedZoom);
    if (std::isnan(result))
    {
      return 0;
    }
    return std::clamp(result, _range.Min, _range.Max);
  }

  double Value::Evaluate(double percentResolutionLength, const ZoomNeeded &zoomNeeded) const noexcept
  {
    auto result = calc::Evaluate(_tree, percentResolutionLength, zoomNeeded);
    if (std::isnan(result))
    {
      return 0;
    }
    return std::clamp(result, _range.Min, _range.Max);
  }

  calc::Tree Value::CopyTree() const noexcept
  {
    return calc::Copy(_tree);
  }

  Child Value::CopyRoot() const noexcept
  {
    auto tree = CopyTree();
    return {krys::move(tree.Root)};
  }

  bool Value::operator==(const Value &other) const noexcept
  {
    return _tree == other._tree;
  }
}
