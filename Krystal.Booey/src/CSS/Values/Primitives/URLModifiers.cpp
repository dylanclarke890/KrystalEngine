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
