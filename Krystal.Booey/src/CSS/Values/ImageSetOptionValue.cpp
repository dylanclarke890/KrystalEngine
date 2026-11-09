/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"

namespace krys::boo::css
{
  ImageSetOptionValue::ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution) noexcept
      : Value(ValueType::ImageSetOption), _image(krys::move(image)), _resolution(krys::move(resolution))
  {
  }

  ImageSetOptionValue::ImageSetOptionValue(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution,
                                           CSSOMString &&type) noexcept
      : Value(ValueType::ImageSetOption), _image(krys::move(image)), _resolution(krys::move(resolution)),
        _mimeType(krys::move(type))
  {
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), PrimitiveValue::Create(1.0, UnitType::x)));
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image,
                                                       Ref<PrimitiveValue> &&resolution) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), krys::move(resolution)));
  }

  Ref<ImageSetOptionValue> ImageSetOptionValue::Create(Ref<Value> &&image, Ref<PrimitiveValue> &&resolution,
                                                       CSSOMString type) noexcept
  {
    krys_debug_assert(image->IsImageValue() || image->IsImageGeneratorValue());
    return AdoptRef(*new ImageSetOptionValue(krys::move(image), krys::move(resolution), krys::move(type)));
  }

  bool ImageSetOptionValue::Equals(const ImageSetOptionValue &other) const noexcept
  {
    if (!_image->Equals(*other._image))
    {
      return false;
    }

    if (!_resolution->Equals(*other._resolution))
    {
      return false;
    }

    if (_mimeType != other._mimeType)
    {
      return false;
    }

    return true;
  }

  CSSOMString ImageSetOptionValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    CSSOMString result;
    result.append(_image->CssText(context));
    result.push_back(u8' ');
    result.append(_resolution->CssText(context));
    if (_mimeType)
    {
      result.append(u8" type(\"");
      result.append(*_mimeType);
      result.append(u8"\")");
    }

    return result;
  }

  void ImageSetOptionValue::SetResolution(Ref<PrimitiveValue> &&resolution) noexcept
  {
    _resolution = krys::move(resolution);
  }

  void ImageSetOptionValue::SetType(CSSOMString type) noexcept
  {
    _mimeType = krys::move(type);
  }

  bool ImageSetOptionValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return _resolution->TraverseSubresources(handler) || _image->TraverseSubresources(handler);
  }
}
