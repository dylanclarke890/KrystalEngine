#include "Krystal.Booey/HTML/HTMLProgressElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/FormTestUtils.hpp"

namespace krys::boo::html::tests
{
  using boo::tests::FormFixture;

  TEST_CASE("HTMLProgressElement: value, max and position",
            "[HTML][HTMLProgressElement][html:concept-progress-maximum][html:concept-progress-value]"
            "[html:concept-progress-current-value][html:dom-progress-position][html:dom-progress-value]"
            "[html:attr-progress-value][html:attr-progress-max]")
  {
    FormFixture fixture(u8"<progress id=i></progress><progress id=p value=3 max=4></progress>"
                        u8"<progress id=over value=9 max=x></progress><progress id=neg value=-2></progress>");
    auto &indeterminate = fixture.Get<HTMLProgressElement>(u8"i");
    CHECK(indeterminate.Value() == 0);
    CHECK(indeterminate.Max() == 1);
    CHECK(indeterminate.Position() == -1);

    auto &progress = fixture.Get<HTMLProgressElement>(u8"p");
    CHECK(progress.Position() == 0.75);

    // A max that doesn't parse is 1, and a value above it is the max.
    auto &over = fixture.Get<HTMLProgressElement>(u8"over");
    CHECK(over.Max() == 1);
    CHECK(over.Value() == 1);
    CHECK(fixture.Get<HTMLProgressElement>(u8"neg").Value() == 0);

    // max is limited to positive numbers.
    progress.Max(0);
    CHECK(progress.Max() == 4);
    progress.Max(8);
    CHECK(progress.Max() == 8);
  }
}
