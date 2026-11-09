/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  template <Numeric StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <DimensionPercentageNumeric StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <Calc StyleType>
  struct CSSValueCreation<StyleType>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueCreation<NumberOrPercentage<nR, pR, V>>
  {
    using StyleType = NumberOrPercentage<nR, pR, V>;

    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };

  template <css::Range nR, css::Range pR, typename V>
  struct CSSValueCreation<NumberOrPercentageResolvedToNumber<nR, pR, V>>
  {
    using StyleType = NumberOrPercentageResolvedToNumber<nR, pR, V>;

    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &pool, const RenderStyle &style,
                                              const StyleType &value) noexcept
    {
      return css::CreateValue(pool, ConvertToCSS(value, style));
    }
  };
}
