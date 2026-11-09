/*
 * Copyright (C) 2013 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2021 Apple Inc. All right reserved.
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

#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  FilterImageValue::FilterImageValue(Ref<Value> &&imageValueOrNone, boo::css::Filter &&filter) noexcept
      : Value(ValueType::FilterImage), _imageValueOrNone(krys::move(imageValueOrNone)),
        _filter(krys::move(filter))
  {
  }

  Ref<FilterImageValue> FilterImageValue::Create(Ref<Value> &&imageValueOrNone,
                                                 boo::css::Filter &&filter) noexcept
  {
    return AdoptRef(*new FilterImageValue(krys::move(imageValueOrNone), krys::move(filter)));
  }

  bool FilterImageValue::Equals(const FilterImageValue &other) const noexcept
  {
    return EqualInputImages(other) && _filter == other._filter;
  }

  bool FilterImageValue::EqualInputImages(const FilterImageValue &other) const noexcept
  {
    return CompareValue(_imageValueOrNone, other._imageValueOrNone);
  }

  CSSOMString FilterImageValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder result;
    result.Append(u8"filter(");
    result.Append(_imageValueOrNone->CssText(context));
    result.Append(u8", ");
    Serialise(result, context, _filter);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  IterationStatus FilterImageValue::CustomVisitChildren(
    KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept
  {
    if (func(*_imageValueOrNone) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, _filter) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return IterationStatus::Continue;
  }

  RefPtr<StyleImage> FilterImageValue::CreateStyleImage(const style::BuilderState &state) const noexcept
  {
    return StyleFilterImage::Create(state.CreateStyleImage(*_imageValueOrNone),
                                    style::ConvertToStyle(_filter, state));
  }
}
