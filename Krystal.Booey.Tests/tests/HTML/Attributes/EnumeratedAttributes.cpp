#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("EnumeratedAttribute::ResolveState: keywords, then the empty and invalid value defaults",
            "[HTML][Attributes][html:empty-value-default][html:enumerated-attribute]")
  {
    using Hidden = EnumeratedAttribute<"hidden", HTMLElement>;
    CHECK(Hidden::ResolveState(null) == AttributeState::NotHidden);
    CHECK(Hidden::ResolveState(dom::DOMString(u8"hidden")) == AttributeState::Hidden);
    // Keywords match ASCII case-insensitively.
    CHECK(Hidden::ResolveState(dom::DOMString(u8"UNTIL-Found")) == AttributeState::UntilFound);
    CHECK(Hidden::ResolveState(dom::DOMString(u8"")) == AttributeState::Hidden);
    CHECK(Hidden::ResolveState(dom::DOMString(u8"bogus")) == AttributeState::Hidden);

    // An attribute without a missing value default is in no state when it's missing, and one without an
    // invalid value default is in no state when its value is invalid.
    using Popover = EnumeratedAttribute<"popover", HTMLElement>;
    CHECK(Popover::ResolveState(null) == null);
    CHECK(Popover::ResolveState(dom::DOMString(u8"")) == AttributeState::Auto);
    CHECK(Popover::ResolveState(dom::DOMString(u8"bogus")) == AttributeState::Manual);

    // The empty value default applies when the empty string isn't a keyword.
    using CrossOrigin = EnumeratedAttribute<"crossorigin", HTMLElement>;
    CHECK(CrossOrigin::ResolveState(null) == AttributeState::NoCORS);
    CHECK(CrossOrigin::ResolveState(dom::DOMString(u8"")) == AttributeState::Anonymous);
    CHECK(CrossOrigin::ResolveState(dom::DOMString(u8"Use-Credentials")) == AttributeState::UseCredentials);
    CHECK(CrossOrigin::ResolveState(dom::DOMString(u8"bogus")) == AttributeState::Anonymous);

    // A keyword wins over the empty value default.
    using Translate = EnumeratedAttribute<"translate", HTMLElement>;
    CHECK(Translate::ResolveState(dom::DOMString(u8"NO")) == AttributeState::No);
    CHECK(Translate::ResolveState(dom::DOMString(u8"")) == AttributeState::Yes);
    CHECK(Translate::ResolveState(dom::DOMString(u8"maybe")) == AttributeState::Inherit);
  }

  TEST_CASE("EnumeratedAttribute::ResolveCanonicalKeyword: the canonical keyword of the attribute's state",
            "[HTML][Attributes][html:canonical-keyword]")
  {
    using Popover = EnumeratedAttribute<"popover", HTMLElement>;
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(dom::DOMString(u8"AUTO")) == u8"auto");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(dom::DOMString(u8"")) == u8"auto");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(dom::DOMString(u8"bogus")) == u8"manual");
    CHECK(Popover::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(null) == null);

    // Explicitly given canonical keywords: "on" maps to the sentences state, whose keyword is "sentences".
    using Autocapitalize = EnumeratedAttribute<"autocapitalize", HTMLElement>;
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(dom::DOMString(u8"on")) == u8"sentences");
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(dom::DOMString(u8"off")) == u8"none");
    CHECK(Autocapitalize::ResolveCanonicalKeyword<dom::DOMString>(null) == u8"");

    using CrossOrigin = EnumeratedAttribute<"crossorigin", HTMLElement>;
    CHECK(CrossOrigin::ResolveCanonicalKeyword<Maybe<dom::DOMString>>(dom::DOMString(u8"")) == u8"anonymous");
  }
}
