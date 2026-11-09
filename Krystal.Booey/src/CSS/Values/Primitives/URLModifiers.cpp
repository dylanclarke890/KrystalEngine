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

#include "Krystal.Booey/CSS/Values/Primitives/URLModifiers.hpp"
#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"

namespace krys::boo::css
{
  void ApplyModifiersToLoaderOptions(const URLModifiers &modifiers,
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
        [](keywords::NoReferrer) { return boo::ReferrerPolicy::NoReferrer; },
        [](keywords::NoReferrerWhenDowngrade) { return boo::ReferrerPolicy::NoReferrerWhenDowngrade; },
        [](keywords::SameOrigin) { return boo::ReferrerPolicy::SameOrigin; },
        [](keywords::Origin) { return boo::ReferrerPolicy::Origin; }, [](keywords::StrictOrigin)
        { return boo::ReferrerPolicy::StrictOrigin; }, [](keywords::OriginWhenCrossOrigin)
        { return boo::ReferrerPolicy::OriginWhenCrossOrigin; }, [](keywords::StrictOriginWhenCrossOrigin)
        { return boo::ReferrerPolicy::StrictOriginWhenCrossOrigin; },
        [](keywords::UnsafeUrl) { return boo::ReferrerPolicy::UnsafeUrl; });
    }
  }
}
