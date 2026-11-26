#include "Krystal.Web/Rendering/RenderView.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Style/ResolverTestUtils.hpp"

namespace krys::web::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "RenderView: a new view's preferred widths need computing",
                   "[Rendering][RenderView]")
  {
    // With nothing rendered under it, nothing else marks the view.
    ResolverFixture fixture {u8"<html style=display:none>"};
    fixture.Document->UpdateStyleIfNeeded();
    auto *view = fixture.Document->GetRenderView();
    REQUIRE(view);
    REQUIRE_FALSE(view->FirstChild());
    CHECK(view->NeedsPreferredLogicalWidthsUpdate());
  }
}
