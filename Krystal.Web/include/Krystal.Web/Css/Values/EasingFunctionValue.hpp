/*
 * Copyright (C) 2007, 2008, 2012, 2016-2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Values/Easing/EasingFunction.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web::css
{
  /// @brief An `<easing-function>` value, e.g. `ease-in` or `steps(4, jump-start)`.
  class EasingFunctionValue final : public Value
  {
  private:
    web::css::EasingFunction _easingFunction;

    explicit EasingFunctionValue(web::css::EasingFunction &&easingFunction) noexcept;

  public:
    KRYS_NODISCARD static Ref<EasingFunctionValue> Create(web::css::EasingFunction easingFunction) noexcept;

    KRYS_NODISCARD const web::css::EasingFunction &GetEasingFunction() const noexcept
    {
      return _easingFunction;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const EasingFunctionValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::css::EasingFunctionValue)
  KRYS_NODISCARD static bool IsType(const krys::web::css::Value &value) noexcept
  {
    return value.Type() == krys::web::css::ValueType::EasingFunction;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
