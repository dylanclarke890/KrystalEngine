#include "Krystal.Booey/Style/Values/Math/MathDepth.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  MathDepth CSSValueConversion<MathDepth>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::AutoAdd)
    {
      return {state.ParentStyle().GetMathDepth().Value
              + (state.ParentStyle().GetMathStyle() == boo::MathStyle::Compact)};
    }
    if (auto *function = DynamicDowncast<css::FunctionValue>(value);
        function && function->Name() == css::ValueId::Add)
    {
      if (auto addFunction = RequiredListDowncast<css::FunctionValue, css::PrimitiveValue>(state, value))
      {
        const auto &first = addFunction->item(0);
        auto addend = ConvertCSSValueToStyle<MathDepth::Base>(state, first);
        return {state.ParentStyle().GetMathDepth().Value + addend.Value};
      }
    }
    return {ConvertCSSValueToStyle<MathDepth::Base>(state, value).Value};
  }
}
