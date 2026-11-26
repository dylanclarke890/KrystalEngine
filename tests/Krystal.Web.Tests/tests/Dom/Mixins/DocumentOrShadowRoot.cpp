#include "Krystal.Web/Dom/Mixins/DocumentOrShadowRoot.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Html/CustomElementTestUtils.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Html/Window.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE_METHOD(html::tests::CustomElementFixture,
                   "DocumentOrShadowRoot.customElementRegistry: the document's or shadow root's registry",
                   "[DOM][DocumentOrShadowRoot][dom:mixin-documentorshadowroot]"
                   "[dom:dom-documentorshadowroot-customelementregistry]")
  {
    SECTION("A document in a frame has its window's registry")
    {
      REQUIRE(Document->CustomElementRegistry());
      CHECK(Document->CustomElementRegistry() == &Document->Window()->CustomElements());
    }

    SECTION("A document without a browsing context has none")
    {
      auto detached = CreateRef<HTMLDocument>();
      CHECK(detached->CustomElementRegistry() == nullptr);
    }

    SECTION("A shadow root has the registry it was attached with, or its document's")
    {
      auto host = Create(u8"div");
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      CHECK(shadow->CustomElementRegistry() == Document->CustomElementRegistry());

      auto scoped = html::CustomElementRegistry::CreateScoped();
      auto scopedHost = Create(u8"div");
      auto scopedShadow =
        scopedHost->AttachShadow({.Mode = ShadowRootMode::Open, .CustomElementRegistry = scoped.get()})
          .ReleaseValue();
      CHECK(scopedShadow->CustomElementRegistry() == scoped);
    }
  }
}
