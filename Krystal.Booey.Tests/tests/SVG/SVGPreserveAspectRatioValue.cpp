#include "Krystal.Booey/SVG/SVGPreserveAspectRatioValue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"

namespace krys::boo::svg::tests
{
  using Value = SVGPreserveAspectRatioValue;

  TEST_CASE("SVGPreserveAspectRatioValue: parsing and serialising", "[SVG][SVGPreserveAspectRatioValue]")
  {
    CHECK(Value().ValueAsString() == u8"xMidYMid meet");
    CHECK(Value(u8"none").ValueAsString() == u8"none meet");
    CHECK(Value(u8"xMinYMax slice").ValueAsString() == u8"xMinYMax slice");
    CHECK(Value(u8"  xMaxYMid  meet ").ValueAsString() == u8"xMaxYMid meet");
    // `slice` is ignored with `none`.
    CHECK(Value(u8"none slice").MeetOrSlice() == Value::SVG_MEETORSLICE_MEET);

    Value value;
    CHECK(value.Parse(u8"xMinYMin"));
    CHECK(value.Align() == Value::SVG_PRESERVEASPECTRATIO_XMINYMIN);
    CHECK(value.Parse(u8"xMaxYMax slice"));
    CHECK(value.Align() == Value::SVG_PRESERVEASPECTRATIO_XMAXYMAX);
    CHECK(value.MeetOrSlice() == Value::SVG_MEETORSLICE_SLICE);

    // A failed parse leaves the default.
    CHECK_FALSE(value.Parse(u8"xMidYMid foo"));
    CHECK(value == Value());
    CHECK_FALSE(value.Parse(u8"XMidYMid"));
    CHECK_FALSE(value.Parse(u8"xMidYMi"));
    CHECK_FALSE(value.Parse(u8"xMidYMid meat"));
    CHECK_FALSE(value.Parse(u8""));

    // Without validation, what follows is left in the buffer.
    dom::DOMStringView buffer = u8"xMinYMid slice, 1 2";
    CHECK(value.Parse(buffer, false));
    CHECK(value.ValueAsString() == u8"xMinYMid slice");
    CHECK(buffer == u8", 1 2");
  }

  TEST_CASE("SVGPreserveAspectRatioValue: setters", "[SVG][SVGPreserveAspectRatioValue]")
  {
    Value value;
    REQUIRE_FALSE(value.SetAlign(Value::SVG_PRESERVEASPECTRATIO_NONE).HasException());
    REQUIRE_FALSE(value.SetMeetOrSlice(Value::SVG_MEETORSLICE_SLICE).HasException());
    CHECK(value.ValueAsString() == u8"none slice");
    CHECK(value.SetAlign(0) == dom::ExceptionCode::NotSupportedError);
    CHECK(value.SetAlign(11) == dom::ExceptionCode::NotSupportedError);
    CHECK(value.SetMeetOrSlice(3) == dom::ExceptionCode::NotSupportedError);
    CHECK(value.ValueAsString() == u8"none slice");
  }

  TEST_CASE("SVGPreserveAspectRatioValue: transforms", "[SVG][SVGPreserveAspectRatioValue]")
  {
    // A 100x50 view box in 200x200: meet scales by 2 and centres vertically.
    auto meet = Value().GetCTM(0, 0, 100, 50, 200, 200);
    CHECK(meet == AffineTransform(2, 0, 0, 2, 0, 50));
    auto meetAtTop = Value(u8"xMidYMin").GetCTM(0, 0, 100, 50, 200, 200);
    CHECK(meetAtTop == AffineTransform(2, 0, 0, 2, 0, 0));
    // Slice scales by 4 and centres horizontally.
    auto slice = Value(u8"xMidYMid slice").GetCTM(0, 0, 100, 50, 200, 200);
    CHECK(slice == AffineTransform(4, 0, 0, 4, -100, 0));
    auto sliceAtEnd = Value(u8"xMaxYMid slice").GetCTM(0, 0, 100, 50, 200, 200);
    CHECK(sliceAtEnd == AffineTransform(4, 0, 0, 4, -200, 0));
    // None stretches, from the view box's origin.
    auto none = Value(u8"none").GetCTM(10, 0, 100, 50, 200, 200);
    CHECK(none == AffineTransform(2, 0, 0, 4, -20, 0));

    // Meet shrinks the destination around a square image; slice crops the source.
    FloatRect dest(0, 0, 200, 100);
    FloatRect source(0, 0, 50, 50);
    Value().TransformRect(dest, source);
    CHECK(dest == FloatRect(50, 0, 100, 100));
    CHECK(source == FloatRect(0, 0, 50, 50));

    dest = FloatRect(0, 0, 200, 100);
    Value(u8"xMidYMax slice").TransformRect(dest, source);
    CHECK(dest == FloatRect(0, 0, 200, 100));
    CHECK(source == FloatRect(0, 25, 50, 25));
  }
}
