#include "Krystal.Gfx/Render2D/CoverageBlur.hpp"
#include "catch2/catch_test_macros.hpp"
#include <cmath>

namespace krys::gfx::tests
{
  namespace
  {
    /// @brief A square of full coverage, `size` pixels across, at the origin.
    KRYS_NODISCARD CoverageMask Square(uint32 size)
    {
      return {.Bounds = {.X = 0, .Y = 0, .Width = size, .Height = size},
              .Coverage = List<uint8>(static_cast<size_t>(size) * size, 255u)};
    }

    KRYS_NODISCARD double Total(const CoverageMask &mask)
    {
      double total = 0.0;
      for (const auto value : mask.Coverage)
      {
        total += value / 255.0;
      }
      return total;
    }

    /// @brief How much of a Gaussian with standard deviation `sigma` centred on `p` falls between `low` and
    /// `high`.
    KRYS_NODISCARD double GaussianMass(double low, double high, double p, double sigma)
    {
      const double k = 1.0 / (sigma * std::sqrt(2.0));
      return 0.5 * (std::erf((high - p) * k) - std::erf((low - p) * k));
    }

    /// @brief The largest gap, in coverage steps, between a blurred square and the exact blur of a square.
    KRYS_NODISCARD double LargestError(const CoverageMask &blurred, double size, double sigma)
    {
      double error = 0.0;
      for (int32 y = blurred.Bounds.Y; y < blurred.Bounds.Y + static_cast<int32>(blurred.Bounds.Height); ++y)
      {
        for (int32 x = blurred.Bounds.X; x < blurred.Bounds.X + static_cast<int32>(blurred.Bounds.Width); ++x)
        {
          const double expected =
            GaussianMass(0.0, size, x + 0.5, sigma) * GaussianMass(0.0, size, y + 0.5, sigma) * 255.0;
          error = std::max(error, std::abs(blurred.At(x, y) - expected));
        }
      }
      return error;
    }
  }

  TEST_CASE("CoverageBlur: grows the mask by the blur's reach", "[Gfx][Render2D]")
  {
    CHECK(detail::BlurExtent(0.f) == 0u);
    CHECK(detail::BlurExtent(-1.f) == 0u);
    CHECK(detail::BlurExtent(1.f) == 3u);
    CHECK(detail::BlurExtent(4.f) == 12u);
    // From 8, three boxes of SVG's size: 19 here, so 3 times 9.
    CHECK(detail::BlurExtent(10.f) == 3u * 9u);

    const auto square = Square(8u);
    CHECK(detail::BlurCoverage(square, 0.f).Bounds == square.Bounds);
    const auto blurred = detail::BlurCoverage(square, 4.f);
    CHECK(blurred.Bounds == PixelRect2D {.X = -12, .Y = -12, .Width = 32u, .Height = 32u});
    CHECK(blurred.Coverage.size() == 32u * 32u);
  }

  TEST_CASE("CoverageBlur: keeps the coverage's total", "[Gfx][Render2D]")
  {
    for (const float sigma : {0.5f, 1.5f, 3.f, 6.5f})
    {
      INFO("sigma " << sigma);
      const auto square = Square(10u);
      // Each pixel rounds by at most half a step.
      const auto blurred = detail::BlurCoverage(square, sigma);
      CHECK(std::abs(Total(blurred) - Total(square)) < blurred.Coverage.size() * 0.5 / 255.0);
    }
  }

  TEST_CASE("CoverageBlur: is close to a Gaussian", "[Gfx][Render2D]")
  {
    // Below 8 the kernel is exact for whole pixels, short of rounding and the tails past three standard
    // deviations; from 8, three boxes stand in for it, within a few percent.
    for (const float sigma : {0.5f, 1.f, 2.f, 5.f, 7.5f})
    {
      INFO("sigma " << sigma);
      const uint32 size = static_cast<uint32>(4.f * sigma) + 8u;
      CHECK(LargestError(detail::BlurCoverage(Square(size), sigma), size, sigma) < 1.5);
    }
    CHECK(LargestError(detail::BlurCoverage(Square(48u), 10.f), 48.0, 10.0) < 8.0);
  }
}
