#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/Rendering/Style/StylePaintImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::css
{
  PaintImageValue::PaintImageValue(CSSOMString &&name, Ref<VariableData> &&arguments) noexcept
      : Value(ValueType::PaintImage), _name(krys::move(name)), _arguments(krys::move(arguments))
  {
  }

  Ref<PaintImageValue> PaintImageValue::Create(CSSOMString name, Ref<VariableData> arguments) noexcept
  {
    return AdoptRef(*new PaintImageValue(krys::move(name), krys::move(arguments)));
  }

  CSSOMString PaintImageValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    CSSOMString result {u8"paint("};
    result.append(_name);
    // Each argument as Chrome keeps it: between the commas, without the whitespace around it.
    auto arguments = _arguments->TokenRange();
    while (!arguments.IsAtEnd())
    {
      arguments.DiscardWhitespace();
      auto argument = arguments;
      while (!arguments.IsAtEnd() && arguments.Peek().Type() != TokenType::Comma)
      {
        arguments.DiscardComponentValue();
      }
      auto range = argument.RangeUntil(arguments);
      range.DiscardTrailingWhitespace();
      result.append(u8", ");
      result.append(range.Serialise(TokenSerialisationMode::CustomProperty));
      if (!arguments.IsAtEnd())
      {
        arguments.Discard();
      }
    }
    result.push_back(u8')');
    return result;
  }

  RefPtr<StyleImage> PaintImageValue::CreateStyleImage(const style::BuilderState &) const noexcept
  {
    return StylePaintImage::Create(_name, _arguments);
  }
}
