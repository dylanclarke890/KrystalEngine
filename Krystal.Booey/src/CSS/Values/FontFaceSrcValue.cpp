/*
 * Copyright (C) 2007-2022 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/FontFaceSrcValue.hpp"
#include "Krystal.Booey/CSS/CSSOM/Markup.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Fonts/FontCustomPlatformData.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFont.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontLoadRequest.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  FontFaceSrcLocalValue::FontFaceSrcLocalValue(CSSOMStringAtom &&fontFaceName) noexcept
      : Value(ValueType::FontFaceSrcLocal), _fontFaceName(krys::move(fontFaceName))
  {
  }

  Ref<FontFaceSrcLocalValue> FontFaceSrcLocalValue::Create(CSSOMStringAtom fontFaceName) noexcept
  {
    return AdoptRef(*new FontFaceSrcLocalValue(krys::move(fontFaceName)));
  }

  CSSOMString FontFaceSrcLocalValue::CustomCSSText(const SerialisationContext &) const noexcept
  {
    StringBuilder result;
    result.Append(u8"local(");
    SerialiseString(CSSOMString(_fontFaceName.View()), result);
    result.Append(u8')');
    return krys::move(result).ToString();
  }

  bool FontFaceSrcLocalValue::Equals(const FontFaceSrcLocalValue &other) const noexcept
  {
    return _fontFaceName == other._fontFaceName;
  }

  FontFaceSrcResourceValue::FontFaceSrcResourceValue(CSSURL &&location, CSSOMString &&format,
                                                     List<FontTechnology> &&technologies) noexcept
      : Value(ValueType::FontFaceSrcResource), _location(Resolve(krys::move(location))),
        _format(krys::move(format)), _technologies(krys::move(technologies))
  {
  }

  FontFaceSrcResourceValue::~FontFaceSrcResourceValue() noexcept = default;

  Ref<FontFaceSrcResourceValue> FontFaceSrcResourceValue::Create(CSSURL location, CSSOMString format,
                                                                 List<FontTechnology> &&technologies) noexcept
  {
    return AdoptRef(
      *new FontFaceSrcResourceValue(krys::move(location), krys::move(format), krys::move(technologies)));
  }

  RefPtr<FontLoadRequest>
    FontFaceSrcResourceValue::GetFontLoadRequest(dom::Document &context,
                                                 bool isInitiatingElementInUserAgentShadowTree) noexcept
  {
    if (_cachedFont)
    {
      return CachedFontLoadRequest::Create(*_cachedFont);
    }

    bool isFormatSVG;
    if (_format.empty())
    {
      // In order to avoid conflicts with the old WinIE style of font-face, if there is no format specified,
      // we check to see if the URL ends with .eot. We will not try to load those.
      auto lastPathComponent = _location.Resolved.LastPathComponent();
      constexpr utf8_stringview eot = u8".eot";
      if (lastPathComponent.size() >= eot.size()
          && text::IsASCIICaselessEqual(lastPathComponent.substr(lastPathComponent.size() - eot.size()), eot)
          && !_location.Resolved.ProtocolIsData())
      {
        return nullptr;
      }
      isFormatSVG = false;
    }
    else
    {
      isFormatSVG = text::IsASCIICaselessEqual(_format, u8"svg");
      if (!FontCustomPlatformData::SupportsFormat(_format))
      {
        return nullptr;
      }
    }

    for (auto technology : _technologies)
    {
      if (!FontCustomPlatformData::SupportsTechnology(technology))
      {
        return nullptr;
      }
    }

    RefPtr<FontLoadRequest> request = context.GetFontLoadRequest(_location.Resolved.String(), isFormatSVG,
                                                                 isInitiatingElementInUserAgentShadowTree);
    if (auto *cachedRequest = DynamicDowncast<CachedFontLoadRequest>(request.get()))
    {
      _cachedFont = &cachedRequest->GetCachedFont();
    }

    return request;
  }

  bool FontFaceSrcResourceValue::CustomTraverseSubresources(
    KRYS_NOESCAPE const Func<bool(const CachedResource &)> &handler) const noexcept
  {
    return _cachedFont && handler(*_cachedFont);
  }

  bool FontFaceSrcResourceValue::CustomMayDependOnBaseUrl() const noexcept
  {
    return MayDependOnBaseURL(_location);
  }

  CSSOMString FontFaceSrcResourceValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    StringBuilder builder;

    Serialise(builder, context, _location);

    if (!_format.empty())
    {
      builder.Append(u8" format(");
      SerialiseString(_format, builder);
      builder.Append(u8')');
    }
    if (!_technologies.empty())
    {
      builder.Append(u8" tech(");
      for (size_t i = 0uz; i < _technologies.size(); ++i)
      {
        if (i)
        {
          builder.Append(u8", ");
        }
        builder.Append(CssTextFromFontTech(_technologies[i]));
      }
      builder.Append(u8')');
    }

    return krys::move(builder).ToString();
  }

  bool FontFaceSrcResourceValue::Equals(const FontFaceSrcResourceValue &other) const noexcept
  {
    return _location == other._location && _format == other._format && _technologies == other._technologies;
  }
}
