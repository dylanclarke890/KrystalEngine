#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLFraction.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLOperator.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLRoot.hpp"
#include "Krystal.Booey/Rendering/MathML/RenderMathMLUnderOver.hpp"
#include "Krystal.Gfx.Tests/Golden.hpp"
#include "Krystal.UI.Tests/ViewTestUtils.hpp"
#include "Krystal.UI/View.hpp"

namespace krys::ui::tests
{
  TEST_CASE("View: the editor page's MathML", "[UI]")
  {
    View view({1'200, 600});
    REQUIRE(
      view.LoadFile(gfx::tests::GoldenDirectory() / ".." / ".." / "Krystal.Editor" / "pages" / "index.html")
        .has_value());
    WaitForLoad(view);
    for (int32 pass = 0; pass < 4; ++pass)
    {
      view.UpdateRendering();
      boo::html::EventLoop::Current().RunUntilIdle();
    }
    auto &document = view.Document();
    auto byId = [&](utf8_stringview id)
    {
      auto element = document.GetElementById(id);
      REQUIRE(element);
      return element;
    };
    auto rendererOf = [&](utf8_stringview id) -> boo::RenderElement &
    {
      auto *renderer = byId(id)->Renderer();
      REQUIRE(renderer);
      return *renderer;
    };
    auto boxOf = [&](utf8_stringview id)
    {
      return byId(id)->GetBoundingClientRect();
    };

    // The section in view.
    byId(u8"mathml")->ScrollIntoView(true);
    view.UpdateRendering();

    SECTION("formulas")
    {
      // A display formula is centred in its card.
      auto formula = boxOf(u8"math-quadratic");
      auto fraction = boxOf(u8"math-fraction");
      CHECK(fraction->Width() < formula->Width());
      CHECK(Is<boo::RenderMathMLFraction>(rendererOf(u8"math-fraction")));

      // Inline, the fraction is in compact style, so its parts are set smaller.
      auto inlineFraction = boxOf(u8"math-inline-fraction");
      CHECK(inlineFraction->Width() < fraction->Width());
      CHECK(inlineFraction->Height() < fraction->Height());
    }

    SECTION("sums and integrals")
    {
      // In display style, the sum is large, with its limits over it.
      auto sigma = boxOf(u8"math-sigma");
      auto upper = boxOf(u8"math-upper");
      CHECK(Is<boo::RenderMathMLUnderOver>(*rendererOf(u8"math-sigma").Parent()));
      CHECK(upper->Bottom() <= sigma->Top());
      CHECK(upper->Left() > sigma->Left());
      CHECK(upper->Right() < sigma->Right());

      // Inline, it's smaller, and its limits move to its side.
      auto inlineSigma = boxOf(u8"math-inline-sigma");
      auto inlineUpper = boxOf(u8"math-inline-upper");
      CHECK(inlineSigma->Height() < sigma->Height());
      CHECK(inlineUpper->Left() >= inlineSigma->Right());
      CHECK(inlineUpper->Top() < inlineSigma->Top());

      // The integral is larger in display style too.
      CHECK(boxOf(u8"math-inline-integral")->Height() < boxOf(u8"math-integral")->Height());
      auto &integral = Downcast<boo::RenderMathMLOperator>(rendererOf(u8"math-integral"));
      CHECK(integral.IsLargeOperatorInDisplayStyle());
    }

    SECTION("matrices, roots and scripts")
    {
      // The bracket stretches to the matrix's height.
      auto bracket = boxOf(u8"math-bracket");
      auto matrix = boxOf(u8"math-matrix");
      CHECK(bracket->Height() >= matrix->Height());
      CHECK(bracket->Right() <= matrix->Left());

      // Each radical covers the one inside it.
      auto root = boxOf(u8"math-root");
      auto nested = boxOf(u8"math-nested");
      auto innermost = boxOf(u8"math-innermost");
      CHECK(Is<boo::RenderMathMLRoot>(rendererOf(u8"math-root")));
      CHECK(root->Height() > nested->Height());
      CHECK(nested->Height() > innermost->Height());
      CHECK(root->Right() >= nested->Right());
      CHECK(nested->Right() >= innermost->Right());

      // The prescript is before the identifier, the subscript after it, both below its baseline.
      auto identifier = boxOf(u8"math-c");
      auto subscript = boxOf(u8"math-k");
      auto prescript = boxOf(u8"math-n");
      CHECK(prescript->Right() <= identifier->Left());
      CHECK(subscript->Left() >= identifier->Right());
      CHECK(prescript->Bottom() > identifier->Bottom());
      CHECK(subscript->Bottom() > identifier->Bottom());
    }
  }
}
