#include "Krystal.Gfx/Render2D/CoverageBlur.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>
#include <cmath>
#include <numbers>

namespace krys::gfx::detail
{
  namespace
  {
    /// @brief The box size SVG's `feGaussianBlur` gives a standard deviation.
    KRYS_NODISCARD uint32 BoxSize(float sigma) noexcept
    {
      return static_cast<uint32>(
        std::floor(sigma * 3.f * std::sqrt(2.f * std::numbers::pi_v<float>) / 4.f + 0.5f));
    }

    /// @brief Averages each value with `left` values before it and `right` after it, those past the ends
    /// being 0.
    void Box(const List<float> &in, List<float> &out, uint32 left, uint32 right) noexcept
    {
      const auto count = static_cast<int64>(in.size());
      const float scale = 1.f / static_cast<float>(left + right + 1u);
      float sum = 0.f;
      for (int64 i = 0; i < static_cast<int64>(right) && i < count; ++i)
      {
        sum += in[static_cast<size_t>(i)];
      }
      for (int64 i = 0; i < count; ++i)
      {
        const int64 entering = i + static_cast<int64>(right);
        const int64 leaving = i - static_cast<int64>(left) - 1;
        if (entering < count)
        {
          sum += in[static_cast<size_t>(entering)];
        }
        if (leaving >= 0)
        {
          sum -= in[static_cast<size_t>(leaving)];
        }
        out[static_cast<size_t>(i)] = sum * scale;
      }
    }

    /// @brief Blurs one line of values in place, which has room for the blur's spread at each end.
    void BlurLine(List<float> &line, List<float> &scratch, float sigma, const List<float> &kernel) noexcept
    {
      scratch.resize(line.size());
      if (sigma < BoxBlurSigma)
      {
        const auto radius = static_cast<int64>(kernel.size() / 2u);
        const auto count = static_cast<int64>(line.size());
        for (int64 i = 0; i < count; ++i)
        {
          float sum = 0.f;
          for (int64 k = -radius; k <= radius; ++k)
          {
            const int64 j = i + k;
            if (j >= 0 && j < count)
            {
              sum += line[static_cast<size_t>(j)] * kernel[static_cast<size_t>(k + radius)];
            }
          }
          scratch[static_cast<size_t>(i)] = sum;
        }
        line.swap(scratch);
        return;
      }

      // An odd size is three centred boxes; an even one is a box to the left, one to the right and one a
      // pixel wider, centred.
      const uint32 size = BoxSize(sigma);
      if (size % 2u == 1u)
      {
        const uint32 half = size / 2u;
        Box(line, scratch, half, half);
        Box(scratch, line, half, half);
        Box(line, scratch, half, half);
      }
      else
      {
        const uint32 half = size / 2u;
        Box(line, scratch, half, half - 1u);
        Box(scratch, line, half - 1u, half);
        Box(line, scratch, half, half);
      }
      line.swap(scratch);
    }
  }

  uint32 BlurExtent(float sigma) noexcept
  {
    if (!(sigma > 0.f) || !std::isfinite(sigma))
    {
      return 0u;
    }
    if (sigma < BoxBlurSigma)
    {
      return static_cast<uint32>(std::ceil(3.f * sigma));
    }
    return 3u * (BoxSize(sigma) / 2u);
  }

  CoverageMask BlurCoverage(const CoverageMask &mask, float sigma) noexcept
  {
    const uint32 extent = BlurExtent(sigma);
    if (extent == 0u || mask.Bounds.IsEmpty())
    {
      return mask;
    }

    List<float> kernel;
    if (sigma < BoxBlurSigma)
    {
      kernel.resize(2u * extent + 1u);
      float total = 0.f;
      for (uint32 i = 0u; i < kernel.size(); ++i)
      {
        // The Gaussian's mass over the pixel, not its value at the centre: exact for even coverage.
        const float x = static_cast<float>(i) - static_cast<float>(extent);
        const float k = 1.f / (sigma * std::sqrt(2.f));
        kernel[i] = 0.5f * (std::erf((x + 0.5f) * k) - std::erf((x - 0.5f) * k));
        total += kernel[i];
      }
      for (auto &weight : kernel)
      {
        weight /= total;
      }
    }

    const uint32 width = mask.Bounds.Width + 2u * extent;
    const uint32 height = mask.Bounds.Height + 2u * extent;
    List<float> values(static_cast<size_t>(width) * height, 0.f);
    for (uint32 y = 0u; y < mask.Bounds.Height; ++y)
    {
      for (uint32 x = 0u; x < mask.Bounds.Width; ++x)
      {
        values[static_cast<size_t>(y + extent) * width + x + extent] =
          static_cast<float>(mask.Coverage[static_cast<size_t>(y) * mask.Bounds.Width + x]) / 255.f;
      }
    }

    // Rows, then columns: a Gaussian is separable, and so is each box.
    List<float> line;
    List<float> scratch;
    for (uint32 y = extent; y < extent + mask.Bounds.Height; ++y)
    {
      line.assign(values.begin() + static_cast<int64>(y) * width,
                  values.begin() + static_cast<int64>(y + 1u) * width);
      BlurLine(line, scratch, sigma, kernel);
      std::copy(line.begin(), line.end(), values.begin() + static_cast<int64>(y) * width);
    }
    for (uint32 x = 0u; x < width; ++x)
    {
      line.resize(height);
      for (uint32 y = 0u; y < height; ++y)
      {
        line[y] = values[static_cast<size_t>(y) * width + x];
      }
      BlurLine(line, scratch, sigma, kernel);
      for (uint32 y = 0u; y < height; ++y)
      {
        values[static_cast<size_t>(y) * width + x] = line[y];
      }
    }

    CoverageMask blurred {.Bounds = {.X = mask.Bounds.X - static_cast<int32>(extent),
                                     .Y = mask.Bounds.Y - static_cast<int32>(extent),
                                     .Width = width,
                                     .Height = height},
                          .Coverage = {}};
    blurred.Coverage.resize(values.size());
    for (size_t i = 0u; i < values.size(); ++i)
    {
      blurred.Coverage[i] = static_cast<uint8>(std::clamp(values[i], 0.f, 1.f) * 255.f + 0.5f);
    }
    return blurred;
  }
}
