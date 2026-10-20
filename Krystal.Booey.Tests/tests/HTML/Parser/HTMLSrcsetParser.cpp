#include "Krystal.Booey/HTML/Parser/HTMLSrcsetParser.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::html::tests
{
  TEST_CASE("HTMLSrcsetParser: parses candidates and their descriptors", "[HTML][Parser][HTMLSrcsetParser]")
  {
    SECTION("Densities and widths")
    {
      auto candidates = ParseImageCandidatesFromSrcsetAttribute(u8"a.png, b.png 2x,c.png 1.5x , d.png 300w");
      REQUIRE(candidates.size() == 4u);
      CHECK(candidates[0].String == u8"a.png");
      CHECK(candidates[0].Density == UninitializedDescriptor);
      CHECK(candidates[1].String == u8"b.png");
      CHECK(candidates[1].Density == 2.f);
      CHECK(candidates[2].String == u8"c.png");
      CHECK(candidates[2].Density == 1.5f);
      CHECK(candidates[3].String == u8"d.png");
      CHECK(candidates[3].ResourceWidth == 300);
    }

    SECTION("Commas in URLs and trailing commas")
    {
      auto candidates = ParseImageCandidatesFromSrcsetAttribute(u8"data:a,b 1x, c.png,,, ,d.png");
      REQUIRE(candidates.size() == 3u);
      CHECK(candidates[0].String == u8"data:a,b");
      CHECK(candidates[1].String == u8"c.png");
      CHECK(candidates[2].String == u8"d.png");
    }

    SECTION("Invalid descriptors drop their candidate")
    {
      auto candidates = ParseImageCandidatesFromSrcsetAttribute(
        u8"a.png 1x 2x, b.png 0w, c.png 10h, d.png 10w 5h, e.png -1x, f.png 2q, g.png 2x");
      REQUIRE(candidates.size() == 2u);
      CHECK(candidates[0].String == u8"d.png");
      CHECK(candidates[1].String == u8"g.png");
    }

    SECTION("Parentheses in descriptors")
    {
      auto candidates = ParseImageCandidatesFromSrcsetAttribute(u8"a.png (x, y) 2x, b.png");
      REQUIRE(candidates.size() == 1u);
      CHECK(candidates[0].String == u8"b.png");
    }
  }

  TEST_CASE("HTMLSrcsetParser: picks the candidate that fits the device scale factor",
            "[HTML][Parser][HTMLSrcsetParser]")
  {
    SECTION("No srcset uses src")
    {
      auto candidate = BestFitSourceForImageAttributes(1, dom::DOMStringView(u8"a.png"), null, 0);
      CHECK(candidate.String == u8"a.png");
      CHECK(candidate.SrcOrigin());
      CHECK(BestFitSourceForImageAttributes(1, null, null, 0).IsEmpty());
    }

    SECTION("Densities, with src as 1x")
    {
      auto pick = [](float scale)
      {
        return BestFitSourceForImageAttributes(scale, dom::DOMStringView(u8"one.png"),
                                               dom::DOMStringView(u8"two.png 2x, three.png 3x"), 0);
      };
      CHECK(pick(1).String == u8"one.png");
      CHECK(pick(1.5f).String == u8"two.png");
      CHECK(pick(2).String == u8"two.png");
      CHECK(pick(4).String == u8"three.png");
      CHECK(pick(2).Density == 2.f);
    }

    SECTION("Widths resolve against the source size, and src is ignored")
    {
      auto pick = [](float sourceSize)
      {
        return BestFitSourceForImageAttributes(1, dom::DOMStringView(u8"src.png"),
                                               dom::DOMStringView(u8"small.png 100w, large.png 400w"),
                                               sourceSize);
      };
      CHECK(pick(100).String == u8"small.png");
      CHECK(pick(200).String == u8"large.png");
      CHECK(pick(200).Density == 2.f);
      CHECK(pick(1'000).String == u8"large.png");
    }

    SECTION("Ignored candidates")
    {
      auto candidate = BestFitSourceForImageAttributes(1, null, dom::DOMStringView(u8"a.png 1x, b.png 2x"), 0,
                                                       [](const ImageCandidate &candidate)
                                                       { return candidate.String == u8"a.png"; });
      CHECK(candidate.String == u8"b.png");
    }
  }
}
