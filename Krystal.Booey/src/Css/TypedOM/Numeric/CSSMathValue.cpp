/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Css/TypedOM/Numeric/CSSMathValue.hpp"
#include "Krystal.Booey/Css/Calc/Tree.hpp"
#include "Krystal.Booey/Css/Calc/Value.hpp"
#include "Krystal.Booey/Css/Values/PrimitiveValue.hpp"

namespace krys::boo
{
  RefPtr<css::Value> CSSMathValue::ToCSSValue() const noexcept
  {
    auto node = ToCalcTreeNode();
    if (!node)
    {
      return nullptr;
    }

    auto type = css::calc::GetType(*node);
    auto category = type.CalculationCategory();
    if (!category)
    {
      return nullptr;
    }

    return css::PrimitiveValue::Create(css::calc::Value::Create(*category, css::All,
                                                                css::calc::Tree {
                                                                  .Root = krys::move(*node),
                                                                  .Type = type,
                                                                  .Stage = css::calc::Stage::Specified,
                                                                }));
  }
}
