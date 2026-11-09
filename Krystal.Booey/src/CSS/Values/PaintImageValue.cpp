/*
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

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
