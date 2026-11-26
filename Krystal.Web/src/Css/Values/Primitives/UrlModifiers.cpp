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

#include "Krystal.Web/Css/Values/Primitives/UrlModifiers.hpp"
#include "Krystal.Web/Loader/ResourceLoaderOptions.hpp"

namespace krys::web::css
{
  void ApplyModifiersToLoaderOptions(const UrlModifiers &modifiers,
                                     ResourceLoaderOptions &loaderOptions) noexcept
  {
    // https://www.w3.org/TR/css-values-5/#typedef-request-url-modifier-cross-origin-modifier
    // (Skipped: there's no request mode or credentials mode without a network.)

    if (modifiers.Integrity)
    {
      // https://www.w3.org/TR/css-values-5/#typedef-request-url-modifier-integrity-modifier
      //
      // The URL request modifier steps for this modifier given request req are to set
      // request's integrity metadata to the given <string>.
      loaderOptions.Integrity = modifiers.Integrity->Parameters;
    }

    if (modifiers.ReferrerPolicy)
    {
      // https://www.w3.org/TR/css-values-5/#typedef-request-url-modifier-referrer-policy-modifier
      //
      // The URL request modifier steps for this modifier given request req are to set
      // request's referrer policy to the ReferrerPolicy that matches the given value.
      loaderOptions.RequestReferrerPolicy = krys::SwitchOn(
        modifiers.ReferrerPolicy->Parameters,
        [](keywords::NoReferrer) { return web::ReferrerPolicy::NoReferrer; },
        [](keywords::NoReferrerWhenDowngrade) { return web::ReferrerPolicy::NoReferrerWhenDowngrade; },
        [](keywords::SameOrigin) { return web::ReferrerPolicy::SameOrigin; },
        [](keywords::Origin) { return web::ReferrerPolicy::Origin; }, [](keywords::StrictOrigin)
        { return web::ReferrerPolicy::StrictOrigin; }, [](keywords::OriginWhenCrossOrigin)
        { return web::ReferrerPolicy::OriginWhenCrossOrigin; }, [](keywords::StrictOriginWhenCrossOrigin)
        { return web::ReferrerPolicy::StrictOriginWhenCrossOrigin; },
        [](keywords::UnsafeUrl) { return web::ReferrerPolicy::UnsafeUrl; });
    }
  }
}
