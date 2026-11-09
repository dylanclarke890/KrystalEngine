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

#include "Krystal.Booey/CSS/TypedOM/Color/CSSColor.hpp"

namespace krys::boo::css
{
  dom::ExceptionOr<Ref<CSSColor>> CSSColor::Create(CSSKeywordish colorSpace, List<CSSColorPercent> channels,
                                                   CSSNumberish alpha) noexcept
  {
    // https://drafts.css-houdini.org/css-typed-om/#dom-csscolor-csscolor
    List<RectifiedCSSColorComponent> rectifiedChannels;
    rectifiedChannels.reserve(channels.size());
    for (auto &channel : channels)
    {
      auto rectified = RectifyCSSColorPercent(krys::move(channel));
      if (rectified.HasException())
      {
        return rectified.ReleaseException();
      }
      rectifiedChannels.push_back(rectified.ReleaseValue());
    }
    return AdoptRef(*new CSSColor(CSSKeywordValue::RectifyKeywordish(krys::move(colorSpace)),
                                  krys::move(rectifiedChannels), krys::move(alpha)));
  }

  CSSColor::CSSColor(Ref<CSSKeywordValue> colorSpace, List<RectifiedCSSColorComponent> channels,
                     CSSNumberish alpha) noexcept
      : _colorSpace(krys::move(colorSpace)), _channels(krys::move(channels)), _alpha(krys::move(alpha))
  {
  }

  Ref<CSSKeywordValue> CSSColor::ColorSpace() const noexcept
  {
    return _colorSpace;
  }

  void CSSColor::SetColorSpace(CSSKeywordish colorSpace) noexcept
  {
    _colorSpace = CSSKeywordValue::RectifyKeywordish(krys::move(colorSpace));
  }

  List<CSSColorPercent> CSSColor::Channels() const noexcept
  {
    List<CSSColorPercent> channels;
    channels.reserve(_channels.size());
    for (const auto &channel : _channels)
    {
      channels.push_back(ToCSSColorComponent(channel));
    }
    return channels;
  }
}
