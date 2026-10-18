#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <filesystem>

namespace krys::gfx::tests
{
  /// @brief An 8-bit RGBA image, rows from the top.
  struct GoldenImage
  {
    uint32 Width {0u};
    uint32 Height {0u};
    List<Array<uint8, 4>> Pixels {};
  };

  /// @brief How far an image is from its golden.
  struct GoldenComparison
  {
    /// @brief The largest difference in any channel of any pixel.
    uint32 LargestDifference {0u};
    /// @brief Pixels with a channel differing by more than the tolerance.
    uint32 PixelsOver {0u};
    uint32 PixelCount {0u};
    /// @brief The mean of each pixel's largest channel difference.
    double MeanDifference {0.0};
    bool SizesMatch {true};
  };

  /// @brief Where the goldens are kept, in the source tree.
  KRYS_NODISCARD std::filesystem::path GoldenDirectory();
  /// @brief Where actual and difference images are written, in the build tree.
  KRYS_NODISCARD std::filesystem::path GoldenOutputDirectory();
  /// @brief Whether `KRYS_UPDATE_GOLDENS=1` asks for Render2D's goldens to be written rather than checked.
  KRYS_NODISCARD bool UpdatingGoldens();

  KRYS_NODISCARD Maybe<GoldenImage> LoadPng(const std::filesystem::path &path);
  bool SavePng(const std::filesystem::path &path, const GoldenImage &image);

  /// @brief Compares two images channel by channel.
  KRYS_NODISCARD GoldenComparison CompareImages(const GoldenImage &actual, const GoldenImage &expected,
                                                uint32 tolerance);
  /// @brief The difference between two images of one size, scaled up so small differences show, on black.
  KRYS_NODISCARD GoldenImage DifferenceImage(const GoldenImage &actual, const GoldenImage &expected);
}
