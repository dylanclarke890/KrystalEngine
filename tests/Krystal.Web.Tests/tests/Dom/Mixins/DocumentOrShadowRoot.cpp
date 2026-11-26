#include "Krystal.Web/Dom/Mixins/DocumentOrShadowRoot.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Html/CustomElementTestUtils.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"
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

  template <typename T>
  concept DocumentOrShadowRootMixin = requires(T &node) {
    { node.CustomElementRegistry() } -> SameType<html::CustomElementRegistry *>;
  };

  TEST_CASE("Mixin::DocumentOrShadowRoot", "[HTML][Mixins][dom:mixin-documentorshadowroot]")
  {
    STATIC_REQUIRE(DocumentOrShadowRootMixin<Document>);
    STATIC_REQUIRE(DocumentOrShadowRootMixin<ShadowRoot>);
  }
}
