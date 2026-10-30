#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  namespace
  {
    using Capabilities = FontSelectionCapabilities;
    using Range = FontSelectionRange;
    using Value = FontSelectionValue;

    KRYS_NODISCARD Capabilities Face(float weight, float width = 100, float slope = 0) noexcept
    {
      return {Range {Value {weight}}, Range {Value {width}}, Range {Value {slope}}};
    }

    KRYS_NODISCARD size_t Best(const FontSelectionRequest &request, const List<Capabilities> &faces) noexcept
    {
      return FontSelectionAlgorithm {request, faces}.IndexOfBestCapabilities();
    }
  }

  TEST_CASE("FontSelectionValue", "[Graphics][FontSelectionAlgorithm]")
  {
    // Two fractional bits: quarters are exact, finer values truncate.
    CHECK(static_cast<float>(Value {62.5f}) == 62.5f);
    CHECK(static_cast<float>(Value {1.3f}) == 1.25f);
    CHECK(Value {7}.RawValue() == 28);

    CHECK(Value {3} + Value {2} == Value {5});
    CHECK(Value {3} - Value {5} == Value {-2});
    CHECK(Value {1.5f} * Value {2} == Value {3});
    CHECK(Value {3} / Value {2} == Value {1.5f});
    CHECK(-Value {4} == Value {-4});
    CHECK(Value {1} < Value {1.25f});

    CHECK(static_cast<float>(Value::MaximumValue()) == 8191.75f);
    CHECK(static_cast<float>(Value::MinimumValue()) == -8192.f);
    CHECK(Value::ClampFloat(100000.f) == Value::MaximumValue());
    CHECK(Value::ClampFloat(-100000.f) == Value::MinimumValue());
    CHECK(Value::ClampFloat(450.f) == Value {450});

    CHECK(IsItalic(ItalicValue()));
    CHECK_FALSE(IsItalic(Value {13}));
    CHECK_FALSE(IsItalic(std::nullopt));
    CHECK(IsFontWeightBold(Value {600}));
    CHECK_FALSE(IsFontWeightBold(Value {599}));
    CHECK(SemiExpandedWidthValue() == Value {112.5f});
  }

  TEST_CASE("FontSelectionRange", "[Graphics][FontSelectionAlgorithm]")
  {
    Range range {Value {700}, Value {300}};
    CHECK(range.Minimum == Value {300});
    CHECK(range.Maximum == Value {700});
    CHECK(range.Includes(Value {500}));
    CHECK_FALSE(range.Includes(Value {800}));

    range.Expand(Range {Value {900}});
    CHECK(range == Range {Value {300}, Value {900}});

    FontSelectionSpecifiedCapabilities specified;
    CHECK(specified.ComputeFontSelectionCapabilities() == Capabilities {});
    specified.Weight = Range {Value {100}, Value {900}};
    CHECK(specified.ComputeWeight() == Range {Value {100}, Value {900}});
    CHECK(specified.ComputeWidth() == Range {NormalWidthValue()});
  }

  TEST_CASE("FontSelectionAlgorithm", "[Graphics][FontSelectionAlgorithm]")
  {
    FontSelectionRequest normal {NormalWeightValue(), NormalWidthValue(), std::nullopt};

    SECTION("an exact match wins")
    {
      CHECK(Best(normal, {Face(700), Face(400), Face(300)}) == 1);
    }

    SECTION("weights between 400 and 500 look up to 500 first, then down, then up")
    {
      CHECK(Best(normal, {Face(300), Face(500), Face(600)}) == 1);
      CHECK(Best(normal, {Face(300), Face(600)}) == 0);
      CHECK(Best({Value {450}, NormalWidthValue(), std::nullopt}, {Face(600), Face(470)}) == 1);
    }

    SECTION("light weights look down, then up")
    {
      FontSelectionRequest light {Value {300}, NormalWidthValue(), std::nullopt};
      CHECK(Best(light, {Face(500), Face(200), Face(100)}) == 1);
      CHECK(Best(light, {Face(700), Face(500)}) == 1);
    }

    SECTION("bold weights look up, then down")
    {
      FontSelectionRequest bold {BoldWeightValue(), NormalWidthValue(), std::nullopt};
      CHECK(Best(bold, {Face(500), Face(900), Face(800)}) == 2);
      CHECK(Best(bold, {Face(400), Face(600)}) == 1);
    }

    SECTION("width is matched before weight")
    {
      FontSelectionRequest condensed {NormalWeightValue(), CondensedWidthValue(), std::nullopt};
      CHECK(Best(condensed, {Face(400, 100), Face(700, 75)}) == 1);
      // Narrower widths are preferred for condensed requests, wider ones for expanded requests.
      CHECK(Best(condensed, {Face(400, 125), Face(400, 50)}) == 1);
      FontSelectionRequest expanded {NormalWeightValue(), ExpandedWidthValue(), std::nullopt};
      CHECK(Best(expanded, {Face(400, 50), Face(400, 150)}) == 1);
    }

    SECTION("italic requests prefer italic faces, then oblique, then normal")
    {
      FontSelectionRequest italic {NormalWeightValue(), NormalWidthValue(), ItalicValue()};
      CHECK(Best(italic, {Face(400, 100, 0), Face(400, 100, 20)}) == 1);
      CHECK(Best(italic, {Face(400, 100, 0), Face(400, 100, -14)}) == 0);
      CHECK(Best(normal, {Face(400, 100, 14), Face(400, 100, 0)}) == 1);
    }

    SECTION("capabilities bounds")
    {
      List<Capabilities> faces {Face(300), Face(800)};
      FontSelectionAlgorithm algorithm {normal, faces};
      CHECK(algorithm.WeightDistance(Face(400)).Distance == Value {});
      CHECK(algorithm.WeightDistance(Face(300)).Value == Value {300});
    }
  }
}
