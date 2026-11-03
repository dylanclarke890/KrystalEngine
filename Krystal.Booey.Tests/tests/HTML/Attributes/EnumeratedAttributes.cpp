#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/HTMLIFrameElement.hpp"
#include "Krystal.Booey/HTML/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/HTMLLinkElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("EnumeratedAttribute::ResolveState: keywords, then the empty and invalid value defaults",
            "[HTML][Attributes][html:empty-value-default][html:enumerated-attribute]")
  {
    using Hidden = EnumeratedAttribute<"hidden", HTMLElement>;
    CHECK(Hidden::ResolveState(null) == AttributeState::NotHidden);
    CHECK(Hidden::ResolveState(u8"hidden") == AttributeState::Hidden);
    // Keywords match ASCII case-insensitively.
    CHECK(Hidden::ResolveState(u8"UNTIL-Found") == AttributeState::UntilFound);
    CHECK(Hidden::ResolveState(u8"") == AttributeState::Hidden);
    CHECK(Hidden::ResolveState(u8"bogus") == AttributeState::Hidden);

    // An attribute without a missing value default is in no state when it's missing, and one without an
    // invalid value default is in no state when its value is invalid.
    using Popover = EnumeratedAttribute<"popover", HTMLElement>;
    CHECK(Popover::ResolveState(null) == null);
    CHECK(Popover::ResolveState(u8"") == AttributeState::Auto);
    CHECK(Popover::ResolveState(u8"bogus") == AttributeState::Manual);

    // The empty value default applies when the empty string isn't a keyword.
    using CrossOrigin = EnumeratedAttribute<"crossorigin", HTMLElement>;
    CHECK(CrossOrigin::ResolveState(null) == AttributeState::NoCORS);
    CHECK(CrossOrigin::ResolveState(u8"") == AttributeState::Anonymous);
    CHECK(CrossOrigin::ResolveState(u8"Use-Credentials") == AttributeState::UseCredentials);
    CHECK(CrossOrigin::ResolveState(u8"bogus") == AttributeState::Anonymous);

    // A keyword wins over the empty value default.
    using Translate = EnumeratedAttribute<"translate", HTMLElement>;
    CHECK(Translate::ResolveState(u8"NO") == AttributeState::No);
    CHECK(Translate::ResolveState(u8"") == AttributeState::Yes);
    CHECK(Translate::ResolveState(u8"maybe") == AttributeState::Inherit);
  }

  TEST_CASE("EnumeratedAttribute::ResolveCanonicalKeyword: the canonical keyword of the attribute's state",
            "[HTML][Attributes][html:canonical-keyword]")
  {
    using Popover = EnumeratedAttribute<"popover", HTMLElement>;
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(u8"AUTO") == u8"auto");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(u8"") == u8"auto");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(u8"bogus") == u8"manual");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(null) == null);

    // Explicitly given canonical keywords: "on" maps to the sentences state, whose keyword is "sentences".
    using Autocapitalize = EnumeratedAttribute<"autocapitalize", HTMLElement>;
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(u8"on") == u8"sentences");
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(u8"off") == u8"none");
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(null) == u8"");

    using CrossOrigin = EnumeratedAttribute<"crossorigin", HTMLElement>;
    CHECK(CrossOrigin::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(u8"") == u8"anonymous");
  }

  namespace
  {
    template <typename TElement>
    KRYS_NODISCARD Ref<TElement> Create(dom::Document &document, dom::DOMStringView name,
                                        dom::DOMStringView attribute = {}, Maybe<dom::DOMString> value = null)
    {
      auto element = document.CreateElement(name).ReleaseValue();
      if (value)
      {
        REQUIRE_FALSE(element->SetAttribute(attribute, krys::move(*value)).HasException());
      }
      return ShareRef(Downcast<TElement>(Downcast<HTMLElement>(*element)));
    }

    template <typename TElement>
    void CheckCrossOriginAndFetchPriority(dom::Document &document, dom::DOMStringView name)
    {
      INFO(reinterpret_cast<const char *>(name.data()));
      CHECK(Create<TElement>(document, name)->CrossOrigin() == null);
      CHECK(Create<TElement>(document, name, u8"crossorigin", dom::DOMString())->CrossOrigin()
            == u8"anonymous");
      CHECK(Create<TElement>(document, name, u8"crossorigin", u8"bogus")->CrossOrigin() == u8"anonymous");
      CHECK(Create<TElement>(document, name, u8"crossorigin", u8"USE-Credentials")->CrossOrigin()
            == u8"use-credentials");
      // Setting it sets the content attribute.
      auto element = Create<TElement>(document, name);
      element->CrossOrigin(u8"use-credentials");
      CHECK(element->GetAttribute(u8"crossorigin") == u8"use-credentials");
      element->CrossOrigin(null);
      CHECK_FALSE(element->HasAttribute(u8"crossorigin"));

      CHECK(Create<TElement>(document, name)->FetchPriority() == u8"auto");
      CHECK(Create<TElement>(document, name, u8"fetchpriority", u8"HIGH")->FetchPriority() == u8"high");
      CHECK(Create<TElement>(document, name, u8"fetchpriority", u8"urgent")->FetchPriority() == u8"auto");
    }

    template <typename TElement>
    void CheckReferrerPolicy(dom::Document &document, dom::DOMStringView name)
    {
      INFO(reinterpret_cast<const char *>(name.data()));
      CHECK(Create<TElement>(document, name)->ReferrerPolicy() == u8"");
      CHECK(Create<TElement>(document, name, u8"referrerpolicy", u8"No-Referrer")->ReferrerPolicy()
            == u8"no-referrer");
      CHECK(Create<TElement>(document, name, u8"referrerpolicy",
                             dom::DOMString(u8"strict-origin-when-cross-origin"))
              ->ReferrerPolicy()
            == u8"strict-origin-when-cross-origin");
      CHECK(Create<TElement>(document, name, u8"referrerpolicy", u8"bogus")->ReferrerPolicy() == u8"");
    }
  }

  TEST_CASE(
    "CORS settings and fetch priority attributes: crossOrigin and fetchPriority reflect their known values",
    "[HTML][Attributes][html:cors-settings-attribute][html:attr-crossorigin-none]"
    "[html:fetch-priority-attribute][html:dom-img-crossorigin][html:dom-img-fetchpriority]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    CheckCrossOriginAndFetchPriority<HTMLImageElement>(*document, u8"img");
    CheckCrossOriginAndFetchPriority<HTMLLinkElement>(*document, u8"link");
    CheckCrossOriginAndFetchPriority<HTMLScriptElement>(*document, u8"script");

    // A script's crossOrigin isn't its type.
    auto script = Create<HTMLScriptElement>(*document, u8"script", u8"type", u8"module");
    CHECK(script->CrossOrigin() == null);
  }

  TEST_CASE("Referrer policy attributes: referrerPolicy reflects its known values",
            "[HTML][Attributes][html:referrer-policy-attribute][html:dom-a-referrerpolicy]"
            "[html:dom-img-referrerpolicy][html:dom-iframe-referrerpolicy]"
            "[html:dom-area-referrerpolicy]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    CheckReferrerPolicy<HTMLAnchorElement>(*document, u8"a");
    CheckReferrerPolicy<HTMLAreaElement>(*document, u8"area");
    CheckReferrerPolicy<HTMLIFrameElement>(*document, u8"iframe");
    CheckReferrerPolicy<HTMLImageElement>(*document, u8"img");
    CheckReferrerPolicy<HTMLLinkElement>(*document, u8"link");
    CheckReferrerPolicy<HTMLScriptElement>(*document, u8"script");
  }
}
