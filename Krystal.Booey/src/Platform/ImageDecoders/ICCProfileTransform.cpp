#include "Krystal.Booey/Platform/ImageDecoders/ICCProfileTransform.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <array>
#include <cstring>
#include <utility>
#include <lcms2.h>

namespace krys::boo
{
  namespace
  {
    struct LCMSProfileDeleter
    {
      void operator()(void *profile) const noexcept
      {
        cmsCloseProfile(profile);
      }
    };
    using LCMSProfilePtr = UniquePtr<void, LCMSProfileDeleter>;

    KRYS_NODISCARD cmsUInt32Number LCMSFormat(ICCProfileTransform::PixelLayout layout) noexcept
    {
      switch (layout)
      {
        case ICCProfileTransform::PixelLayout::RGB:          return TYPE_RGB_8;
        case ICCProfileTransform::PixelLayout::RGBA:         return TYPE_RGBA_8;
        case ICCProfileTransform::PixelLayout::BGRA:         return TYPE_BGRA_8;
        case ICCProfileTransform::PixelLayout::InvertedCMYK: return TYPE_CMYK_8_REV;
      }
      return TYPE_RGB_8;
    }

    KRYS_NODISCARD constexpr uint32 BytesPerPixel(ICCProfileTransform::PixelLayout layout) noexcept
    {
      return layout == ICCProfileTransform::PixelLayout::RGB ? 3u : 4u;
    }

    /// @brief Whether an RGB to RGB transform leaves every colour of a grid through the RGB cube (each
    /// channel at 0, 17, ..., 255) as it is: a profile that describes sRGB.
    KRYS_NODISCARD bool LeavesColoursUnchanged(cmsHTRANSFORM transform) noexcept
    {
      constexpr uint32 steps = 16u;
      constexpr uint32 pixelCount = steps * steps * steps;
      std::array<uint8, pixelCount * 3> probe {};
      size_t index = 0;
      for (uint32 red = 0; red < steps; ++red)
      {
        for (uint32 green = 0; green < steps; ++green)
        {
          for (uint32 blue = 0; blue < steps; ++blue)
          {
            probe[index++] = static_cast<uint8>(red * 17u);
            probe[index++] = static_cast<uint8>(green * 17u);
            probe[index++] = static_cast<uint8>(blue * 17u);
          }
        }
      }
      std::array<uint8, pixelCount * 3> result {};
      cmsDoTransform(transform, probe.data(), result.data(), pixelCount);
      return probe == result;
    }
  }

  ICCProfileTransform::ICCProfileTransform(ICCProfileTransform &&other) noexcept
      : _transform(std::exchange(other._transform, nullptr))
  {
  }

  ICCProfileTransform &ICCProfileTransform::operator=(ICCProfileTransform &&other) noexcept
  {
    if (this != &other)
    {
      if (_transform)
      {
        cmsDeleteTransform(_transform);
      }
      _transform = std::exchange(other._transform, nullptr);
    }
    return *this;
  }

  ICCProfileTransform::~ICCProfileTransform() noexcept
  {
    if (_transform)
    {
      cmsDeleteTransform(_transform);
    }
  }

  bool ICCProfileTransform::DescribesPixels(Span<const uint8> profile, PixelLayout source) noexcept
  {
    if (profile.empty())
    {
      return false;
    }
    auto sourceProfile =
      LCMSProfilePtr(cmsOpenProfileFromMem(profile.data(), static_cast<cmsUInt32Number>(profile.size())));
    return sourceProfile
           && cmsGetColorSpace(sourceProfile.get())
                == (source == PixelLayout::InvertedCMYK ? cmsSigCmykData : cmsSigRgbData);
  }

  ICCProfileTransform ICCProfileTransform::Create(Span<const uint8> profile, PixelLayout source,
                                                  PixelLayout destination) noexcept
  {
    if (profile.empty())
    {
      return {};
    }
    auto sourceProfile =
      LCMSProfilePtr(cmsOpenProfileFromMem(profile.data(), static_cast<cmsUInt32Number>(profile.size())));
    if (!sourceProfile)
    {
      return {};
    }
    const auto expectedSpace = source == PixelLayout::InvertedCMYK ? cmsSigCmykData : cmsSigRgbData;
    if (cmsGetColorSpace(sourceProfile.get()) != expectedSpace || destination == PixelLayout::InvertedCMYK)
    {
      return {};
    }

    auto srgbProfile = LCMSProfilePtr(cmsCreate_sRGBProfile());
    if (!srgbProfile)
    {
      return {};
    }

    // An RGB profile that changes no colours needs no transform (most embedded profiles are sRGB's).
    if (source != PixelLayout::InvertedCMYK)
    {
      cmsHTRANSFORM probe = cmsCreateTransform(sourceProfile.get(), TYPE_RGB_8, srgbProfile.get(), TYPE_RGB_8,
                                               INTENT_RELATIVE_COLORIMETRIC, 0);
      if (!probe)
      {
        return {};
      }
      const bool unchanged = LeavesColoursUnchanged(probe);
      cmsDeleteTransform(probe);
      if (unchanged)
      {
        return {};
      }
    }

    const bool copyAlpha =
      BytesPerPixel(source) == 4u && BytesPerPixel(destination) == 4u && source != PixelLayout::InvertedCMYK;
    cmsHTRANSFORM transform =
      cmsCreateTransform(sourceProfile.get(), LCMSFormat(source), srgbProfile.get(), LCMSFormat(destination),
                         INTENT_RELATIVE_COLORIMETRIC, copyAlpha ? cmsFLAGS_COPY_ALPHA : 0);
    return transform ? ICCProfileTransform(transform) : ICCProfileTransform();
  }

  void ICCProfileTransform::Transform(const uint8 *source, uint8 *destination,
                                      uint32 pixelCount) const noexcept
  {
    if (_transform && pixelCount)
    {
      cmsDoTransform(_transform, source, destination, pixelCount);
    }
  }
}
