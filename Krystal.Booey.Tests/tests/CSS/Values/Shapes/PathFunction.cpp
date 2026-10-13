#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesValueVisitation.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    PathFunction MakePath(CSSOMStringView d, Maybe<FillRule> fillRule = {})
    {
      return PathFunction {Path {.FillRule = krys::move(fillRule), .Data = {svg::SVGPathByteStream(d)}}};
    }
  }

  TEST_CASE("PathFunction(Serialise)", "[CSS][Shapes][PathFunction]")
  {
    CHECK(SerialiseForTest(MakePath(u8"M 0 0 L 10 10")) == u8"path(\"M 0 0 L 10 10\")");
    // The path data is written back out in a canonical form.
    CHECK(SerialiseForTest(MakePath(u8"M0,0l10-10z")) == u8"path(\"M 0 0 l 10 -10 Z\")");
    CHECK(SerialiseForTest(MakePath(u8"M 0.5 1e2")) == u8"path(\"M 0.5 100\")");
    CHECK(SerialiseForTest(MakePath(u8"")) == u8"path(\"\")");

    // `nonzero` is the default, so it's omitted.
    CHECK(SerialiseForTest(MakePath(u8"M 0 0", FillRule {keywords::Nonzero {}})) == u8"path(\"M 0 0\")");
    CHECK(SerialiseForTest(MakePath(u8"M 0 0", FillRule {keywords::Evenodd {}}))
          == u8"path(evenodd, \"M 0 0\")");
  }

  TEST_CASE("PathFunction(Equality)", "[CSS][Shapes][PathFunction]")
  {
    CHECK(MakePath(u8"M 0 0 L 10 10") == MakePath(u8"M0,0L10,10"));
    CHECK(MakePath(u8"M 0 0 L 10 10") != MakePath(u8"M 0 0 L 10 11"));
    CHECK(MakePath(u8"M 0 0") != MakePath(u8"M 0 0", FillRule {keywords::Evenodd {}}));
    // An explicit `nonzero` serialises the same as none, but is a different value.
    CHECK(MakePath(u8"M 0 0") != MakePath(u8"M 0 0", FillRule {keywords::Nonzero {}}));
    // Relative and absolute commands are different paths, even if they draw the same thing.
    CHECK(MakePath(u8"M 0 0 L 10 10") != MakePath(u8"M 0 0 l 10 10"));
  }

  TEST_CASE("PathFunction(Dependencies and visitation)", "[CSS][Shapes][PathFunction]")
  {
    CHECK(CollectComputedStyleDependencies(MakePath(u8"M 0 0", FillRule {keywords::Evenodd {}}))
            .Properties.empty());

    size_t visited = 0;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };
    CHECK(VisitValueChildren(visitor, MakePath(u8"M 0 0 L 1 1")) == IterationStatus::Continue);
    CHECK(visited == 0);
  }
}
