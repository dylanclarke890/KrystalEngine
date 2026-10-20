#include "Krystal.Booey/CSS/Values/Primitives/URLModifiers.hpp"
#include "Krystal.Booey/Loader/ResourceLoaderOptions.hpp"

namespace krys::boo::css
{
  void ApplyModifiersToLoaderOptions(const URLModifiers &modifiers,
                                     ResourceLoaderOptions &loaderOptions) noexcept
  {
    if (modifiers.CrossOrigin)
    {
      // https://www.w3.org/TR/css-values-5/#typedef-request-url-modifier-cross-origin-modifier
      //
      // The URL request modifier steps for this modifier given request req are:
      // 1. Set request's mode to "cors".
      loaderOptions.RequestMode = FetchOptions::Mode::Cors;

      // 2. If the given value is use-credentials, set request's credentials mode to "include"
      if (std::holds_alternative<keywords::UseCredentials>(modifiers.CrossOrigin->Parameters))
      {
        loaderOptions.CredentialsMode = FetchOptions::Credentials::Include;
      }
      else
      {
        loaderOptions.CredentialsMode = FetchOptions::Credentials::SameOrigin;
      }
    }

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

    loaderOptions.IsLoadedFromOpaqueSource = modifiers.LoadedFromOpaqueSource;
  }
}
