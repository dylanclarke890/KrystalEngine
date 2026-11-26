#include "Krystal.Web/Css/Values/Borders/BorderRadius.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Web/Css/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    using NonNegativeLP = LengthPercentage<NonNegative>;

    BorderRadius::Axis Axis(NonNegativeLP topLeft, NonNegativeLP topRight, NonNegativeLP bottomRight,
                            NonNegativeLP bottomLeft)
    {
      return BorderRadius::Axis {krys::move(topLeft), krys::move(topRight), krys::move(bottomRight),
                                 krys::move(bottomLeft)};
    }

    BorderRadius::Axis Axis(const NonNegativeLP &all)
    {
      return Axis(all, all, all, all);
    }
  }

  TEST_CASE("BorderRadius(HasDefaultValue)", "[CSS][Borders][BorderRadius]")
  {
    CHECK(HasDefaultValue(BorderRadius::DefaultValue()));
    // Only `0px` is the default, not other zeros.
    CHECK_FALSE(HasDefaultValue(BorderRadius {.Horizontal = Axis(NonNegativeLP {0_css_percentage}),
                                              .Vertical = Axis(NonNegativeLP {0_css_px})}));
    CHECK_FALSE(
      HasDefaultValue(BorderRadius {.Horizontal = Axis(NonNegativeLP {0_css_px}, NonNegativeLP {0_css_px},
                                                       NonNegativeLP {0_css_px}, NonNegativeLP {1_css_px}),
                                    .Vertical = Axis(NonNegativeLP {0_css_px})}));
    CHECK_FALSE(HasDefaultValue(BorderRadius {.Horizontal = Axis(NonNegativeLP {0_css_px}),
                                              .Vertical = Axis(NonNegativeLP {1_css_px})}));
  }

  TEST_CASE("BorderRadius(Serialise)", "[CSS][Borders][BorderRadius]")
  {
    // The default value serialises as nothing.
    CHECK(SerialiseForTest(BorderRadius::DefaultValue()).empty());

    CHECK(SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {10_css_px}),
                                         .Vertical = Axis(NonNegativeLP {10_css_px})})
          == u8"10px");

    // Each axis is shortened to as few values as reproduce it.
    CHECK(
      SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                        NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px}),
                                     .Vertical = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                      NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px})})
      == u8"1px 2px");
    CHECK(
      SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                        NonNegativeLP {3_css_px}, NonNegativeLP {2_css_px}),
                                     .Vertical = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                      NonNegativeLP {3_css_px}, NonNegativeLP {2_css_px})})
      == u8"1px 2px 3px");
    CHECK(
      SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                        NonNegativeLP {3_css_px}, NonNegativeLP {4_css_px}),
                                     .Vertical = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                      NonNegativeLP {3_css_px}, NonNegativeLP {4_css_px})})
      == u8"1px 2px 3px 4px");

    // The vertical radii are only written when they differ.
    CHECK(SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {10_css_px}),
                                         .Vertical = Axis(NonNegativeLP {5_css_percentage})})
          == u8"10px / 5%");
    CHECK(SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {0_css_px}),
                                         .Vertical = Axis(NonNegativeLP {5_css_px})})
          == u8"0px / 5px");
    CHECK(
      SerialiseForTest(BorderRadius {.Horizontal = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                                        NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px}),
                                     .Vertical = Axis(NonNegativeLP {3_css_px})})
      == u8"1px 2px / 3px");
  }

  TEST_CASE("BorderRadius(Corners and equality)", "[CSS][Borders][BorderRadius]")
  {
    BorderRadius radius {.Horizontal = Axis(NonNegativeLP {1_css_px}, NonNegativeLP {2_css_px},
                                            NonNegativeLP {3_css_px}, NonNegativeLP {4_css_px}),
                         .Vertical = Axis(NonNegativeLP {5_css_px}, NonNegativeLP {6_css_px},
                                          NonNegativeLP {7_css_px}, NonNegativeLP {8_css_px})};

    CHECK(radius.TopLeft() == BorderRadius::Corner {NonNegativeLP {1_css_px}, NonNegativeLP {5_css_px}});
    CHECK(radius.TopRight() == BorderRadius::Corner {NonNegativeLP {2_css_px}, NonNegativeLP {6_css_px}});
    CHECK(radius.BottomRight() == BorderRadius::Corner {NonNegativeLP {3_css_px}, NonNegativeLP {7_css_px}});
    CHECK(radius.BottomLeft() == BorderRadius::Corner {NonNegativeLP {4_css_px}, NonNegativeLP {8_css_px}});
    CHECK(SerialiseForTest(radius.TopLeft()) == u8"1px 5px");
    CHECK(SerialiseForTest(BorderRadius::Corner {NonNegativeLP {1_css_px}, NonNegativeLP {1_css_px}})
          == u8"1px");

    CHECK(radius == radius);
    CHECK(radius != BorderRadius::DefaultValue());
  }

  TEST_CASE("BorderRadius(Dependencies and visitation)", "[CSS][Borders][BorderRadius]")
  {
    CHECK(CollectComputedStyleDependencies(BorderRadius::DefaultValue()).Properties.empty());
    CHECK(CollectComputedStyleDependencies(BorderRadius {.Horizontal = Axis(NonNegativeLP {1_css_em}),
                                                         .Vertical = Axis(NonNegativeLP {0_css_px})})
            .Properties
          == List<PropertyId> {PropertyId::FontSize});

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, BorderRadius::DefaultValue()) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
