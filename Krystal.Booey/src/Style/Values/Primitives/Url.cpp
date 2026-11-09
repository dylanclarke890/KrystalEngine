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

#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // Possible states/transitions:
  //
  // 1. empty (empty string)
  //      CSS:    [.specified = "", .resolved = "" ]
  //      Style:  [.resolved = ""]
  // 2. local (starts with #)
  //      CSS:    [.specified = "#foo", .resolved = "#foo"]
  //      Style:  [.resolved = "#foo"]
  // 3. relative, in stylesheet
  //      CSS:    [.specified = "foo/bar.png", .resolved = { stylesheet-base-url, "foo/bar.png" }]
  //      Style:  [.resolved = { stylesheet-base-url, "foo/bar.png" }]
  // 4. relative, in document with base-url
  //      CSS:    [.specified = "foo/bar.png", .resolved = { "foo/bar.png", document-base-url }]
  //      Style:  [.resolved = { document-base-url, "foo/bar.png" }]
  // 5. relative, in document without base-url
  //      CSS:    [.specified = "foo/bar.png", .resolved = null-url]
  //      Style:  [.resolved = "foo/bar.png"]

  URL ToStyleWithScriptExecutionContext(const css::CSSURL &url, const dom::Document &context) noexcept
  {
    if (url.Resolved.IsNull())
    {
      return {
        .Resolved = context.CompleteURL(url.Specified),
        .Modifiers = url.Modifiers,
      };
    }

    return {
      .Resolved = url.Resolved,
      .Modifiers = url.Modifiers,
    };
  }

  css::CSSURL ToCSS<URL>::operator()(const URL &url, const RenderStyle &) noexcept
  {
    return {
      .Specified = url.Resolved.String(),
      .Resolved = url.Resolved,
      .Modifiers = url.Modifiers,
    };
  }

  URL ToStyle<css::CSSURL>::operator()(const css::CSSURL &url, const BuilderState &state) noexcept
  {
    return ToStyleWithScriptExecutionContext(url, state.GetDocument());
  }

  Ref<css::Value> CSSValueCreation<URL>::operator()(css::ValuePool &, const RenderStyle &style,
                                                    const URL &value) noexcept
  {
    return css::UrlValue::Create(ConvertToCSS(value, style));
  }

  URL CSSValueConversion<URL>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    if (auto *url = RequiredDowncast<css::UrlValue>(state, value))
    {
      return ConvertToStyle(url->URL(), state);
    }
    return {.Resolved = url::URL {u8""}, .Modifiers = {}};
  }

  // MARK: - Serialisation

  void Serialiser<URL>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                   const RenderStyle &style, const URL &value) noexcept
  {
    css::Serialise(builder, context, ConvertToCSS(value, style));
  }
}
