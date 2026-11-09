/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSColorValue.hpp"
#include "Krystal.Booey/CSS/TypedOM/CSSUnitValue.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::css
{
  namespace
  {
    /// @brief Rectifies a colour component: a number is @p numberToValue's value, a numeric value must match
    /// @p matches, and a keyword must be `none`.
    template <typename TNumberToValue, typename TMatches>
    KRYS_NODISCARD dom::ExceptionOr<RectifiedCSSColorComponent> RectifyCSSColorComponent(
      Variant<double, RefPtr<CSSNumericValue>, CSSOMString, RefPtr<CSSKeywordValue>> component,
      const TNumberToValue &numberToValue, const TMatches &matches) noexcept
    {
      auto keyword = [](Ref<CSSKeywordValue> value) -> dom::ExceptionOr<RectifiedCSSColorComponent>
      {
        if (!infra::StringAlgorithms::ASCIICaseInsensitiveMatch(value->Value(), u8"none"))
        {
          return dom::Exception {dom::ExceptionCode::SyntaxError};
        }
        return RectifiedCSSColorComponent {RefPtr<CSSKeywordValue>(krys::move(value))};
      };
      return krys::SwitchOn(
        krys::move(component), [&](double value) -> dom::ExceptionOr<RectifiedCSSColorComponent>
        { return RectifiedCSSColorComponent {RefPtr<CSSNumericValue>(numberToValue(value))}; },
        [&](RefPtr<CSSNumericValue> &&value) -> dom::ExceptionOr<RectifiedCSSColorComponent>
        {
          if (!value || !matches(value->Type()))
          {
            return dom::Exception {dom::ExceptionCode::SyntaxError};
          }
          return RectifiedCSSColorComponent {krys::move(value)};
        },
        [&](CSSOMString &&value) { return keyword(CSSKeywordValue::RectifyKeywordish(krys::move(value))); },
        [&](RefPtr<CSSKeywordValue> &&value) -> dom::ExceptionOr<RectifiedCSSColorComponent>
        {
          if (!value)
          {
            return dom::Exception {dom::ExceptionCode::SyntaxError};
          }
          return keyword(Ref<CSSKeywordValue>(krys::move(value)));
        });
    }
  }

  dom::ExceptionOr<RectifiedCSSColorComponent>
    CSSColorValue::RectifyCSSColorPercent(CSSColorPercent percent) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorpercent
    return RectifyCSSColorComponent(
      krys::move(percent),
      [](double value) { return CSSUnitValue::Create(value * 100, UnitType::Percentage); },
      [](const CSSNumericType &type) { return type.Matches<CSSNumericBaseType::Percent>(); });
  }

  dom::ExceptionOr<RectifiedCSSColorComponent>
    CSSColorValue::RectifyCSSColorAngle(CSSColorAngle angle) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolorangle
    return RectifyCSSColorComponent(
      krys::move(angle), [](double value) { return CSSUnitValue::Create(value, UnitType::deg); },
      [](const CSSNumericType &type) { return type.Matches<CSSNumericBaseType::Angle>(); });
  }

  dom::ExceptionOr<RectifiedCSSColorComponent>
    CSSColorValue::RectifyCSSColorNumber(CSSColorNumber number) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#rectify-a-csscolornumber
    return RectifyCSSColorComponent(
      krys::move(number), [](double value) { return CSSUnitValue::Create(value, UnitType::Number); },
      [](const CSSNumericType &type) { return type.MatchesNumber(); });
  }

  CSSColorPercent CSSColorValue::ToCSSColorComponent(const RectifiedCSSColorComponent &component) noexcept
  {
    return krys::SwitchOn(
      component, [](const RefPtr<CSSNumericValue> &value) { return CSSColorPercent {value}; },
      [](const RefPtr<CSSKeywordValue> &value) { return CSSColorPercent {value}; });
  }

  CSSColorPercent CSSColorValue::ToCSSColorPercent(const CSSNumberish &numberish) noexcept
  {
    return krys::SwitchOn(
      numberish, [](double value) { return CSSColorPercent {value}; },
      [](const RefPtr<CSSNumericValue> &value) { return CSSColorPercent {value}; });
  }

  Ref<CSSKeywordValue> CSSColorValue::ColorSpaceKeyword(utf8_stringview colorSpace) noexcept
  {
    return CSSKeywordValue::RectifyKeywordish(CSSOMString(colorSpace));
  }

  RefPtr<Value> CSSColorValue::ToCSSValue() const noexcept
  {
    return nullptr;
  }
}
