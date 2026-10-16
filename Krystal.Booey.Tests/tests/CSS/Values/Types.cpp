#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    template <typename T>
    CSSOMString SerialiseForTest(const T &value)
    {
      return Serialise(DefaultSerialisationContext(), value);
    }

    CustomIdentifier Ident(const char8 *name)
    {
      return CustomIdentifier {CSSOMStringAtom(name)};
    }
  }

  TEST_CASE("Types(serialising leaves)", "[CSS][Types]")
  {
    CHECK(SerialiseForTest(Ident(u8"foo")) == u8"foo");
    CHECK(SerialiseForTest(Ident(u8"1st")) == u8"\\31 st");
    CHECK(SerialiseForTest(PropertyIdentifier {PropertyId::BackgroundColor}) == u8"background-color");
    CHECK(SerialiseForTest(CSSOMString {u8"say \"hi\""}) == u8"\"say \\\"hi\\\"\"");
    CHECK(SerialiseForTest(CSSOMStringAtom {u8"atom"}) == u8"\"atom\"");
    CHECK(SerialiseForTest(keywords::Auto {}) == u8"auto");
    CHECK(SerialiseForTest(Symbol {SymbolRaw {ValueId::Pi}}) == u8"pi");
    CHECK(SerialiseForTest(SymbolRaw {ValueId::E}) == u8"e");

    StringBuilder builder;
    builder.Append(u8"x: ");
    Serialise(builder, DefaultSerialisationContext(), keywords::None {});
    CHECK(builder == u8"x: none");
  }

  TEST_CASE("Types(serialising aggregates)", "[CSS][Types]")
  {
    CHECK(SerialiseForTest(
            SpaceSeparatedTuple<keywords::Auto, CustomIdentifier> {keywords::Auto {}, Ident(u8"a")})
          == u8"auto a");
    CHECK(
      SerialiseForTest(CommaSeparatedTuple<CustomIdentifier, CustomIdentifier> {Ident(u8"a"), Ident(u8"b")})
      == u8"a, b");
    CHECK(SerialiseForTest(SpaceSeparatedList<CustomIdentifier> {Ident(u8"a"), Ident(u8"b"), Ident(u8"c")})
          == u8"a b c");
    CHECK(SerialiseForTest(CommaSeparatedList<CustomIdentifier> {Ident(u8"a"), Ident(u8"b")}) == u8"a, b");
    CHECK(SerialiseForTest(SpaceSeparatedList<CustomIdentifier> {}) == u8"");

    // Optional-like: empty serialises to nothing, and empty tuple elements are skipped.
    CHECK(SerialiseForTest(Maybe<CustomIdentifier> {}) == u8"");
    CHECK(SerialiseForTest(Maybe<CustomIdentifier> {Ident(u8"a")}) == u8"a");
    CHECK(SerialiseForTest(SpaceSeparatedTuple<Maybe<CustomIdentifier>, CustomIdentifier> {
            Maybe<CustomIdentifier> {}, Ident(u8"b")})
          == u8"b");

    // Variant-like.
    using Alternatives = Variant<keywords::Auto, CustomIdentifier>;
    CHECK(SerialiseForTest(Alternatives {keywords::Auto {}}) == u8"auto");
    CHECK(SerialiseForTest(Alternatives {Ident(u8"a")}) == u8"a");

    // Function notation.
    CHECK(SerialiseForTest(FunctionNotation<ValueId::Calc, CustomIdentifier> {Ident(u8"x")}) == u8"calc(x)");
    CHECK(SerialiseForTest(
            FunctionNotation<ValueId::Calc, SpaceSeparatedTuple<CustomIdentifier, CustomIdentifier>> {
              {Ident(u8"x"), Ident(u8"y")}})
          == u8"calc(x y)");
  }

  TEST_CASE("Types(serialising coalescing tuples)", "[CSS][Types]")
  {
    using Pair = MinimallySerialisingSpaceSeparatedPair<CustomIdentifier>;
    CHECK(SerialiseForTest(Pair {Ident(u8"a"), Ident(u8"a")}) == u8"a");
    CHECK(SerialiseForTest(Pair {Ident(u8"a"), Ident(u8"b")}) == u8"a b");

    using Edges = MinimallySerialisingSpaceSeparatedRectEdges<CustomIdentifier>;
    CHECK(SerialiseForTest(Edges {Ident(u8"a")}) == u8"a");
    CHECK(SerialiseForTest(Edges {Ident(u8"a"), Ident(u8"b"), Ident(u8"a"), Ident(u8"b")}) == u8"a b");
    CHECK(SerialiseForTest(Edges {Ident(u8"a"), Ident(u8"b"), Ident(u8"c"), Ident(u8"b")}) == u8"a b c");
    CHECK(SerialiseForTest(Edges {Ident(u8"a"), Ident(u8"b"), Ident(u8"c"), Ident(u8"d")}) == u8"a b c d");
  }

  TEST_CASE("Types(computed style dependencies)", "[CSS][Types]")
  {
    auto dependencies =
      CollectComputedStyleDependencies(SpaceSeparatedTuple<keywords::Auto, CustomIdentifier, Maybe<Symbol>> {
        keywords::Auto {}, Ident(u8"a"), null});
    CHECK(dependencies.IsComputationallyIndependent());

    ComputedStyleDependencies withProperty;
    withProperty.Properties.push_back(PropertyId::FontSize);
    CollectComputedStyleDependencies(withProperty, CommaSeparatedList<CustomIdentifier> {Ident(u8"a")});
    CHECK_FALSE(withProperty.IsComputationallyIndependent());

    ComputedStyleDependencies withContainer;
    withContainer.ContainerDimensions = true;
    CHECK_FALSE(withContainer.IsComputationallyIndependent());

    ComputedStyleDependencies withViewport;
    withViewport.ViewportDimensions = true;
    CHECK(withViewport.IsComputationallyIndependent());
  }

  TEST_CASE("Types(value children visitation)", "[CSS][Types]")
  {
    size_t visited = 0uz;
    Func<IterationStatus(Value &)> visitor = [&](Value &)
    {
      ++visited;
      return IterationStatus::Continue;
    };

    using Tuple = SpaceSeparatedTuple<keywords::Auto, CustomIdentifier, Symbol, Maybe<CustomIdentifier>>;
    CHECK(VisitValueChildren(visitor,
                             Tuple {keywords::Auto {}, Ident(u8"a"), Symbol {SymbolRaw {ValueId::Pi}}, null})
          == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, CommaSeparatedList<CustomIdentifier> {Ident(u8"a"), Ident(u8"b")})
          == IterationStatus::Continue);
    CHECK(VisitValueChildren(visitor, Variant<keywords::Auto, CSSOMString> {CSSOMString {u8"s"}})
          == IterationStatus::Continue);
    CHECK(visited == 0uz);
  }
}
