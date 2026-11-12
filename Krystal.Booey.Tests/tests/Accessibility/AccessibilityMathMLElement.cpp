#include "Krystal.Booey/Accessibility/AccessibilityMathMLElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Accessibility/AccessibilityTestUtils.hpp"

namespace krys::boo::ax::tests
{
  TEST_CASE("AccessibilityMathMLElement: roles and parts", "[Accessibility][AccessibilityMathMLElement]")
  {
    AccessibilityFixture page(
      u8"<math id=math alttext='x squared over two'>"
      u8"<mfrac id=fraction><msup id=power><mi id=x>x</mi><mn id=two>2</mn></msup>"
      u8"<mn id=denominator>2</mn></mfrac>"
      u8"<mroot id=root><mi id=radicand>y</mi><mn id=index>3</mn></mroot>"
      u8"<msqrt id=sqrt><mi>a</mi><mo>+</mo><mi>b</mi></msqrt>"
      u8"<munderover id=sum><mo id=sigma>&#x2211;</mo><mi id=under>i</mi><mi id=over>n</mi>"
      u8"</munderover>"
      u8"<mspace id=space width=1em></mspace>"
      u8"</math>");

    auto &math = page.Object(u8"math");
    REQUIRE(Is<AccessibilityMathMLElement>(math));
    CHECK(math.Role() == AccessibilityRole::DocumentMath);
    CHECK(math.AccessibleName() == u8"x squared over two");

    auto &fraction = page.Object(u8"fraction");
    CHECK(fraction.Role() == AccessibilityRole::MathElement);
    CHECK(fraction.IsMathFraction());
    CHECK(fraction.MathNumeratorObject() == &page.Object(u8"power"));
    CHECK(fraction.MathDenominatorObject() == &page.Object(u8"denominator"));

    auto &power = page.Object(u8"power");
    CHECK(power.IsMathSubscriptSuperscript());
    CHECK(power.MathBaseObject() == &page.Object(u8"x"));
    CHECK(power.MathSuperscriptObject() == &page.Object(u8"two"));
    CHECK(power.MathSubscriptObject() == nullptr);
    CHECK(page.Object(u8"x").IsMathIdentifier());
    CHECK(page.Object(u8"two").IsMathNumber());

    auto &root = page.Object(u8"root");
    CHECK(root.IsMathRoot());
    CHECK_FALSE(root.IsMathSquareRoot());
    CHECK(root.MathRootIndexObject() == &page.Object(u8"index"));
    REQUIRE(root.MathRadicand());
    CHECK(root.MathRadicand()->size() == 1);

    auto &sqrt = page.Object(u8"sqrt");
    CHECK(sqrt.IsMathSquareRoot());
    CHECK(sqrt.MathRootIndexObject() == nullptr);
    REQUIRE(sqrt.MathRadicand());
    CHECK(sqrt.MathRadicand()->size() == 3);

    auto &sum = page.Object(u8"sum");
    CHECK(sum.IsMathUnderOver());
    CHECK(sum.MathBaseObject() == &page.Object(u8"sigma"));
    CHECK(sum.MathUnderObject() == &page.Object(u8"under"));
    CHECK(sum.MathOverObject() == &page.Object(u8"over"));
    CHECK(page.Object(u8"sigma").IsMathOperator());

    // Only the elements recognised are exposed: not mspace.
    CHECK(page.Object(u8"space").IsIgnored());
    CHECK_FALSE(page.Object(u8"x").IsIgnored());
  }

  TEST_CASE("AccessibilityMathMLElement: multiscripts", "[Accessibility][AccessibilityMathMLElement]")
  {
    AccessibilityFixture page(u8"<math><mmultiscripts id=scripts><mi id=base>X</mi>"
                              u8"<mi id=postSub>a</mi><mi id=postSup>b</mi>"
                              u8"<mprescripts/><mi id=preSub>c</mi><mi id=preSup>d</mi>"
                              u8"</mmultiscripts></math>");
    auto &scripts = page.Object(u8"scripts");
    CHECK(scripts.IsMathMultiscript());
    CHECK(scripts.MathBaseObject() == &page.Object(u8"base"));

    auto postscripts = scripts.MathPostscripts();
    REQUIRE(postscripts.size() == 1);
    CHECK(postscripts[0].first == &page.Object(u8"postSub"));
    CHECK(postscripts[0].second == &page.Object(u8"postSup"));
    auto prescripts = scripts.MathPrescripts();
    REQUIRE(prescripts.size() == 1);
    CHECK(prescripts[0].first == &page.Object(u8"preSub"));
    CHECK(prescripts[0].second == &page.Object(u8"preSup"));
  }
}
