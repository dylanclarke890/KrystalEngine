/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Values/FilterEffects/Filter.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web
{
  class StyleImage;
}

namespace krys::web::style
{
  class BuilderState;
}

namespace krys::web::css
{
  /// @brief The value of a `filter()` image function.
  class FilterImageValue final : public Value
  {
  private:
    const Ref<Value> _imageValueOrNone;
    web::css::Filter _filter;

    FilterImageValue(Ref<Value> &&imageValueOrNone, web::css::Filter &&filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<FilterImageValue> Create(Ref<Value> &&imageValueOrNone,
                                                       web::css::Filter &&filter) noexcept;

    KRYS_NODISCARD bool Equals(const FilterImageValue &other) const noexcept;

    KRYS_NODISCARD bool EqualInputImages(const FilterImageValue &other) const noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> CreateStyleImage(const style::BuilderState &state) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::css::FilterImageValue)
  KRYS_NODISCARD static bool IsType(const krys::web::css::Value &value) noexcept
  {
    return value.Type() == krys::web::css::ValueType::FilterImage;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
