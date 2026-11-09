/*
 * Copyright (C) 2018-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo::css
{
  namespace
  {
    void ApplyUASheetBehaviorsToContext(ParserContext &context) noexcept
    {
      context.thumbAndTrackPseudoElementsEnabled = true;
      context.cssInternalAutoBaseParsingEnabled = true;
    }
  }

  const ParserContext &StrictParserContext() noexcept
  {
    static MainThreadNeverDestroyed<ParserContext> strictContext(ParserMode::HTMLStandard);
    return strictContext;
  }

  ParserContext::ParserContext(ParserMode mode, const url::URL &baseURL) noexcept
      : BaseURL(baseURL), Mode(mode)
  {
    if (IsUASheetBehavior(mode))
    {
      ApplyUASheetBehaviorsToContext(*this);
    }

    StaticValuePool::Init();
  }
  ParserContext::ParserContext(const dom::Document &document) noexcept
      : ParserContext(document.CssParserContext())
  {
  }

  ParserContext::ParserContext(const dom::Document &document, const url::URL &sheetBaseURL,
                               utf8_stringview charset) noexcept
      : ParserContext(ParserMode::HTMLStandard)
  {
    BaseURL = sheetBaseURL.IsNull() ? document.BaseURL() : sheetBaseURL;
    Charset = charset;
    Mode = document.InQuirksMode() ? ParserMode::HTMLQuirks : ParserMode::HTMLStandard;
    IsHTMLDocument = Is<dom::HTMLDocument>(document);
    HasDocumentSecurityOrigin = true;

    auto &settings = document.GetSettings();
    useSystemAppearance = settings.UseSystemAppearance;
    springTimingFunctionEnabled = settings.SpringTimingFunctionEnabled;
    cssTransformStyleSeparatedEnabled = settings.CSSTransformStyleSeparatedEnabled;
    thumbAndTrackPseudoElementsEnabled = settings.ThumbAndTrackPseudoElementsEnabled;
    colorLayersEnabled = settings.CSSColorLayersEnabled;
    PropertySettings = css::PropertySettings(settings);
    // FIXME: The media text track display quirk, once documents have quirks.
  }

  void ParserContext::SetUASheetMode() noexcept
  {
    Mode = ParserMode::UASheet;
    ApplyUASheetBehaviorsToContext(*this);
  }

  size_t ParserContext::Hash() const noexcept
  {
    uint32 bits =
      static_cast<uint32>(IsHTMLDocument) << 0 | static_cast<uint32>(HasDocumentSecurityOrigin) << 1
      | static_cast<uint32>(useSystemAppearance) << 2 | static_cast<uint32>(springTimingFunctionEnabled) << 3
      | static_cast<uint32>(cssTransformStyleSeparatedEnabled) << 4
      | static_cast<uint32>(thumbAndTrackPseudoElementsEnabled) << 5
      | static_cast<uint32>(colorLayersEnabled) << 6
      | static_cast<uint32>(cssInternalAutoBaseParsingEnabled) << 7
      | static_cast<uint32>(webkitMediaTextTrackDisplayQuirkEnabled) << 8;
    return krys::Hash::Combine(BaseURL, Charset, PropertySettings, Mode, bits);
  }
}
