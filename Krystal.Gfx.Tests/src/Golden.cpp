#include "Krystal.Gfx.Tests/Golden.hpp"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <stb/stb_image.h>
#include <stb/stb_image_write.h>

namespace krys::gfx::tests
{
  std::filesystem::path GoldenDirectory()
  {
    return KRYS_GFX_TESTS_GOLDEN_DIR;
  }

  std::filesystem::path GoldenOutputDirectory()
  {
    return std::filesystem::current_path() / "golden-output";
  }

  bool UpdatingGoldens()
  {
    char *value = nullptr;
    size_t length = 0u;
    if (_dupenv_s(&value, &length, "KRYS_UPDATE_GOLDENS") != 0 || value == nullptr)
    {
      return false;
    }
    const bool updating = std::strcmp(value, "1") == 0;
    std::free(value);
    return updating;
  }

  Maybe<GoldenImage> LoadPng(const std::filesystem::path &path)
  {
    int width = 0;
    int height = 0;
    int channels = 0;
    unsigned char *data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
    if (data == nullptr)
    {
      return {};
    }
    GoldenImage image {
      .Width = static_cast<uint32>(width), .Height = static_cast<uint32>(height), .Pixels = {}};
    image.Pixels.resize(static_cast<size_t>(width) * static_cast<size_t>(height));
    std::memcpy(image.Pixels.data(), data, image.Pixels.size() * 4u);
    stbi_image_free(data);
    return image;
  }

  bool SavePng(const std::filesystem::path &path, const GoldenImage &image)
  {
    std::error_code error;
    std::filesystem::create_directories(path.parent_path(), error);
    return stbi_write_png(path.string().c_str(), static_cast<int>(image.Width),
                          static_cast<int>(image.Height), 4, image.Pixels.data(),
                          static_cast<int>(image.Width * 4u))
           != 0;
  }

  GoldenComparison CompareImages(const GoldenImage &actual, const GoldenImage &expected, uint32 tolerance)
  {
    GoldenComparison comparison {.PixelCount = static_cast<uint32>(actual.Pixels.size())};
    if (actual.Width != expected.Width || actual.Height != expected.Height)
    {
      comparison.SizesMatch = false;
      return comparison;
    }
    double total = 0.0;
    for (size_t i = 0u; i < actual.Pixels.size(); ++i)
    {
      uint32 largest = 0u;
      for (size_t c = 0u; c < 4u; ++c)
      {
        const int difference = int(actual.Pixels[i][c]) - int(expected.Pixels[i][c]);
        largest = std::max(largest, static_cast<uint32>(difference < 0 ? -difference : difference));
      }
      comparison.LargestDifference = std::max(comparison.LargestDifference, largest);
      comparison.PixelsOver += largest > tolerance ? 1u : 0u;
      total += largest;
    }
    comparison.MeanDifference =
      actual.Pixels.empty() ? 0.0 : total / static_cast<double>(actual.Pixels.size());
    return comparison;
  }

  GoldenImage DifferenceImage(const GoldenImage &actual, const GoldenImage &expected)
  {
    GoldenImage difference {.Width = actual.Width, .Height = actual.Height, .Pixels = {}};
    difference.Pixels.resize(actual.Pixels.size());
    for (size_t i = 0u; i < actual.Pixels.size() && i < expected.Pixels.size(); ++i)
    {
      for (size_t c = 0u; c < 3u; ++c)
      {
        const int value = std::abs(int(actual.Pixels[i][c]) - int(expected.Pixels[i][c])) * 4;
        difference.Pixels[i][c] = static_cast<uint8>(std::min(value, 255));
      }
      difference.Pixels[i][3] = 255u;
    }
    return difference;
  }
}
