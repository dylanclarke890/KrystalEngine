#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using LP = LengthPercentage<NonNegative>;

    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    BorderRadius::Axis Axis(LP topLeft, LP topRight, LP bottomRight, LP bottomLeft)
    {
      return BorderRadius::Axis {topLeft, topRight, bottomRight, bottomLeft};
    }

    BorderRadius::Axis Axis(LP all)
    {
      return Axis(all, all, all, all);
    }
  }

  TEST_CASE("BorderRadius(HasDefaultValue)", "[CSS][Borders][BorderRadius]")
  {
    CHECK(HasDefaultValue(BorderRadius::DefaultValue()));
    // Only `0px` is the default, not other zeros.
    CHECK_FALSE(HasDefaultValue(
      BorderRadius {.Horizontal = Axis(LP {0_css_percentage}), .Vertical = Axis(LP {0_css_px})}));
    CHECK_FALSE(HasDefaultValue(
      BorderRadius {.Horizontal = Axis(LP {0_css_px}, LP {0_css_px}, LP {0_css_px}, LP {1_css_px}),
                    .Vertical = Axis(LP {0_css_px})}));
    CHECK_FALSE(
      HasDefaultValue(BorderRadius {.Horizontal = Axis(LP {0_css_px}), .Vertical = Axis(LP {1_css_px})}));
  }

  TEST_CASE("BorderRadius(Serialise)", "[CSS][Borders][BorderRadius]")
  {
    // The default value serialises as nothing.
    CHECK(SerialiseForTest(BorderRadius::DefaultValue()) == u8"");

    CHECK(
      SerialiseForTest(BorderRadius {.Horizontal = Axis(LP {10_css_px}), .Vertical = Axis(LP {10_css_px})})
      == u8"10px");

    // Each axis is shortened to as few values as reproduce it.
    CHECK(SerialiseForTest(
            BorderRadius {.Horizontal = Axis(LP {1_css_px}, LP {2_css_px}, LP {1_css_px}, LP {2_css_px}),
                          .Vertical = Axis(LP {1_css_px}, LP {2_css_px}, LP {1_css_px}, LP {2_css_px})})
          == u8"1px 2px");
    CHECK(SerialiseForTest(
            BorderRadius {.Horizontal = Axis(LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {2_css_px}),
                          .Vertical = Axis(LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {2_css_px})})
          == u8"1px 2px 3px");
    CHECK(SerialiseForTest(
            BorderRadius {.Horizontal = Axis(LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {4_css_px}),
                          .Vertical = Axis(LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {4_css_px})})
          == u8"1px 2px 3px 4px");

    // The vertical radii are only written when they differ.
    CHECK(SerialiseForTest(
            BorderRadius {.Horizontal = Axis(LP {10_css_px}), .Vertical = Axis(LP {5_css_percentage})})
          == u8"10px / 5%");
    CHECK(SerialiseForTest(BorderRadius {.Horizontal = Axis(LP {0_css_px}), .Vertical = Axis(LP {5_css_px})})
          == u8"0px / 5px");
    CHECK(SerialiseForTest(
            BorderRadius {.Horizontal = Axis(LP {1_css_px}, LP {2_css_px}, LP {1_css_px}, LP {2_css_px}),
                          .Vertical = Axis(LP {3_css_px})})
          == u8"1px 2px / 3px");
  }

  TEST_CASE("BorderRadius(Corners and equality)", "[CSS][Borders][BorderRadius]")
  {
    BorderRadius radius {.Horizontal = Axis(LP {1_css_px}, LP {2_css_px}, LP {3_css_px}, LP {4_css_px}),
                         .Vertical = Axis(LP {5_css_px}, LP {6_css_px}, LP {7_css_px}, LP {8_css_px})};

    CHECK(radius.TopLeft() == BorderRadius::Corner {LP {1_css_px}, LP {5_css_px}});
    CHECK(radius.TopRight() == BorderRadius::Corner {LP {2_css_px}, LP {6_css_px}});
    CHECK(radius.BottomRight() == BorderRadius::Corner {LP {3_css_px}, LP {7_css_px}});
    CHECK(radius.BottomLeft() == BorderRadius::Corner {LP {4_css_px}, LP {8_css_px}});
    CHECK(SerialiseForTest(radius.TopLeft()) == u8"1px 5px");
    CHECK(SerialiseForTest(BorderRadius::Corner {LP {1_css_px}, LP {1_css_px}}) == u8"1px");

    CHECK(radius == radius);
    CHECK(radius != BorderRadius::DefaultValue());
  }

  TEST_CASE("BorderRadius(Dependencies and visitation)", "[CSS][Borders][BorderRadius]")
  {
    CHECK(CollectComputedStyleDependencies(BorderRadius::DefaultValue()).Properties.empty());
    CHECK(CollectComputedStyleDependencies(
            BorderRadius {.Horizontal = Axis(LP {1_css_em}), .Vertical = Axis(LP {0_css_px})})
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
