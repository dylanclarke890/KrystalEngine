/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
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

#include "Krystal.Booey/CSS/TypedOM/CSSImageValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"

namespace krys::boo::css
{
  Ref<CSSImageValue> CSSImageValue::Create(Ref<ImageValue> cssValue, dom::Document &document) noexcept
  {
    return AdoptRef(*new CSSImageValue(krys::move(cssValue), document));
  }

  CSSImageValue::CSSImageValue(Ref<ImageValue> cssValue, dom::Document &document) noexcept
      : _cssValue(krys::move(cssValue)), _document(CreateWeakPtr(&document))
  {
  }

  CSSImageValue::~CSSImageValue() noexcept = default;

  void CSSImageValue::Serialise(StringBuilder &builder, EnumSet<SerialisationArguments>) const noexcept
  {
    builder.Append(_cssValue->CssText(DefaultSerialisationContext()));
  }

  CachedImage *CSSImageValue::Image() const noexcept
  {
    return _cssValue->GetCachedImage();
  }

  dom::Document *CSSImageValue::Document() const noexcept
  {
    return _document.get();
  }

  RefPtr<Value> CSSImageValue::ToCSSValue() const noexcept
  {
    return _cssValue;
  }
}
